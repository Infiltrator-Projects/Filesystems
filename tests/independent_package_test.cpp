// SPDX-License-Identifier: GPL-3.0-or-later
#include "independent_package.hpp"

#include <iostream>
#include <string_view>

namespace {
int fail(const char* reason) {
    std::cerr << "independent_package_test: " << reason << '\n';
    return 1;
}
}

int main() {
    using namespace filesystems;
    IndependentEvidence e;
    if (evaluate_independent_evidence(e) != IndependentProof::Missing)
        return fail("absent package was accepted");
    e.package_installed = true;
    if (evaluate_independent_evidence(e) != IndependentProof::Unverified)
        return fail("unverified package was accepted");
    e.lists_expected_file = true;
    e.sole_owner = true;
    e.regular_file = true;
    e.valid_declared_digest = true;
    e.digest_matches = true;
    e.elf64_shared = true;
    e.format_identity_matches = true;
    if (evaluate_independent_evidence(e) != IndependentProof::VerifiedPackage)
        return fail("fully checked installed package was not verified");
    auto reject_each = [&](bool IndependentEvidence::* field) {
        IndependentEvidence bad = e;
        bad.*field = false;
        return evaluate_independent_evidence(bad) ==
            IndependentProof::Unverified;
    };
    for (auto field : {&IndependentEvidence::lists_expected_file,
                       &IndependentEvidence::sole_owner,
                       &IndependentEvidence::regular_file,
                       &IndependentEvidence::valid_declared_digest,
                       &IndependentEvidence::digest_matches,
                       &IndependentEvidence::elf64_shared,
                       &IndependentEvidence::format_identity_matches}) {
        if (!reject_each(field))
            return fail("missing proof step did not reject installed package");
    }

    // Each format has its own package and module; the classic FAT catalogue
    // intentionally describes three independent package products.
    // The observation function is exercised here on a non-matching identity:
    // tests must never install or manipulate a real /System path.
    if (!observe_independent_packages("not-a-format").empty())
        return fail("unknown format created an independent package observation");
    std::cout << "INDEPENDENT-PACKAGE-FAIL-CLOSED-CONTRACT-PASS\n";
    return 0;
}
