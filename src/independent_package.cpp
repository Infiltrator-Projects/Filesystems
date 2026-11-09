// SPDX-License-Identifier: GPL-3.0-or-later
#include "independent_package.hpp"
#include "process_runner.hpp"

#include <glib.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/stat.h>

namespace filesystems {
namespace {

struct IndependentSpec {
    std::string_view identity;
    std::string_view name;
    std::string_view package;
    std::string_view required_export;
    std::string_view required_manifest_marker;
};

// This is an installed-package observation mapping, not a source, build or
// dependency mapping. No standalone format code is included or copied here.
// The classic FAT catalogue identity intentionally groups three distinct SOs.
constexpr std::array<IndependentSpec, 13> kIndependent = {{
    {"ext2",  "EXT2.so",  "infiltrator-filesystem-ext2",  "", ""},
    {"ext3",  "EXT3.so",  "infiltrator-filesystem-ext3",  "", ""},
    {"ext4",  "EXT4.so",  "infiltrator-filesystem-ext4",  "", ""},
    {"ofs",   "OFS.so",   "infiltrator-filesystem-ofs",   "", "IFS-MODULE:ofs:1"},
    {"ffs",   "FFS.so",   "infiltrator-filesystem-ffs",
        "infiltrator_filesystem_ffs_defrag_v1", ""},
    {"sfs",   "SFS.so",   "infiltrator-filesystem-sfs",   "", ""},
    {"sfs2",  "SFS2.so",  "infiltrator-filesystem-sfs2",  "", ""},
    {"pfs3",  "PFS3.so",  "infiltrator-filesystem-pfs3", "", ""},
    {"fat16", "FAT12.so", "infiltrator-filesystem-fat12", "", "IFS-MODULE:fat12:1"},
    {"fat16", "FAT16.so", "infiltrator-filesystem-fat16", "", "IFS-MODULE:fat16:1"},
    {"fat16", "FAT32.so", "infiltrator-filesystem-fat32", "", "IFS-MODULE:fat32:1"},
    {"exfat", "exFAT.so", "infiltrator-filesystem-exfat",
        "infiltrator_filesystem_exfat_defrag_v1", ""},
    {"ntfs",  "NTFS.so",  "infiltrator-filesystem-ntfs",  "", ""}
}};

bool exact_line(const std::string& output, const std::string& expected)
{
    std::istringstream lines(output);
    std::string line;
    while (std::getline(lines, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line == expected) {
            return true;
        }
    }
    return false;
}

bool valid_sha256(const std::string& text)
{
    if (text.size() != 64U) return false;
    for (const unsigned char c : text) {
        if (!std::isxdigit(c)) return false;
    }
    return true;
}

bool sha256_file(const std::string& path, std::string* result)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) return false;
    std::unique_ptr<GChecksum, decltype(&g_checksum_free)> digest(
        g_checksum_new(G_CHECKSUM_SHA256), &g_checksum_free);
    if (!digest) return false;
    std::array<char, 65536> buffer {};
    while (stream) {
        stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const auto read = stream.gcount();
        if (read > 0) {
            g_checksum_update(digest.get(),
                reinterpret_cast<const guchar*>(buffer.data()),
                static_cast<gssize>(read));
        }
    }
    if (!stream.eof()) return false;
    *result = g_checksum_get_string(digest.get());
    return true;
}

IndependentObservation inspect(const IndependentSpec& spec,
                               const IndependentProbeBackend& backend)
{
    IndependentObservation observation;
    observation.module = std::string(spec.name);
    observation.package = std::string(spec.package);
    IndependentEvidence evidence;
    std::string version;

    const auto package = backend.command({
        "dpkg-query", "-W",
        "-f=${db:Status-Status}\\t${Version}\\t${X-Infiltrator-Resource-SHA256}\\n",
        observation.package
    });
    if (!package.succeeded()) {
        if (package.spawned && package.exit_status == 1 &&
            package.stderr_text.find("no packages found matching") != std::string::npos) {
            observation.proof = IndependentProof::Missing;
            observation.detail = "not installed";
        } else {
            observation.detail = "package database query failed";
        }
        return observation;
    }

    std::istringstream line(package.stdout_text);
    std::string status;
    std::string expected_digest;
    std::string extra;
    if (!std::getline(line, status, '\t') ||
        !std::getline(line, version, '\t') ||
        !std::getline(line, expected_digest) ||
        std::getline(line, extra) ||
        status != "installed") {
        observation.proof = status == "not-installed" || status == "config-files"
            ? IndependentProof::Missing : IndependentProof::Unverified;
        observation.detail = observation.proof == IndependentProof::Missing
            ? "not installed" : "unexpected package status/metadata";
        return observation;
    }
    if (!expected_digest.empty() && expected_digest.back() == '\r')
        expected_digest.pop_back();
    evidence.package_installed = true;
    evidence.valid_declared_digest = valid_sha256(expected_digest);
    // SHA-256 is hexadecimal, so A-F case must not change whether the
    // installed bytes match the package's declared digest.
    if (evidence.valid_declared_digest) {
        std::transform(expected_digest.begin(), expected_digest.end(),
                       expected_digest.begin(),
                       [](const unsigned char c) {
                           return static_cast<char>(std::tolower(c));
                       });
    }
    const std::string path = "/System/Filesystems/" + observation.module;
    const std::string disk_path = backend.module_directory + "/" + observation.module;

    const auto listing = backend.command({"dpkg-query", "-L", observation.package});
    evidence.lists_expected_file =
        listing.succeeded() && exact_line(listing.stdout_text, path);

    const auto owners = backend.command({"dpkg-query", "-S", path});
    evidence.sole_owner = owners.succeeded() &&
        (owners.stdout_text == observation.package + ": " + path + "\n" ||
         owners.stdout_text == observation.package + ": " + path);

    struct stat file_status {};
    evidence.regular_file = lstat(disk_path.c_str(), &file_status) == 0 &&
        S_ISREG(file_status.st_mode) && (file_status.st_mode & 0022) == 0;

    if (evidence.regular_file && evidence.valid_declared_digest) {
        std::string actual_digest;
        evidence.digest_matches = sha256_file(disk_path, &actual_digest) &&
            actual_digest == expected_digest;
    }
    if (evidence.regular_file) {
        const auto elf = backend.command({"readelf", "-h", disk_path});
        evidence.elf64_shared = elf.succeeded() &&
            elf.stdout_text.find("ELF64") != std::string::npos &&
            elf.stdout_text.find("Type:") != std::string::npos &&
            elf.stdout_text.find("DYN (") != std::string::npos &&
            elf.stdout_text.find("Advanced Micro Devices X86-64") !=
                std::string::npos;
        evidence.format_identity_matches = evidence.elf64_shared;
        if (!spec.required_export.empty()) {
            const auto symbols = backend.command({"readelf", "-W", "--dyn-syms", disk_path});
            evidence.format_identity_matches =
                evidence.format_identity_matches && symbols.succeeded() &&
                symbols.stdout_text.find(spec.required_export) != std::string::npos;
        }
        if (!spec.required_manifest_marker.empty()) {
            const auto markers = backend.command({"readelf", "-p", ".ifsmeta", disk_path});
            evidence.format_identity_matches =
                evidence.format_identity_matches && markers.succeeded() &&
                markers.stdout_text.find(spec.required_manifest_marker) !=
                    std::string::npos;
        }
    }
    observation.proof = evaluate_independent_evidence(evidence);
    if (observation.proof == IndependentProof::VerifiedPackage) {
        observation.detail = "package " + version +
            " owns the expected ELF shared object; SHA-256 matches its installed"
            " package metadata (runtime operations not tested)";
    } else {
        observation.detail = "installed package or ELF could not be verified"
            " (file owner, digest, format identity or metadata mismatch)";
    }
    return observation;
}

} // namespace

IndependentProof evaluate_independent_evidence(const IndependentEvidence& e)
{
    if (!e.package_installed) return IndependentProof::Missing;
    return e.lists_expected_file && e.sole_owner && e.regular_file &&
        e.valid_declared_digest && e.digest_matches && e.elf64_shared &&
        e.format_identity_matches
        ? IndependentProof::VerifiedPackage
        : IndependentProof::Unverified;
}

std::vector<IndependentObservation> observe_independent_packages_with_backend(
    const std::string_view filesystem_id, const IndependentProbeBackend& backend)
{
    std::vector<IndependentObservation> observations;
    if (backend.module_directory.empty() || !backend.command) {
        return observations;
    }
    for (const auto& spec : kIndependent) {
        if (spec.identity == filesystem_id) {
            observations.push_back(inspect(spec, backend));
        }
    }
    return observations;
}

std::vector<IndependentObservation> observe_independent_packages(
    const std::string_view filesystem_id)
{
    // Do not allow environment overrides, PATH binaries or untrusted callers
    // to redirect production observations away from installed packages.
    const IndependentProbeBackend production {
        "/System/Filesystems",
        [](const std::vector<std::string>& args) {
            return run_command(args);
        }
    };
    return observe_independent_packages_with_backend(filesystem_id, production);
}

} // namespace filesystems
