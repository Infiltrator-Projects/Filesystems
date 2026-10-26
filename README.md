# Filesystems

Filesystems is the InfiltratorOS filesystem capability and integration manager. It owns the catalogue of real persistent local-storage filesystem formats that InfiltratorOS can recognise, install support for and, as each format is qualified, use as the filesystem beneath an InfiltratorOS installation.

The application and repository are named **Filesystems**. **Boot** starts the
selected operating system, and **Setup** is the name reserved for the future
installer. **InfiltratorFS** remains a distinct filesystem format in the catalogue.

The authoritative source/release version is stored in [`VERSION`](VERSION). Published releases use the corresponding `v<version>` tag only after that exact source revision and its release artefacts have completed qualification.

The baseline Linux distribution is **Debian stable**. As of September 2026 that is Debian 13 "trixie". Derivatives can benefit where they retain Debian-compatible package names and APT behaviour, but the catalogue is authored and qualified against Debian first.

## Actual copied filesystem implementations

The 41-format catalogue is distinct from the source inventory. **The original
Linux implementation code for 26 additional formats is checked in under
`native/filesystems/<id>/linux/` as editable working source**, including NTFS
recovered from the historical NTFS3 source tree. These are not frozen references
and an automated upstream refresh is not permitted to overwrite modifications.

The existing nine individually managed native modules remain governed by
`data/native-filesystems.tsv`; simply having original code does not qualify a
module for installation. See
[`docs/SOURCE_BASELINE_STATUS.md`](docs/SOURCE_BASELINE_STATUS.md) for the
seven formats whose complete source baselines are still absent from this
repository. Neither design documents nor Debian provider names are counted as
copied implementation source.

## Product scope

The catalogue contains **41 real on-media filesystem formats**. A catalogue row is a filesystem format identity, not a Linux provider or a generic mount mechanism.

In scope are persistent local-storage formats such as InfiltratorFS, EXT2/3/4, XFS, Btrfs, FAT, exFAT, NTFS, HFS/HFS+, APFS, UDF, Amiga OFS/FFS/SFS/SFS2/PFS3, historical workstation formats and other genuine disk filesystems.

Out of scope are network and cloud mounts, distributed client protocols that do not define the selected local disk format, overlay/union filesystems, archive mounts, device namespaces, virtual filesystems, encryption/container layers that merely expose another filesystem, tools that are not a filesystem format, and duplicate provider identities such as `ntfs3`, `zfs-fuse`, `apfs-fuse` or `apfs-dkms`.

Provider choice is subordinate to filesystem identity. NTFS is one filesystem whether the current Linux access path is NTFS-3G, NTFS3 or a future Infiltrator-owned adapter. APFS is one filesystem whether accessed through a FUSE reference provider or a kernel driver. The user chooses a format, not an implementation brand.

The complete enumerated contract is maintained in [`docs/FILESYSTEMS_MATRIX.md`](docs/FILESYSTEMS_MATRIX.md). CI verifies that every compiled catalogue ID appears there exactly once.

## Drop-in filesystem modules — target architecture

The intended native installation model is **one drag-and-drop file per on-media
filesystem identity, directly under `/System/Filesystems/`**, following the
discoverable Calendar/Clock module model. A compatible and trusted
`FAT12.so`, `FAT16.so`, `FAT32.so`, `exfat.so`, `ofs.so` or `ffs.so` advertises capabilities to
Filesystems, Files, Defragmenter, Test Media and eventually Boot without
separate per-application registration. **Install FAT12** places only `FAT12.so`
in the directory. FAT16 and FAT32 have their own independent single-file
installations; all three reuse common FAT *source code*, not a combined
`fat.so` binary or a shared runtime dependency.

The filesystem-specific parsing, allocation, formatting, write/verification and
defragmentation primitives are to be consolidated from the existing Filesystems,
Defragmenter and Test Media implementations **without dropping behaviour**.
Defragmenter retains its generic safety, placement and transaction authority.

This is **a design target, not implemented drop-in support**. Ordinary Linux
`.so` files are not kernel drivers or UEFI images. Boot's proposed restricted
ELF loader must first be engineered and qualified; its initial reader must be
reachable before the root filesystem can be read. Existing Linux `.ko` and
firmware `.efi` deployment remain necessary during migration. The working
InfiltratorFS driver and physical test media must not be disturbed.

See the [Drop-in Filesystem Module Contract](docs/DROP_IN_FILESYSTEM_MODULES.md)
for the module ABI, capability model, Boot bootstrapping, privilege boundaries,
deduplication plan and qualification gates.

## Desktop integration ownership

GNOME Disks, Nemo and the native filesystem manager are distinct integration
surfaces. Filesystems only owns a `dpkg-divert` launcher when no other package
already owns that launcher. In particular, systems with InfiltratorFS's GNOME
Disks diversion retain its existing binary, formatting support and ownership
unchanged. Filesystems remains installable and its independent CLI formatter,
kernel module support and device-identification rules still work.

If another package owns a launcher, Filesystems does **not** chain or inject its
desktop preload into that application: Filesystems-specific visual presentation
inside that application is unavailable until an independently designed shared
integration interface exists. It does not compromise the other package to
provide cosmetic support. Missing or inconsistent foreign launcher state is
reported instead of overwritten. The ownership contract is regression-tested
using independent private `dpkg-divert` databases.

## Two product surfaces, one identity contract

Filesystems has different presentation contexts that consume the same stable filesystem identities while keeping platform-specific support policy separate.

The normal **desktop manager** combines the identity contract with Linux support policy: it discovers support already present on the running system, installs or removes the current provider for a filesystem, loads or unloads eligible kernel modules, reports limitations and integrates supported formats with the desktop storage stack.

The planned **Setup** installer has its own interface. During operating-system installation it will ask which qualified filesystem should be used for the target storage and drive the installation transaction through the Filesystems setup capability contract. Windows, EFI and setup consumers use the identity-only API rather than inheriting Debian package or Linux module policy.

The intended setup flow is:

```text
choose target filesystem
        |
        v
format the target storage in that format
        |
        v
install/copy InfiltratorOS
        |
        +--> install the matching Linux VFS support
        |    and include it in the initramfs when required
        |
        v
boot the installed system
```

The agreed cross-project separation of **kernel stored on a filesystem** from **operating system rooted on a filesystem**, together with modular `.efi` / `.ko` handling, capability adaptation and phased tests, is recorded in [the Bootable Filesystem Integration Contract](docs/BOOTABLE_FILESYSTEM_CONTRACT.md).

The setup surface exposes only filesystems whose complete install path is qualified. It does not infer support merely because a catalogue entry exists or because a Linux package can be installed.

## OFS specification-completeness audit

The current OFS implementation is **not yet complete** across all documented
512..32768-byte filesystem block sizes or full unsigned 32-bit addressing.
The image formatter has experimental block-size/large-volume support; Linux,
EFI and physical formatting remain separately gated. See
[OFS specification-completeness audit](docs/OFS_SPECIFICATION_COMPLIANCE.md).

## AmigaDOS OFS partition formatting (experimental)

A deliberately explicit Linux-only mode can now format an already-created
**unmounted block partition**, not a complete disk:

```sh
sudo filesystems-ofs-image --format-partition /dev/mmcblk0p1 TestOFS --confirm /dev/mmcblk0p1
sudo filesystems-ofs-image --verify-partition /dev/mmcblk0p1
```

Those commands are **destructive** and require prior device identity
verification. The formatter refuses a whole disk, mounted/busy partitions,
non-512-byte logical sectors, unsupported sizes, and paths without matching
confirmation. It does not create a partition table, mount media, install a
kernel or provide an OFS UEFI reader. For SD-card-class devices, the experimental formatter uses the whole
confirmed partition when requested; it does not impose a 2 GiB volume limit.
Its supported range is 1 MiB to the shared 31-bit block-count boundary
(just under 1 TiB with 512-byte blocks). A sparse 120 GiB regression image
exercises extended bitmaps without allocating 120 GiB of host storage.
Actual SD-card formatting remains destructive and unqualified until
independent physical-media validation.
This is developer-preview functionality, not a general-purpose supported
physical-drive formatter. The block-partition path must be independently
qualified on expendable physical media before it can be promoted.

## AmigaDOS OFS test-image creation (developer preview)

The Filesystems package supplies `filesystems-ofs-image`, an independent
AmigaDOS OFS creator and verifier. In image mode it creates **new regular-file
images only**, refusing existing paths, symlinks and block devices, then
verifying its result before reporting success. Its separate, confirmed
partition mode is documented above. Neither mode partitions an entire drive
nor places a kernel into the filesystem.

```sh
filesystems-ofs-image --create ofs-test.img 64 TestOFS
filesystems-ofs-image --verify ofs-test.img
```

The `64` is MiB (1–1048575 supported by the current block-count architecture). These image files
are suitable for controlled VM/loopback testing. Root filesystem and Boot EFI
driver qualification remain separate. See
[Amiga filesystem architecture](docs/AMIGA_FILESYSTEMS.md).

## Support discovery

Filesystems treats support as a capability that must be observed rather than assumed.

The Linux manager scans the running system for:

- kernel-registered filesystems;
- loaded and available kernel modules;
- installed Debian packages;
- repository package availability;
- project-owned native module state;
- desktop identification/integration state.

Probe failures are explicit. A failed package, repository, module or kernel inventory is not converted into a normal "missing" result and does not enable an action based on incomplete state.

## Installation and removal

Install and remove operations are scoped to the selected filesystem. Project-native filesystems deploy their own kernel driver and their own desktop-identification material. Package-backed filesystems use the selected provider package without turning provider package names into new filesystem identities.

Destructive package removal is bound to the exact installed package names and architectures shown for confirmation. The installer repeats its simulation for early feedback, then an installed APT guard checks the final removal actions and Debian protection metadata under APT's frontend lock. A changed plan is refused before any package removal, including changes made while authentication was pending.

Privileged executable selection is fixed to trusted system/application locations rather than inherited from the caller's `PATH`, and privileged operations use structured argument vectors. APT's required shell-hook interface receives a fixed installed guard path with individually quoted package identities; it never receives arbitrary command text.

## Project-native Linux filesystems

The project currently owns native Linux implementations for:

- EXT2
- EXT3
- EXT4
- Amiga OFS
- Amiga FFS
- SFS
- SFS2
- PFS3

Each native filesystem is an independent deployment unit. Installing or removing one must not silently install, remove, unload or mutate another filesystem's driver.

Kernel-module replacement is transactional. If replacement fails, rollback restores both the previous persistent module image and the previous runtime selection when that module had been loaded.

## Desktop integration

Desktop identification is a separate transaction from kernel-driver deployment. It publishes filesystem identity to udev/UDisks and desktop tools without making desktop metadata authoritative over kernel support.

A successfully committed native driver is not rolled back merely because a later desktop-integration refresh fails; that partial success is reported explicitly so the failing layer can be repaired independently.

## Windows

The Windows tree contains the canonical project EXT2 driver build path and packaging/validation material. Windows support is maintained as a separate platform surface rather than by importing Debian/Linux package policy.

## Build

A normal Linux build uses CMake/Ninja:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Release qualification additionally exercises project-native filesystem builds, sanitizers/adversarial parser tests, runtime qualification, packaging, desktop-identification validation and Windows EXT2 code analysis.

## Packaging

The release produces one Debian package and one self-extracting `.run` installer. Per-filesystem desktop templates are carried inside the main package and activated only for the selected filesystem; there is no supplemental desktop/UDisks Debian package.

The central Infiltrator repository publication path accepts only an exact qualified release revision and verifies the advertised application/version after publication.

## Architecture documentation

Key contracts are documented under `docs/`, including:

- `FILESYSTEMS_MATRIX.md` — enumerated filesystem contract and status;
- `DROP_IN_FILESYSTEM_MODULES.md` — drag-and-drop single-file modules, shared Defragmenter/Test Media capabilities and future Boot loader;
- `BRITTLENESS_REDUCTION.md` — ownership, transaction, coupling and failure-reporting boundaries;
- platform/build/runtime qualification documents and implementation notes.

The implementation deliberately keeps filesystem semantics, support policy, privileged mutation, desktop presentation and setup capability as separate responsibilities so that adding or repairing one filesystem does not create hidden coupling to unrelated formats.

[Validation](docs/VALIDATION.md) defines local tests, mounted-runtime evidence and immutable artifact publication. [PHILOSOPHY.md](docs/PHILOSOPHY.md) owns the engineering standard.

## EFI filesystem components

Firmware filesystem implementations are maintained by Filesystems, independently
of Boot. The migrated adapters and provenance are documented in
[`firmware/uefi/README.md`](firmware/uefi/README.md). The Debian release carries
the ext4 reader and the separately marked experimental OFS reader under
`/usr/lib/filesystems/efi/`. Installing Filesystems does not copy modules into an
EFI partition or alter a boot-manager installation. Setup, or an explicit local
test, selects the required drivers for its chosen target directory.
