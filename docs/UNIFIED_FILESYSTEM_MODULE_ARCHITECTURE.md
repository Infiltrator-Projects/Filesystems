# Unified filesystem module — authoritative target architecture

**Meaning of “unified”:** *Operations within one filesystem*, not multiple
filesystem formats linked into one driver. `FFS.so` and `exFAT.so` must
always be separately built, packaged, identified and installed from
`Filesystem-FFS` and `Filesystem-exFAT`. The same per-format ownership rule
applies to every other independent repository. Internal regression binaries
(e.g. `FFS-legacy.so`) are not installable products.

**Status: approved destination, not a claim that the loaders or all filesystem capabilities already work.** This contract is the architectural source of truth for Filesystems, each independent Filesystem-* repository, BOOT, the InfiltratorOS kernel, Common (the Libraries repository), Defragmenter, Files, Disks and Setup. Product-specific documents must distinguish this target from the currently shipped Linux/UEFI integrations.

## Non-negotiable product contract

**One on-disk filesystem identity → one authoritative filesystem implementation → one user-distributed, independently installed `.so` → any qualified consumer.** FAT12 and FAT16 are different identities and therefore have separate `FAT12.so` and `FAT16.so`. UFS1 and UFS2 are separately identified on-disk formats and likewise belong in `UFS1.so` and `UFS2.so`; the software interface ABI V1/V2 numbering does not define an on-disk format. The default native location is `/System/Filesystems/<format>.so`. An end user does not install a per-format `.ko`, a per-format `.efi`, and a per-format `.so` as three filesystem products. Each independent filesystem repository owns its implementation, tests and release.

The filesystem-specific code owns format detection, allocation and directory semantics, file reads/writes, formatting, recovery, defragmentation and format-specific metadata where qualified. Filesystems, BOOT, the kernel and applications **consume** that implementation through versioned contracts. They must not grow their own competing FAT12/FAT16/etc. engines simply to work around current platform interfaces. Shared *platform plumbing* belongs to its appropriate generic loader, not to a copy in each format.

The suffix `.so` describes the **one distributed module identity**. It does not require a glibc-dependent Linux shared object to execute unmodified in firmware or the kernel. The eventual representation may be a restricted freestanding ELF profile, a self-describing module with validated executable sections, or another proven single-file format preserving the public `.so` contract. Choose based on demonstrable feasibility, compatibility, trust and measurable engineering cost. Do **not** impose three independent filesystem implementations as a prerequisite.

## Generic consumers and their responsibilities

- **BOOT** is the generic UEFI-stage module loader/adapter. It finds a verified firmware-accessible instance of the same `.so`, maps/validates its supported execution profile or payload, supplies bounded block I/O and host services, and exposes the required EFI Simple File System functionality. BOOT contains no per-format disk parser. Firmware needs a UEFI-compatible BOOT executable and an appropriate loading mechanism; it cannot directly `LoadImage` an arbitrary Linux ELF. BOOT normally requests read-only access for discovering and loading boot assets, even if the engine supports writes elsewhere.
- **The InfiltratorOS kernel** must load and execute the canonical filesystem `.so` directly in kernel context through a kernel-owned, format-neutral, verified executable loader and native VFS bindings. No FUSE, CUSE, virtiofs, libfuse, userspace filesystem service, daemon relay, mount proxy, equivalent translation layer or separate filesystem engine is permitted, including as a transitional shortcut. Kernel privilege, memory ownership, locking, symbol resolution, early-root access and signing remain explicit engineering constraints. An ordinary glibc-dependent Linux `.so` cannot simply be passed to `modprobe`; the direct loader and module execution profile must be implemented and proven. If it does not work yet, record the kernel integration as unfinished rather than substituting a userspace filesystem transport.
- **Filesystems, Defragmenter, Test Media, Files, Disks and Setup** discover the same trusted module through its versioned ABI. They may supply generic raw-device I/O, safety, privilege and lifecycle services but must not duplicate filesystem-specific parsing or mutation. Independent external tools/fixtures must still verify correctness; one engine cannot be its own only test oracle.
- **Common / Libraries** owns reusable product-neutral ABI definitions, version negotiation, identity/metadata validation and host-service interfaces when those contracts are adopted and tested. Common does **not** own or duplicate filesystem format engines, product policy or an unreviewed privileged loader.

## One module, not necessarily one execution context

The one distributed `.so` may internally carry architecture-specific representations, or execute one freestanding core through generic environment adapters, **only if required by verified ABI constraints**. This is an internal implementation choice, not a second product, and not permission to maintain independent FAT logic. The preferred design shares a single format engine. BOOT and Kernel may have their own minimal, native, in-context host adapters for firmware and VFS APIs, but must directly execute the same trusted module without userspace round-trips or an intermediary filesystem service. A separate filesystem-specific `.ko`/`.efi` *distribution* is not the final design.

Bootstrapping must be solved: BOOT cannot find a module stored solely on a filesystem it cannot yet read. A verified *copy of the same module bytes* may be staged on a firmware-readable ESP or in a boot bundle, under controlled install/update rules. This is not a separate filesystem implementation. Linux root-on-that-filesystem additionally requires the canonical module and direct kernel loader **before root mount**; merely staging a userspace loader in initramfs does not satisfy the contract. BOOT loading a kernel from a format and the kernel mounting that format as root are **different tests**.

## Compatibility and migration

Existing per-format `.efi` UEFI readers, Linux `.ko` drivers, distribution filesystem providers and their qualified media/boot paths are **temporary compatibility mechanisms**. Keep them working until the new loader/bridge demonstrates equivalent functionality; do not delete proven source, disable a working boot path, silently write ESP/NVRAM or replace a mounted/root driver to simulate progress. They may be compiled and tested during migration, but must never be described as three required end-user filesystem products.

Current FAT12/FAT16 source/CI includes standalone `.so` and platform-specific kernel/EFI work; that evidence **does not mean** BOOT already executes an `.so` or that the active Linux kernel consumes it. Do not mark a filesystem complete because its package contains three binaries or because separate tests are green. Documentation of released artifacts and their historical contents must remain accurate until migrated.

## Transitional filesystem build outputs

Use one destination per output across the independent filesystem providers. Each output is a regular file.

| Output | Build staging path |
| --- | --- |
| PDF manual | `$DESTDIR/System/Resources/Documentation/<provider-repository>/<manual>.pdf` |
| Canonical shared module | `$DESTDIR/System/Filesystems/<catalogue-filename>.so` |
| Qualified Linux module | `$DESTDIR/System/Kernel/Filesystems/<provider-id>.ko` |

The published Debian package installs the shared module, generated manual and qualified Linux module at these three paths. A manual `.run` compiles the shared module and Linux adapter locally, generates the PDF from its editable source, and replaces the package's files. It uses the running kernel's prepared headers by default and accepts `--kernel-build DIRECTORY` for an explicit build target.

InfiltratorOS owns the selective update policy. It builds a missing or older shared module, verifies and repairs the manual independently, and retains a valid package-owned kernel module from the same release. Its `--system-package-mode` build supplies the shared module and manual; the OS worker packages them with the retained or matching published kernel module, keeping ownership and all three checksums in Debian metadata. Missing-SO repair reads kernel provenance independently. Kernel-only repair reuses the verified SO and PDF without recompiling them.

Keep the `.so` and `.ko` tied to the same provider release. Build and qualify a single amd64 Linux module for the project Linux `6.12.107` target. Publish it as `<provider-id>-linux-6.12.107.ko` with its SHA-256 alongside the provider’s normal source installer and Debian package. InfiltratorOS uses the release digest to stage the module under its stable filename, such as `ofs.ko`, for manual use. The release asset records the kernel target. The installed filename is stable across upgrades.

Twenty-one providers build Linux adapters against the reference kernel. Linux SWAP uses the kernel’s built-in swap implementation; ZFS has no qualified Linux adapter yet.

## Required evidence before a format is called complete

1. A *single* independently versioned, correctly identified `.so`, with tested complete read/write/create/delete/rename/format and any advertised relocation/recovery capabilities, including power-loss/failure recovery limits.
2. Real consumers use this module rather than hidden private implementations; Defragmenter and Test Media pass independent image tests, foreign-format rejection and post-write verification.
3. BOOT loads the appropriate functionality **from the same module** on disposable firmware media, rejects invalid/untrusted binaries, reads a real kernel/initramfs and safely hands off; OVMF, Secure Boot policy and supported physical firmware are qualified separately.
4. The actual OS kernel or its proven generic bridge mounts, reads, writes, remounts and recovers on the target filesystem, including root/initramfs bootstrap if advertised. All supported kernel and CPU ABIs are tested, not assumed.
5. Native install, upgrade, rollback, staged ESP copy, exact bytes/signatures, privileges, unmount/removal and cross-project ownership are tested without surprise format-specific artifacts or unsafe writes.

**Until these tests pass, the single-`.so` design is the end goal, not an assertion of present feature completion.** Maintain explicit per-consumer capability and qualification status.

## Common V2 migration checkpoint — 10 November 2026

**Architecture decision:** `Libraries/include/infiltratr/filesystem_module_v2.h`
is the existing canonical forward ABI, not a reason to create a third API.
V1 remains binary immutable. V2 offers negotiated, truthful handles,
metadata, volume, defragmentation, streams, and snapshots groups with a
freestanding host-services descriptor. All consumers must reject invalid
capability/callback combinations, protect device identity, and independently
verify module authenticity and the applicable execution profile.

**Verified source changes on main (not all installed/released):**

| Repository | Common ABI work | Qualification boundary |
| --- | --- | --- |
| Libraries | V2 host validation, version/group negotiation, safe V2-to-V1 and format-host conversions | Common CI builds and unit tests; does not load an ELF in kernel or firmware |
| Filesystem-FAT12 | Existing canonical V1 + negotiated V2 volume check/repair/format/sync | FAT12 media qualification; no general handle/streams/metadata group |
| Filesystem-FAT16 | V1 canonical FatFs file operations, V2 volume sync/format | Independent filesystem CI; other V2 groups unimplemented |
| Filesystem-FAT32 | V1 canonical FatFs file operations, V2 volume sync/format | Independent filesystem CI; other V2 groups unimplemented |
| Filesystem-OFS | V1 canonical native engine adapter, V2 volume sync/check/format | Standalone boot/kernel evidence is separate; no V2 recover claim |
| Filesystem-exFAT | V1 64-bit file operations, V2 hosted volume sync/format | No direct kernel-execution claim; separate exfat.ko remains transitional |
| Filesystem-FFS | V1 native reader and bounded overwrite, V2 hosted volume query | No unsupported create/format/recovery claim |
| Files | Privileged FAT12 formatter negotiates V2 volume format; retains controlled V1 fallback | Physical removable-partition checks remain in host; UI is still FAT12-specific |

**Still unfinished:** The other independently owned formats need real V2
adapters and independent test-image evidence. Files needs format-neutral
discovery, file/directory operations, volume mapping and a generic privileged
host rather than only the FAT12 action. Defragmenter needs to consume the
negotiated V2 defrag group instead of native per-format interfaces once each
provider implements qualified planner/move/verification callbacks. BOOT
already has a tested freestanding ELF loader for its existing Boot ABI;
adopting V2 negotiation must preserve that verified bootstrap. The Kernel
still needs an authenticated, direct, in-kernel loader for the single .so
execution profile, native VFS bindings, safe concurrency and early-root
bootstrap. Do not use FUSE/daemon proxies or delete any working .ko to
pretend this is done.

**Migration order:** (1) common version/group validation and host services;
(2) one provider proving real V2 operations on disposable media; (3) each
consumer implementing verified negotiation without copied format engines;
(4) remaining provider migrations, preserving independent ownership;
(5) direct kernel execution and root-mount qualification; (6) retire
transitional per-format drivers only after measured equivalence.
Unsupported capabilities must be omitted, not supplied by no-op stubs.

## Implementation pilot: FAT12 only (9 November 2026)

The first concrete Common ABI adoption is in `Filesystem-FAT12`. Its **one** `FAT12.so` exports the format-neutral `infiltrator_filesystem_api_v1` defined by Common/Libraries, alongside the currently tested FAT12-specific V1/V3/V4/Defragmenter/Boot interfaces. Common V1 presently exposes FAT12 **probe and constrained reformat only**, not mount/list/read/write/check/repair. Its format callback calls the existing FAT12 V4 engine rather than implementing another formatter. This first reformat adapter requires exclusive access to an **already FAT12** partition; blank-volume geometry and volume-label support need the next versioned host-contract pass.

Migration gates, in order: (1) prove module provenance, identity and capability validation; (2) add Common V1 mount/file operations reusing the authoritative engine and independent image tests; (3) implement true format-specific fsck/check/repair and Defragmenter operation contracts, rejecting unsupported claims; (4) connect Files and other graphical consumers through a trusted generic host, resolving right-click items to their **backing volume** and obtaining exclusive unmounted access for raw mutations; (5) implement and qualify the **generic** kernel ELF/VFS bridge with root/initramfs bootstrapping, failure isolation and physical-media evidence. BOOT's existing verified read-only SO loader remains a separate, already established consumer. None of these gates authorises removal of the Linux `.ko` before equivalent kernel-backed operation is proven.

The single installed module directory is `/System/Filesystems/`; per-format `.so` files own their on-disk algorithms. Host code must never copy those algorithms into GUI, Kernel, BOOT, Defragmenter or Common. **Common's existing ABI is canonical**; do not create another competing filesystem discovery standard in a consumer.


## Strict Common V2-only cutover — 10 November 2026

**The software filesystem ABI has no backward-compatibility layer.** The only
public discovery symbol is `infiltrator_filesystem_api_v2`, and both the module
descriptor and every advertised group use the exact Common V2 revision.
Common V1, format-specific V1 worker tables, and silent legacy fallback are
retired; adapters must not reinterpret an old pointer table as V2. A future
Common V3 will likewise be an explicit breaking migration, not a promise to
keep V2 modules loadable.

For userspace package observation, require both a **defined** V2 function
export and the format-specific, non-executable
`IFS-MODULE:<format-id>:common2` ELF identity marker, in addition to verified
package ownership and SHA-256. Read-only ELF inspection does not itself
authorize executing, mounting, formatting or repairing the module. Actual
consumer loading additionally requires trusted bytes, a validated executable
profile, complete callback groups and a genuine exclusive host lease for
destructive operations.

A V2 provider advertises **only functions which actually work**: read-only
probe may be qualified before formatting or defragmentation, but must never be
reported as complete functionality. The direct kernel ELF/VFS loader and
boot-time module execution remain engineering deliverables, not consequences of
adding a V2 symbol or passing native .so CI. Coordinated breaking-change PRs
remain staged until every affected consumer and required provider has passed
real-media and platform integration tests; an isolated green build is not a
system-wide cutover.
