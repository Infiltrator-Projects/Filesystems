// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "process_runner.hpp"

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace filesystems {

// A positive status proves the installed Debian package's declared payload
// digest, sole package ownership and ELF type. It does NOT imply a successful
// mount, format, recovery, boot or physical-media write.
enum class IndependentProof { Missing, VerifiedPackage, Unverified };

struct IndependentEvidence {
    bool package_installed = false;
    bool lists_expected_file = false;
    bool sole_owner = false;
    bool regular_file = false;
    bool valid_declared_digest = false;
    bool digest_matches = false;
    bool elf64_shared = false;
    bool format_identity_matches = false;
};

IndependentProof evaluate_independent_evidence(const IndependentEvidence& evidence);

struct IndependentObservation {
    std::string module;
    std::string package;
    IndependentProof proof = IndependentProof::Unverified;
    std::string detail;
};

// Empty only for identities not yet listed in the independent-package contract.
// An explicit read-only observation seam. Production always uses trusted
// absolute system locations; tests can provide an isolated module directory
// and synthetic package database responses without touching the host.
struct IndependentProbeBackend {
    std::string module_directory;
    std::function<CommandResult(const std::vector<std::string>&)> command;
};

std::vector<IndependentObservation> observe_independent_packages_with_backend(
    std::string_view filesystem_id, const IndependentProbeBackend& backend);

std::vector<IndependentObservation> observe_independent_packages(
    std::string_view filesystem_id);

} // namespace filesystems
