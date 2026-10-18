# Changelog

## 0.5.37 — 2026-10-18

- Fix the native Linux kernel-module installer incorrectly hashing relative source
  file paths from the caller's working directory instead of the installed
  source directory. This caused `sha256sum: native/filesystems/...: No such
  file or directory` and prevented `ofs.ko` from building on the Latitude.
- Scope source enumeration and checksum execution to one consistent package
  working directory without changing the caller's directory.
- Add a read-only `native-module-helper source-digest <filesystem-id>`
  diagnostic and CTest regression coverage for unrelated working directories,
  source edits, ignored documentation and missing sources.
- Correct OFS Linux statfs reporting 100% used on a read-only freshly formatted
  SD card. Read-only mounts now validate and count the existing bitmap while
  preserving media bytes; read-only remounts retain the accounting and only
  read-write operations may normalise unused tail bits.
- Extend the QEMU Amiga runtime qualification to require nonzero free blocks
  from a true read-only OFS mount before any media writes. No device contents,
  boot paths or filesystem on-disk structures change.

## 0.5.36 — 2026-10-18

- Fix package configuration on systems where InfiltratorFS already owns the
  GNOME Disks diversion. Filesystems leaves the foreign launcher and diversion
  untouched while retaining its own formatter, native modules, udev rules and
  independently managed Nemo integration.
- Make shared launcher setup, per-filesystem activation and package removal
  respect independent diversion ownership. An incomplete foreign launcher
  remains a clear, non-destructive error.
- Regression-test both foreign GNOME Disks and foreign Nemo ownership,
  per-filesystem activation and safe removal without taking over their binaries.
- Filesystems-specific preload presentation in GNOME Disks is unavailable
  while another package owns that launcher; formatting and native support are
  independent of that presentation layer.

## 0.5.35 — 2026-10-18

- Remove the arbitrary 2 GiB OFS capacity limit for block-partition formatting;
  support legal 512-byte, 31-bit geometries including full 120 GiB volumes.
- Use bitsets for OFS bitmap occupancy verification and image creation, and
  qualify a sparse 120 GiB OFS image in CI.

## 0.5.34 — 2026-10-18

- Add an explicitly confirmed Linux **block-partition** OFS formatter/validator
  mode alongside the non-overwriting image creator. Whole disks, mounted/busy
  partitions, non-512-byte logical sectors and unsupported geometries fail closed.
- Extend OFS geometry test coverage to 2 GiB and add non-partition and
  incorrect-confirmation rejection tests.
- Keep direct SD-card runtime qualification, OFS EFI reading and Linux
  kernel loading separately gated; physical partition formatting remains
  an experimental, opt-in operation rather than a desktop default.

## 0.5.33 — 2026-10-18

- Add the native, image-only AmigaDOS OFS creation and verification tool. It refuses block devices, symlinks and existing files, verifies block metadata and deletes an incomplete newly created image on failure.
- Regression-test bitmap-extension geometry, corrupt media, invalid parameters and non-overwrite safety.
- In release runtime qualification, mount the image created by Filesystems' own tool and compare it byte-for-byte to the pinned independent Defragmenter formatter before exercising the native OFS Linux module.
- This preview is limited to regular test images and does not enable physical-volume formatting or claim a qualified UEFI OFS reader.

## 0.5.32 — 2026-10-18

- Record the shared filesystem-neutral Boot/Setup installation contract and its staged Amiga OFS boot-payload acceptance test.
- Clarify the difference between Boot's EFI readers and kernel-side filesystem modules, and between a readable kernel source and a qualified operating-system root.
- Link the long-term native filesystem design to cross-project integration ownership; use the current Boot product name rather than inherited rEFInd naming.
- No on-disk format, filesystem algorithm, formatter or installed driver behaviour changes.

## 0.5.30 — 2026-10-17

- Pin Libraries 1.19.46 and use canonical shared settings identities.
- Build independent test media with the canonical Defragmenter helper targets.

## 0.5.28

- Move the governing engineering philosophy into `docs/PHILOSOPHY.md` and update maintained documentation references.

## 0.5.27

- Isolate source-contract scratch files so concurrent checks cannot overwrite each other's evidence.
- Pin external CI actions to immutable reviewed commits.
- Stage and compare release drafts before publication; retain all exact-source runtime gates.
- Add the maintained validation entry point.

## 0.5.26

- Rename the application and repository to Filesystems.
- Update desktop and Windows titles, installer text, installed paths, documentation and release filenames.
- Publish one current identity throughout: filesystems for the Debian package and executable, and org.infiltrator.Filesystems for the desktop application.
- Name the planned InfiltratorOS installer Setup and the firmware boot component Boot.
