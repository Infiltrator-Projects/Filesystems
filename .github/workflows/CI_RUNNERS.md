# GitHub Actions runner placement (Filesystems)

Filesystems uses a deliberately hybrid CI fleet. Routing is a policy decision,
not a request to run every project job on whatever runner is idle.

| Lane | Execution | Reason |
| --- | --- | --- |
| Pull-request builds and source checks | GitHub-hosted Ubuntu 24.04 | Untrusted changes must never run on private build hosts. |
| Main-branch Linux 6.12 native-module API build | Self-hosted Linux/X64/linux-native/bigbedroom | Trusted repeat native compilation on an existing project build machine. |
| Main-branch Linux 7.0 Amiga-module API build | Self-hosted Linux/X64/linux-native/bigbedroom | Trusted repeat native compilation on an existing project build machine. |
| Amiga and EXT image/boot/runtime qualification | GitHub-hosted Ubuntu 24.04 | Requires QEMU, preinstalled dependencies and many temporary image paths; keep on isolated hosted runners until separately hardened. |
| Release packages, Windows builds and central APT publication | GitHub-hosted | Clean independent qualification, controlled release, and reproducible publication. |

BigBedroom routing is enabled for trusted main builds following the
owner-approved installation and verification of `libelf-dev` and `libssl-dev`
on the BigBedroom host (18 October 2026). To immediately force all jobs
back to GitHub-hosted runners, set the repository Actions variable
`FILESYSTEMS_BIGBEDROOM_READY` to exactly `false`. When this value is unset
or different, only trusted main pushes use the labelled BigBedroom fleet.
The manually dispatched `BigBedroom runner preflight` remains read-only;
CI never installs packages onto private machines.

The known Linux labels are used by the organisation's other projects:
`[self-hosted, Linux, X64, linux-native, bigbedroom]`. RunnerScope counts
alone are not proof that this exact label set is registered in Filesystems'
GitHub runner group. Check the workflow job's actual runner name on the first
trusted main push before claiming that routing works.

## Non-negotiable safety rules

- Only trusted `push` to `refs/heads/main` requests self-hosted runners unless the emergency disable flag is set, in the two
  module API jobs. A PR, manual run or future new event stays GitHub-hosted.
- Self-hosted jobs must not execute `sudo apt-get install` or mutate machine
  configuration. Check for prerequisite tools and libraries and fail clearly
  if the runner is not provisioned. Install missing tools manually, with the
  machine owner's approval, outside GitHub Actions.
- Source builds and logs use per-run `RUNNER_TEMP`, not hardcoded shared
  `/tmp/linux` paths. Do not clean a global cache or shared work tree.
- A private self-hosted runner group must be restricted to approved repos,
  workflows and trusted users in GitHub settings. YAML event checks are
  necessary but **not sufficient** to contain malicious changes to workflow
  files in an untrusted PR.
- Do not add QEMU, KVM, libvirt, containers, or kernel packages automatically
  to personal workstations. The boot/runtime lanes stay hosted until a
  dedicated build host has a validated sandbox and per-job image isolation.
- Keep the exact source/version/release dependency graph unchanged.
  Publication must never require access to an individual's home machine.

The routing and host-safety checks are recorded in
`.github/workflows/runner_policy_test.py`. The first change is intentionally small.
Only after the native-module lanes have run successfully on trusted BigBedroom
hardware should the team consider a host-local, pinned kernel tree cache.
Caches must be per physical host, keyed by source commit, toolchain and kernel
configuration, and locked across concurrent runner processes. Do not blindly
reuse incompatible kernel trees.

If the named home runner is unavailable or missing toolchain prerequisites,
a trusted main build may queue or fail; check RunnerScope and GitHub's assigned
runner name rather than changing the workflow to run unsafe software on a
personal laptop. Hosted PR/release testing remains available.
