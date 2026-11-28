#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Validate qualification evidence and derive the public tables from the sole register."""
import argparse
from collections import Counter
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / "data/provider-register.json"
DOC = ROOT / "docs/PROVIDER_REGISTER.md"
CODES = ("Q", "M", "P", "I", "F", "B", "U", "N/A")
LABELS = {"Q": "Fully qualified", "M": "Missing", "P": "Partially implemented",
          "I": "Implemented, awaiting qualification", "F": "Failed qualification",
          "B": "Blocked", "U": "Unverified", "N/A": "Inapplicable"}


def percent(value, denominator):
    return round(100 * value / denominator, 4) if denominator else 0.0


def calculate(data):
    totals = Counter()
    finished = blocked = waiting = 0
    names = set()
    for repo in data["repositories"]:
        name = repo["repository"]
        if name in names:
            raise ValueError(f"duplicate repository {name}")
        names.add(name)
        if not re.fullmatch("[0-9a-f]{40}", repo["main_sha"]):
            raise ValueError(f"invalid main revision in {name}")
        ids = set(data["common_requirements"]) | set(repo["native_requirements"])
        if len(ids) != len(data["common_requirements"]) + len(repo["native_requirements"]):
            raise ValueError(f"native ID shadows a common ID in {name}")
        recorded = [n for c in CODES for n in repo["status"][c]]
        if len(recorded) != len(set(recorded)) or set(recorded) != ids:
            raise ValueError(f"missing or duplicated feature in {name}")
        qualified = set(repo["status"]["Q"])
        proven = {n for e in repo["qualification_evidence"] for n in e["features"]
                  if e.get("sha") and re.fullmatch("[0-9a-f]{40}", e["sha"]) and
                  e.get("source") and e.get("result")}
        if proven != qualified:
            raise ValueError(f"qualified features and evidence disagree in {name}: {proven ^ qualified}")
        justified = {n for reason in repo["inapplicability"] if reason["basis"] and reason["source"]
                     for n in reason["features"]}
        if justified != set(repo["status"]["N/A"]):
            raise ValueError(f"unjustified inapplicability in {name}")
        blockers = {n for entry in repo["dependencies"] if entry["reason"] for n in entry["features"]}
        if blockers != set(repo["status"]["B"]):
            raise ValueError(f"blocked feature lacks its dependency in {name}")
        count = {c: len(repo["status"][c]) for c in CODES}
        applicable = sum(count.values()) - count["N/A"]
        repo["measurement"] = dict(counts=count, applicable=applicable,
                                   completion_percent=percent(count["Q"], applicable))
        totals.update(count)
        finished += int(applicable > 0 and count["Q"] == applicable)
        blocked += int(count["B"] > 0)
        waiting += int(count["U"] + count["I"] > 0)
    applicable = sum(totals.values()) - totals["N/A"]
    total = len(names)
    data["measurement"] = dict(repositories=total, complete=finished, incomplete=total-finished,
        blocked_repositories=blocked, awaiting_verification=waiting, applicable=applicable,
        counts=dict(totals), repository_completion_percent=percent(finished, total),
        feature_completion_percent=percent(totals["Q"], applicable))


def render(data):
    m = data["measurement"]
    counts = m["counts"]
    rows = ["# Filesystem implementation and qualification register", "",
            f"GitHub inventory and release evidence checked {data['as_of']}. "
            "Each filesystem has the 100 project requirements, one common release regression and its named native requirements. "
            "The JSON records current source, historical evidence, reference media, dependencies and release hashes.", "",
            "[Machine-readable feature register](../data/provider-register.json)", "",
            "| Measurement | Result |", "|---|---:|"]
    for name, value in [("Total filesystem repositories", m["repositories"]), ("Fully completed repositories", m["complete"]),
                        ("Incomplete repositories", m["incomplete"]), ("Repositories with a blocked requirement", m["blocked_repositories"]),
                        ("Repositories awaiting verification", m["awaiting_verification"]), ("Total applicable requirements", m["applicable"])]:
        rows.append(f"| {name} | {value:,} |")
    for c in CODES:
        rows.append(f"| {LABELS[c]} requirements | {counts[c]:,} |")
    rows.extend([f"| Repository completion | {m['repository_completion_percent']:.2f}% |",
                 f"| Feature completion | {m['feature_completion_percent']:.2f}% |", "",
                 "A repository earns completion when every applicable requirement has qualified. Open questions about applicability remain in the denominator. "
                 "Blocked and awaiting-verification counts can overlap. The release column describes the independently inspected public assets.", "",
                 "## Filesystems", "", "| Repository | Main | Release | Q / applicable | M | P | I | F | B | U | N/A | Complete | Central DEB |",
                 "|---|---|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---|"])
    for r in data["repositories"]:
        v = r["measurement"]; c = v["counts"]; sha = r["main_sha"]; rel = r["release"]
        rows.append(f"| [{r['filesystem']}]({r['url']}) | [{sha[:8]}]({r['url']}/commit/{sha}) | "
                    f"[{rel['version']}]({rel['url']}) | {c['Q']} / {v['applicable']} | " +
                    " | ".join(str(c[x]) for x in CODES[1:]) +
                    f" | {v['completion_percent']:.2f}% | {r['central_publication']['state']} |")
    rows.extend(["", "## Programming and qualification still open", "",
                 "The feature IDs under each status in the JSON give the full remaining list. An implemented callback may support only a subset of variants. "
                 "Each entry below identifies the next native programming requirement and the pinned source for its present scope.", "",
                 "| Filesystem | Current format scope | Next native programming or interoperability requirement |", "|---|---|---|"])
    for r in data["repositories"]:
        rows.append(f"| [{r['filesystem']}]({r['scope_evidence']}) | {r['format_scope']} | {r['next_native_work']} |")
    rows.extend(["", "## Supporting repositories", "", "| Component | Repository or directory | Responsibility |", "|---|---|---|"])
    for r in data["supporting_components"]:
        rows.append(f"| {r['component']} | [{r['location']}]({r['url']}) | {r['responsibility']} |")
    rows.extend(["", "## Continuing the register", "",
        "Edit the JSON after qualifying a change. Preserve earlier evidence and add the relevant SHA, regression, workflow, release or media hash. "
        "Run `python3 tools/render_provider_register.py` to recalculate the totals and produce this table. "
        "Use `--check` to verify both files agree. The historical migration matrices remain available in repository history.", ""])
    return "\n".join(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()
    data = json.loads(DATA.read_text())
    calculate(data)
    # Compact lists keep feature IDs, callback masks and test measurements together.
    encoded = json.dumps(data, ensure_ascii=False, indent=2) + "\n"
    encoded = re.sub(r'\[\n\s+((?:"\d+"(?:,\n\s+|\n\s*))+)\]',
                     lambda m: "[" + ", ".join(re.findall(r'"\d+"', m[1])) + "]", encoded)
    doc = render(data)
    if args.check:
        if DATA.read_text() != encoded or not DOC.exists() or DOC.read_text() != doc:
            raise SystemExit("The register or its rendered table is stale")
    else:
        DATA.write_text(encoded)
        DOC.write_text(doc)


if __name__ == "__main__":
    main()
