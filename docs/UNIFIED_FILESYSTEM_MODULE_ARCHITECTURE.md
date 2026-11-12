# Unified filesystem module — authoritative target architecture

**Meaning of “unified”:** *Operations within one filesystem*, not multiple
filesystem formats linked into one driver. `FFS.so` and `exFAT.so` must
always be separately built, packaged, identified and installed from
`Filesystem-FFS` and `Filesystem-exFAT`. The same per-format ownership rule
applies to every other independent repository. Internal regression binaries
(e.g. `FFS-legacy.so`) are not installable products.

**Status: approved destination, not a claim that the loaders or all filesystem capabilities already work.** This contract is the architectural source of truth for Filesystems, each independent Filesystem-* repository, BOOT, the InfiltratorOS kernel, Common (the Libraries repository), Defragmenter, Files, Disks and Setup. Product-specific documents must distinguish this target from the currently shipped Linux/UEFI integrations.

## Non-negotiable product contract

**One on-disk filesystem identity → one authoritative filesystem implementation → one user-distributed, independently installed `.so` → any qualified consumer.** FAT12 and FAT16 are different identities and therefore have separate `FAT12.so` and `FAT16.so`. UFS1 and UFS2 are separately identified on-disk formats and likewise belong in `UFS1.so` and `UFS2.so`; the software interface revision does not define an on-disk format. The default native location is `/System/Filesystems/<format>.so`. An end user does not install a per-format `.ko`, a per-format `.efi`, and a per-format `.so` as three filesystem products. Each independent filesystem repository owns its implementation, tests and release.

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

## Common V2 filesystem contract

Common owns `Libraries/include/infiltratr/filesystem_module_v2.h` and the
`infiltrator_filesystem_api_v2` discovery symbol. Providers advertise their
implemented CORE, VOLUME and optional operation groups. Hosts validate the
descriptor and callbacks, verify the module and execution profile, and keep
device identity and access under their control.

Package inspection checks that the Common export is a defined function and
checks the format-specific ELF identity marker where present. Consumers
validate the actual descriptor and callback groups before use. Mutation
requires an exclusive host lease.

| Consumer | Current Common V2 use | Remaining qualification |
| --- | --- | --- |
| InfiltratorOS | Verifies installed filesystem packages and stages verified modules with BOOT | Release and physical deployment qualification |
| BOOT | Generic ELF loader and read-only UEFI filesystem bridge | Firmware-compatible profiles for additional formats and physical firmware |
| Files | Detects the selected removable partition and calls the installed provider's format operation | Each provider must implement and qualify its own formatter |
| Defragmenter and Test Media | Common V2 volume queries, selected file maps and media fixture operations | Port and qualify the remaining relocation and recovery operations before retiring their dedicated adapters |
| Linux kernel | Stock Linux still uses its existing filesystem drivers | Authenticated in-kernel shared-object loader, native VFS binding and early-root tests |

[The migration matrix](../COMMON_V2_MIGRATION_MATRIX.md) tracks the
independently owned producers and their operation gaps. The
[released-binary audit](INDEPENDENT_COMMON_V2_STATUS.md) records which
capabilities the inspected package bytes actually advertise.
