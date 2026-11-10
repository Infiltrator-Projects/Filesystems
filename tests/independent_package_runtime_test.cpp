// SPDX-License-Identifier: GPL-3.0-or-later
// Read-only acceptance against the actual manager-side C++ probe, not a
// duplicate Python approximation. All files live in a disposable directory;
// all simulated dpkg-query responses are explicit in-process fixtures.
#include "independent_package.hpp"

#include <glib.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using filesystems::IndependentObservation;
using filesystems::IndependentProbeBackend;
using filesystems::IndependentProof;

namespace {

struct Entry {
    std::string module;
    std::string package;
    std::string digest;
    bool missing_package = false;
    bool missing_listing = false;
    bool wrong_owner = false;
    bool bad_status = false;
    bool bad_metadata = false;
    bool fail_readelf = false;
    bool legacy_export = false;
    bool legacy_marker = false;
};

struct TemporaryDirectory {
    fs::path path;
    TemporaryDirectory()
    {
        GError* error = nullptr;
        gchar* temporary = g_dir_make_tmp("filesystems-independent-probe-XXXXXX",
                                          &error);
        if (temporary == nullptr) {
            const std::string why = error != nullptr ? error->message : "unknown";
            g_clear_error(&error);
            throw std::runtime_error("Cannot create safe temporary fixture: " + why);
        }
        path = temporary;
        g_free(temporary);
    }
    ~TemporaryDirectory()
    {
        std::error_code error;
        fs::remove_all(path, error);
    }
    TemporaryDirectory(const TemporaryDirectory&) = delete;
    TemporaryDirectory& operator=(const TemporaryDirectory&) = delete;
};

std::string read_all(const fs::path& path)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream) throw std::runtime_error("Cannot read fixture " + path.string());
    return std::string(std::istreambuf_iterator<char>(stream),
                       std::istreambuf_iterator<char>());
}

void write_all(const fs::path& path, const std::string& contents)
{
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    if (!stream) throw std::runtime_error("Cannot write fixture " + path.string());
}

std::string digest(const fs::path& path)
{
    const std::string bytes = read_all(path);
    gchar* sum = g_compute_checksum_for_data(
        G_CHECKSUM_SHA256,
        reinterpret_cast<const guchar*>(bytes.data()), bytes.size());
    if (sum == nullptr) throw std::runtime_error("Cannot hash fixture");
    std::string value = sum;
    g_free(sum);
    return value;
}

filesystems::CommandResult fake_ok(const std::string& output)
{
    filesystems::CommandResult result;
    result.spawned = true;
    result.exit_status = 0;
    result.stdout_text = output;
    return result;
}

filesystems::CommandResult fake_error(const std::string& output)
{
    auto result = fake_ok("");
    result.exit_status = 1;
    result.stderr_text = output;
    return result;
}

const IndependentObservation& find_observation(
    const std::vector<IndependentObservation>& all,
    const std::string& module)
{
    for (const auto& observation : all) {
        if (observation.module == module) return observation;
    }
    throw std::runtime_error("No result for " + module);
}

void require(const bool accepted, const std::string& detail)
{
    if (!accepted) throw std::runtime_error(detail);
}

} // namespace

int main(int argc, char** argv)
{
    try {
        require(argc == 6, "Expected FFS, exFAT, FAT12, FAT16, FAT32 test SOs");
        const TemporaryDirectory temporary;
        std::map<std::string, Entry> by_module;
        const std::array<std::string, 5> modules = {
            "FFS.so", "exFAT.so", "FAT12.so", "FAT16.so", "FAT32.so"
        };
        const std::array<std::string, 5> packages = {
            "infiltrator-filesystem-ffs",
            "infiltrator-filesystem-exfat",
            "infiltrator-filesystem-fat12",
            "infiltrator-filesystem-fat16",
            "infiltrator-filesystem-fat32"
        };
        for (std::size_t i = 0; i != modules.size(); ++i) {
            const fs::path target = temporary.path / modules[i];
            fs::copy_file(argv[i + 1], target);
            by_module.emplace(modules[i], Entry{
                modules[i], packages[i], digest(target)
            });
        }

        const IndependentProbeBackend backend {
            temporary.path.string(),
            [&](const std::vector<std::string>& args) {
                if (args.empty()) return fake_error("Missing command");
                if (args[0] == "readelf") {
                    const auto module = fs::path(args.back()).filename().string();
                    const auto entry = by_module.find(module);
                    if (entry != by_module.end() && entry->second.fail_readelf)
                        return fake_error("Fixture readelf command failed");
                    // Use the real ELF, with optional simulated hostile
                    // readelf output proving V1 symbols/markers are rejected.
                    auto result=filesystems::run_command(args);
                    if (entry != by_module.end() && result.succeeded()) {
                        if (entry->second.legacy_export && args.size()==4U &&
                            args[1]=="-W" && args[2]=="--dyn-syms") {
                            result.stdout_text +=
                                "  99: 0000000000000100  1 FUNC GLOBAL DEFAULT 12 "
                                "infiltrator_filesystem_api_v1\n";
                        }
                        if (entry->second.legacy_marker && args.size()==4U &&
                            args[1]=="-p" && args[2]==".ifsmeta") {
                            result.stdout_text +=
                                "  [ 99]  IFS-MODULE:fat12:common1\n";
                        }
                    }
                    return result;
                }
                if (args[0] != "dpkg-query" || args.size() < 3U)
                    return fake_error("Unexpected observation command");
                if (args[1] == "-W" && args.size() == 4U) {
                    for (const auto& pair : by_module) {
                        const Entry& entry = pair.second;
                        if (entry.package != args[3]) continue;
                        if (entry.missing_package)
                            return fake_error("no packages found matching " +
                                              entry.package);
                        if (entry.bad_status)
                            return fake_ok("half-configured\t1.0.0\t" +
                                           entry.digest + "\n");
                        return fake_ok("installed\t1.0.0\t" +
                            (entry.bad_metadata ? "" : entry.digest) + "\n");
                    }
                }
                if (args[1] == "-L" && args.size() == 3U) {
                    for (const auto& pair : by_module) {
                        const Entry& entry = pair.second;
                        if (entry.package == args[2])
                            return fake_ok(entry.missing_listing
                                ? "/some/unrelated/file\n"
                                : "/System/Filesystems/" + entry.module + "\n");
                    }
                }
                if (args[1] == "-S" && args.size() == 3U) {
                    const auto module = fs::path(args[2]).filename().string();
                    const auto entry = by_module.find(module);
                    if (entry != by_module.end()) {
                        return fake_ok(
                            (entry->second.wrong_owner
                             ? "unrelated-package" : entry->second.package) +
                            ": /System/Filesystems/" + module + "\n");
                    }
                }
                return fake_error("Unknown fixture request");
            }
        };

        auto check = [&](const std::string& filesystem_id,
                         const std::string& module,
                         const IndependentProof expected) {
            const auto observations =
                filesystems::observe_independent_packages_with_backend(
                    filesystem_id, backend);
            const auto& actual = find_observation(observations, module);
            require(actual.proof == expected,
                    "Incorrect proof for " + module + " expected " +
                    std::to_string(static_cast<int>(expected)) + " got " +
                    std::to_string(static_cast<int>(actual.proof)) +
                    ": " + actual.detail);
        };

        check("ffs", "FFS.so", IndependentProof::VerifiedPackage);
        check("exfat", "exFAT.so", IndependentProof::VerifiedPackage);
        for (const auto& module : {"FAT12.so", "FAT16.so", "FAT32.so"})
            check("fat16", module, IndependentProof::VerifiedPackage);

        // The classic FAT catalogue row aggregates *three* separate packages.
        require(filesystems::observe_independent_packages_with_backend(
                    "fat16", backend).size() == 3U,
                "FAT12/16/32 were combined or a package was omitted");

        for (const auto& test_case : {
                 std::string("FFS.so"), std::string("exFAT.so"),
                 std::string("FAT12.so"), std::string("FAT16.so"),
                 std::string("FAT32.so")}) {
            Entry& entry = by_module.at(test_case);
            const std::string identity = test_case == "FFS.so" ? "ffs"
                : test_case == "exFAT.so" ? "exfat" : "fat16";
            const fs::path installed = temporary.path / entry.module;
            const std::string original = read_all(installed);
            auto unverified = [&]() {
                check(identity, entry.module, IndependentProof::Unverified);
            };
            auto reset = [&]() {
                write_all(installed, original);
                entry.digest = digest(installed);
            };

            // Uppercase SHA-256 metadata is valid and must compare equal to
            // the same lowercase digest computed from the installed module.
            std::transform(entry.digest.begin(), entry.digest.end(),
                           entry.digest.begin(),
                           [](const unsigned char ch) {
                               return static_cast<char>(std::toupper(ch));
                           });
            check(identity, entry.module, IndependentProof::VerifiedPackage);
            reset();

            entry.missing_package = true;
            check(identity, entry.module, IndependentProof::Missing);
            entry.missing_package = false;

            entry.bad_status = true; unverified(); entry.bad_status = false;
            entry.bad_metadata = true; unverified(); entry.bad_metadata = false;
            entry.missing_listing = true;
            unverified();
            entry.missing_listing = false;
            entry.wrong_owner = true; unverified(); entry.wrong_owner = false;
            entry.fail_readelf = true; unverified(); entry.fail_readelf = false;
            entry.legacy_export = true; unverified(); entry.legacy_export = false;
            entry.legacy_marker = true; unverified(); entry.legacy_marker = false;

            fs::remove(installed);
            unverified();
            reset();

            write_all(installed, original + "changed");
            unverified();
            reset();

            write_all(installed, "This is not an ELF shared object");
            entry.digest = digest(installed);
            unverified();
            reset();

            // Swap in a real ELF of another format, and forge the package's
            // advertised digest to match it. The format-specific export or
            // .ifsmeta token must *still* fail closed.
            const std::string foreign = test_case == "FFS.so"
                ? "exFAT.so" : test_case == "exFAT.so"
                ? "FFS.so" : test_case == "FAT12.so"
                ? "FAT16.so" : "FAT12.so";
            write_all(installed, read_all(temporary.path / foreign));
            entry.digest = digest(installed);
            unverified();
            reset();

            fs::remove(installed);
            fs::create_symlink(temporary.path / foreign, installed);
            unverified();
            fs::remove(installed);
            reset();

            check(identity, entry.module, IndependentProof::VerifiedPackage);
        }

        // The coordinator must observe all 23 independently owned .so files,
        // even though only five have isolated ELF runtime fixtures here.
        // All newly indexed providers fail closed while not installed.
        const std::array<std::pair<std::string_view, std::string_view>, 10> added = {{
            {"xfs", "XFS.so"}, {"btrfs", "BTRFS.so"},
            {"minix", "MINIX.so"}, {"zfs", "ZFS.so"},
            {"ufs", "UFS1.so"}, {"ufs", "UFS2.so"},
            {"hfs", "HFS.so"}, {"hfsplus", "HFSPLUS.so"},
            {"apfs", "APFS.so"}, {"swap", "SWAP.so"}
        }};
        for (const auto& [identity, module] : added) {
            const auto observations =
                filesystems::observe_independent_packages_with_backend(
                    identity, backend);
            const auto& actual=find_observation(observations,std::string(module));
            require(actual.proof != IndependentProof::VerifiedPackage,
                    "uninstalled V2 provider incorrectly qualified: " +
                    std::string(module));
        }
        require(filesystems::observe_independent_packages_with_backend(
                    "ufs", backend).size()==2U,
                "UFS1/UFS2 must remain separate modules under one UFS catalogue row");

        // Every FAT module must still qualify after independently corrupting
        // and restoring its peers. There is no single blended FAT .so.
        for (const auto& module : {"FAT12.so", "FAT16.so", "FAT32.so"})
            check("fat16", module, IndependentProof::VerifiedPackage);
        std::cout << "CXX-INDEPENDENT-DPKG-PROBE-NEGATIVE-FIXTURES-PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "independent_package_runtime_test: " << error.what() << '\n';
        return 1;
    }
}
