# Changelog

## 0.5.48 — 2026-10-21

- Guard OFS superblock commits against invalid root buffers following storage-device loss.
- Improve synchronous superblock error propagation and delayed-work failure reporting.
- Build-validated the Linux OFS module against kernel 7.0.0-38-generic.
- Physical unexpected-removal recovery and full CI qualification remain pending.

## 0.5.47 — 2026-10-20

- Correct EFI directory/open/file-read error attribution: firmware DiskIo read failures and media removal now propagate as I/O/media statuses rather than being unconditionally reported as OFS volume corruption. A physical out-of-range metadata pointer or failed canonical OFS validation still reports EFI_VOLUME_CORRUPTED. No implicit retries or relaxed validation.
- Preserve the original firmware read-error status per synchronous OFS protocol operation, retaining read-only access and existing separate filesystem-driver ownership.
- Real Dell Latitude 5550 OFSBOOT (249,733,120 sectors) passed a read-only root and /boot canonical reader traversal under Linux, but firmware had returned EFI_VOLUME_CORRUPTED for both directory enumeration and kernel load. This adjustment improves failure attribution; it is **not** evidence of a successful Linux kernel boot from OFS, and a firmware-specific I/O root cause has not been established.
- Expand CI to verify explicit transport/metadata error distinctions and retain independent EFI driver build and UEFI protocol tests.

### Field diagnostic utility included

- Add a native read-only media probe that exercises the *existing canonical* OFS EFI directory reader against an authentic partition or image without reformatting, repairing or changing boot state. It records directory entries and hash-cursor context on failure.
- Cover the utility with supported and unsupported OFS firmware fixture cases. This supplements but does not replace the real Dell Latitude 5550 qualification of large OFS directories, firmware I/O and Linux kernel handoff.
- Document the real `EFI_VOLUME_CORRUPTED` directory-enumeration observation without assuming the filesystem on disk is damaged or releasing an unproven driver change.


## 0.5.46 — 2026-10-20

- Correct the GNU-EFI `efi_main` C calling convention for the OFS EFI boot-service driver. OVMF reproduced a stall during its first firmware Boot Services call; only exported EFI protocol callbacks retain the Microsoft x64 ABI.
- Correct UEFI filesystem-controller ownership: observe BlockIo without claiming it, and open DiskIo BY_DRIVER. OVMF reproduced EFI_ACCESS_DENIED on the original BlockIo claim. Release DiskIo in Start failure and Stop.
- Qualify a real standalone EFI driver under QEMU/OVMF on a disposable GPT partition: load image, bind, open and read a nested OFS file, disconnect and rebind, and reject non-OFS media. Run the same cases for the standard release-style binary and the opt-in serial diagnostic build.
- Keep OFS experimental and disabled by default. Virtual execution does not qualify physical Dell firmware, removable-media latency, Infiltrator Boot UI scanning, Secure Boot or a full kernel handoff.

## 0.5.45 — 2026-10-20

- Introduce opt-in COM1 firmware-stage tracing and an isolated QEMU/OVMF Driver Binding harness. Initial virtual tests exposed entrypoint and controller-claim defects later repaired in 0.5.46.
- Preserve 0.5.45's published release identity; do not treat its experimental OFS EFI payload as physically qualified.

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
