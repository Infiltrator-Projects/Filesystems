# Changelog

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
