// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

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
std::vector<IndependentObservation> observe_independent_packages(
    std::string_view filesystem_id);

} // namespace filesystems
