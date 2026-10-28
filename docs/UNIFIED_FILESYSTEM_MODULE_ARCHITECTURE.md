# Unified filesystem module — authoritative target architecture

**Status: approved destination, not a claim that the loaders or all filesystem capabilities already work.** This contract is the architectural source of truth for Filesystems, each independent Filesystem-* repository, BOOT, the InfiltratorOS kernel, Common (the Libraries repository), Defragmenter, Files, Disks and Setup. Product-specific documents must distinguish this target from the currently shipped Linux/UEFI integrations.

## Non-negotiable product contract

**One on-disk filesystem identity → one authoritative filesystem implementation → one user-distributed, independently installed \`.so\` → any qualified consumer.** FAT12 and FAT16 are different identities and therefore have separate \`FAT12.so\` and \`FAT16.so\`. The default native location is \`/System/Filesystems/<format>.so\`. An end user does not install a per-format \`.ko\`, a per-format \`.efi\`, and a per-format \`.so\` as three filesystem products. Each independent filesystem repository owns its implementation, tests and release.

The filesystem-specific code owns format detection, allocation and directory semantics, file reads/writes, formatting, recovery, defragmentation and format-specific metadata where qualified. Filesystems, BOOT, the kernel and applications **consume** that implementation through versioned contracts. They must not grow their own competing FAT12/FAT16/etc. engines simply to work around current platform interfaces. Shared *platform plumbing* belongs to its appropriate generic loader, not to a copy in each format.

The suffix \`.so\` describes the **one distributed module identity**. It does not require a glibc-dependent Linux shared object to execute unmodified in firmware or the kernel. The eventual representation may be a restricted freestanding ELF profile, a self-describing module with validated executable sections, or another proven single-file format preserving the public \`.so\` contract. Choose based on demonstrable feasibility, compatibility, trust and measurable engineering cost. Do **not** impose three independent filesystem implementations as a prerequisite.

## Generic consumers and their responsibilities

- **BOOT** is the generic UEFI-stage module loader/adapter. It finds a verified firmware-accessible instance of the same \`.so\`, maps/validates its supported execution profile or payload, supplies bounded block I/O and host services, and exposes the required EFI Simple File System functionality. BOOT contains no per-format disk parser. Firmware needs a UEFI-compatible BOOT executable and an appropriate loading mechanism; it cannot directly \`LoadImage\` an arbitrary Linux ELF. BOOT normally requests read-only access for discovering and loading boot assets, even if the engine supports writes elsewhere.
- **The InfiltratorOS kernel** will have a generic filesystem bridge/loader for the same \`.so\` contract. A custom kernel may execute a specially designed module profile directly; an interim Linux kernel may require a **generic** VFS bridge, userspace filesystem service or embedded format-independent translation/loading machinery. Kernel privilege, memory ownership, locking, symbol resolution, early-root access and signing remain explicit engineering constraints. A userspace Linux \`.so\` cannot simply be passed to \`modprobe\` or executed as arbitrary kernel code.
- **Filesystems, Defragmenter, Test Media, Files, Disks and Setup** discover the same trusted module through its versioned ABI. They may supply generic raw-device I/O, safety, privilege and lifecycle services but must not duplicate filesystem-specific parsing or mutation. Independent external tools/fixtures must still verify correctness; one engine cannot be its own only test oracle.
- **Common / Libraries** owns reusable product-neutral ABI definitions, version negotiation, identity/metadata validation and host-service interfaces when those contracts are adopted and tested. Common does **not** own or duplicate filesystem format engines, product policy or an unreviewed privileged loader.

## One module, not necessarily one execution context

The one distributed \`.so\` may internally carry architecture-specific representations, or execute one freestanding core through generic environment adapters, **only if required by verified ABI constraints**. This is an internal implementation choice, not a second product, and not permission to maintain independent FAT logic. The preferred design shares a single format engine. Format-independent BOOT/kernel translators are acceptable; a separate filesystem-specific \`.ko\`/\`.efi\` *distribution* is not the final design.

Bootstrapping must be solved: BOOT cannot find a module stored solely on a filesystem it cannot yet read. A verified *copy of the same module bytes* may be staged on a firmware-readable ESP or in a boot bundle, under controlled install/update rules. This is not a separate filesystem implementation. Linux root-on-that-filesystem additionally requires the module and any generic bridge and dependencies **before root mount** (for example through initramfs). BOOT loading a kernel from a format and the kernel mounting that format as root are **different tests**.

## Compatibility and migration

Existing per-format \`.efi\` UEFI readers, Linux \`.ko\` drivers, distribution filesystem providers and their qualified media/boot paths are **temporary compatibility mechanisms**. Keep them working until the new loader/bridge demonstrates equivalent functionality; do not delete proven source, disable a working boot path, silently write ESP/NVRAM or replace a mounted/root driver to simulate progress. They may be compiled and tested during migration, but must never be described as three required end-user filesystem products.

Current FAT12/FAT16 source/CI includes standalone \`.so\` and platform-specific kernel/EFI work; that evidence **does not mean** BOOT already executes an \`.so\` or that the active Linux kernel consumes it. Do not mark a filesystem complete because its package contains three binaries or because separate tests are green. Documentation of released artifacts and their historical contents must remain accurate until migrated.

## Required evidence before a format is called complete

1. A *single* independently versioned, correctly identified \`.so\`, with tested complete read/write/create/delete/rename/format and any advertised relocation/recovery capabilities, including power-loss/failure recovery limits.
2. Real consumers use this module rather than hidden private implementations; Defragmenter and Test Media pass independent image tests, foreign-format rejection and post-write verification.
3. BOOT loads the appropriate functionality **from the same module** on disposable firmware media, rejects invalid/untrusted binaries, reads a real kernel/initramfs and safely hands off; OVMF, Secure Boot policy and supported physical firmware are qualified separately.
4. The actual OS kernel or its proven generic bridge mounts, reads, writes, remounts and recovers on the target filesystem, including root/initramfs bootstrap if advertised. All supported kernel and CPU ABIs are tested, not assumed.
5. Native install, upgrade, rollback, staged ESP copy, exact bytes/signatures, privileges, unmount/removal and cross-project ownership are tested without surprise format-specific artifacts or unsafe writes.

**Until these tests pass, the single-\`.so\` design is the end goal, not an assertion of present feature completion.** Maintain explicit per-consumer capability and qualification status.
