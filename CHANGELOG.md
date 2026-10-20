# Changelog

## 0.5.45 — 2026-10-20

- Fix EFI driver binding ownership: use DiskIo with EFI_OPEN_PROTOCOL_BY_DRIVER and observe BlockIo non-exclusively, as the UEFI filesystem-driver model requires. OVMF reproduced EFI_ACCESS_DENIED when the previous adapter attempted to claim BlockIo already owned by firmware. Balance the DiskIo claim in failure cleanup and Stop().
- Correct the GNU-EFI entry point calling convention for the standalone OFS EFI driver. An OVMF test reproduced a stall at the driver's first Boot Services call; GNU-EFI's crt0 invokes efi_main via the native C ABI, while the driver previously declared EFIAPI (Microsoft x64). Protocol callbacks retain EFIAPI.
- Extend OVMF validation to assert that Stop() releases the protocol and that a subsequent ConnectController() succeeds; exercise both the opt-in diagnostic binary and the exact uninstrumented production-style binary.
- Add opt-in COM1 stage diagnostics around the OFS UEFI Driver Binding, firmware disk reads and filesystem installation; normal firmware drivers are unchanged by the diagnostic instrumentation.
- Build an isolated QEMU/OVMF acceptance test with a disposable GPT disk, real EFI `LoadImage`/`ConnectController`, OFS Simple File System nested-file reads and non-OFS rejection. Archive serial logs so failures show the last reached stage.
- Preserve OFS as experimental. Virtual firmware acceptance does not prove Dell SD-controller latency, power-cycle behaviour or Boot integration. No EFI System Partition or NVRAM is modified.


## 0.5.44 — 2026-10-20

- Restrict experimental OFS EFI probing to logical partitions, preventing whole-disk controller handles from being treated as partition-relative DOS/0.
- Bound OFS EFI directory lookup and enumeration to 512 hash-chain entries; reject cycles and overlong chains without changing the disk format or Linux module.
- Exercise checksum-valid cyclic chains in the reader regressions and execute them in the standalone OFS EFI workflow.
- Keep the UEFI driver experimental and disabled by default. Synchronous firmware disk I/O has no driver-enforceable wall-clock timeout; compilation and host tests do not qualify real Dell firmware.


## 0.5.43 — 2026-10-19

- Take ownership of the firmware filesystem source formerly held by Boot, preserving its implementations, dormant adapters, licences and pinned InfiltratorFS source.
- Build ext4 and OFS EFI drivers independently of Boot and include their separate component payload in the Filesystems Debian release. Keep OFS explicitly experimental.
- Install driver files under `/usr/lib/filesystems/efi/`; selection and deployment to an EFI partition remain separate installer or local-test operations.

## 0.5.42 — 2026-10-19

- Remove the local SD-card boot-payload preparation helper, its CI references
  and the machine-specific setup procedure from the OFS firmware documentation.
- Retain the OFS implementation and independent UEFI driver build unchanged.

## 0.5.39 — 2026-10-18

- Replace OFS 0.5.38's per-512-byte `sb_breadahead()` submission with bounded
  block-layer BIO reads for **contiguous** bitmap extents. A real 119.1 GiB SD
  card showed that 61,451 individual read-ahead submissions took 14.6s,
  leaving mount time unchanged at ~15s; the earlier 2.3s QEMU result was
  not representative of the physical card.
- Each batched extent contains at most 128 OFS bitmap blocks (64 KiB). The
  driver verifies every AmigaDOS bitmap checksum and counts all free bits
  before completing the mount, preserving read-only and fail-safe behavior.
  Noncontiguous/unsupported extents retain the existing buffer-head reader.
- Add per-mount observability for batch count, validated batched blocks,
  fallback blocks, block-I/O latency, read latency, validation and total time.
- Exercise the batched fast path explicitly with a sparse 120 GiB AmigaDOS
  OFS disk under pinned-kernel QEMU; retain normal Amiga RW and crash tests.
- No change to the on-disk OFS format. Physical SD benchmarks still required.

## 0.5.38 — 2026-10-18

- Improve large AmigaDOS OFS mount performance by resolving bitmap keys once
  and issuing a bounded 128-block asynchronous read-ahead window. Continue
  verifying each bitmap checksum and counting every allocation bit before
  exposing the filesystem; reject bitmap pointers outside the partition.
- Emit phase timings for mounts with at least 4096 bitmap blocks: key traversal,
  read-ahead submission, synchronous reads, checksum validation, free-bit
  counting and overall bitmap initialization. No new on-disk metadata.
- Qualify a sparse 120 GiB OFS image in QEMU with the native driver and
  read-only mount, verifying capacity and accounting before running the
  existing small-volume read/write and crash-recovery tests.
- Keep the physical SD card read-only until performance and integrity are
  independently rechecked on the Latitude.

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
