# Changelog

## 0.5.70 — 2026-10-23

- Promote the original independent NTFS3 implementation as the managed
  `ntfs3.ko` module for the single NTFS format identity, backed by the
  passed Linux 6.12 QEMU mount/write/remount and independent ntfscat
  verification (Actions run 39931921927).
- Switch the catalogue from NTFS-3G userspace runtime to the project-managed
  native VFS module; preserve original source and upstream authorship.
- Use the existing native kernel-version matching, source fingerprinting,
  signing, module selection and safe uninstall/rollback machinery.
- Keep EFI/root qualification and untested NTFS semantics explicitly open.


## 0.5.69 — 2026-10-23

- Restore 55 original CP/M implementation and build files from pinned
  lipro-cpm4l/cpmtools e534e20c15973a9559e981efb498a102020e5db7
  under editable per-format `userspace/`, retaining original code and COPYING.
- Extend the original-userspace source manifest and CI implementation
  presence guard to cover CP/M, keeping native installation disabled
  until a real kernel adapter is implemented and independently qualified.
- Source inventory now covers 37/41 formats: 10 managed native drivers,
  25 other Linux driver sources and two userspace-origin implementations.


## 0.5.68 — 2026-10-23

- Import 35 original Fosfat/Smaky implementation and build files from
  Skywalker13/Fosfat commit 289c7eb838dd5e6305c98f09aa375746a6043d86
  under the directly editable per-format userspace source directory.
- Preserve upstream licence/attribution, add CI that proves actual source
  exists, and keep userspace source distinct from qualified .ko modules.
- Inventory: 10 managed native drivers, 25 other original Linux baselines,
  one userspace original baseline, and five source imports outstanding.


## 0.5.67 — 2026-10-23

- Constrain OFS UEFI `BlockIo.ReadBlocks` transfer pages to the low 4 GiB DMA-addressable physical range using `AllocateMaxAddress`. The Dell's Realtek RTS525A is operated with a 32-bit DMA mask by Linux `rtsx_pci`, whereas the prior `AllocateAnyPages` offered no equivalent physical-address bound.
- Retain the page-backed power-of-two alignment guarantees, check the full 512-byte bounce buffer stays in range and fail closed rather than falling back above 4 GiB. Preserve the read-only OFS parser, single-sector reads, bounded retry limit and all Linux drivers.
- Add regression assertions and require real GNU-EFI/OVMF qualification. Dell Latitude 5550 OFS kernel loading remains unverified until the deployed artifact is physically retested.

## 0.5.66 — 2026-10-23

- Add standalone Linux 6.12 UDF filesystem-module runtime qualification: build original copied `udf.ko` against a kernel without built-in UDF, mount media produced by the independent `udftools` formatter, write and rename files, unmount/remount, and verify persistent contents using the separate host kernel's read-only UDF mount.
- Keep UDF in the original-source-only manifest until native runtime qualification passes; this adds no premature installable driver or filesystem capabilities.


## 0.5.65 — 2026-10-23

- Add an independent Linux 6.12 QEMU native NTFS3 qualification workflow using the original source recovered under the canonical NTFS identity, not the NTFS-3G mount provider.
- Format test media independently with `mkfs.ntfs`, load only our actual `ntfs3.ko` inside a Linux guest, exercise directory/file creation, multi-cluster write, rename, unmount/remount persistence and independent `ntfscat` verification.
- Keep NTFS out of the managed module deployment manifest until real guest runtime and data-integrity checks pass; preserve original implementation and attribution.


## 0.5.64 — 2026-10-23

- Add isolated Linux 6.12 QEMU runtime qualification for the original copied exFAT driver, using `exfatprogs` to format independent disposable media, a kernel without exFAT built into vmlinux, the actual external `exfat.ko`, directory creation, file writes, rename, unmount/remount readback and offline fsck.
- Keep exFAT outside the managed native installation manifest until this runtime workflow completes successfully. Preserve copied source and its original attribution unchanged.
- Do not conflate the earlier Debian exported-symbol build pass with genuine runtime qualification.


## 0.5.63 — 2026-10-23

- Promote original independent SCO BFS to managed `bfs.ko`, following successful Debian symbol-resolved module compilation and Linux 6.12 QEMU mount/write/unmount/remount qualification.
- Register BFS in the one native deployment manifest, enabling the existing kernel-version build, signing, module-selection, source freshness and transaction rollback mechanisms. Build `bfs.o` directly as its own out-of-tree module without rewriting the original filesystem logic.
- Retain BFS's source provenance and separate unqualified EFI/root capability. Add a native catalogue regression asserting the managed BFS identity.


## 0.5.62 — 2026-10-23

- Trial a UEFI-specific SD-controller compatibility correction: allocate the
  OFS EFI driver's 512-byte firmware transfer buffer from contiguous UEFI
  physical pages aligned to at least 4096 bytes while respecting any stricter
  Media.IoAlign up to 64 KiB.
- Keep the canonical OFS core, linked-block validation, 512-byte ReadBlocks
  request size, bounded retry policy and Linux ofs.ko unchanged.
- Extend the EFI source contract and retain full OVMF binding/read regression.
  The Dell Latitude 5550's intermittent high-LBA firmware errors still require
  real-device validation; passing CI is not physical boot qualification.

## 0.5.61 — 2026-10-23

- Promote the original independent Minix filesystem driver to the managed
  `minix.ko` install/remove lifecycle after its Linux 6.12 QEMU mount,
  write, unmount and remount runtime test and Debian symbol-resolved build.
- Allow complete native Linux drivers to retain original format semantics
  in their actual `linux/` source directory without manufacturing empty
  `core/` placeholders; retain per-format source hashing, kernel ABI
  matching, signing, module-selection checks and rollback.
- Preserve explicit separation of source availability, native Linux driver
  qualification and future EFI/root qualification.


## 0.5.60 — 2026-10-23

- Make the original Bcachefs tracepoint re-inclusion resolve through the
  actual out-of-tree Kbuild source directory instead of assuming the source
  resides at `fs/bcachefs` in the Linux kernel tree.
- Preserve the original module's code, semantics and independent identity;
  validate the change through Debian exported-symbol linker probes before
  considering native deployment.
- No new module is added to the installable native manifest without runtime
  qualification.


## 0.5.59 — 2026-10-23

- Repair two concrete original-driver out-of-tree integration failures:
  match upstream Linux 6.12.113 by removing F2FS's obsolete
  `fsverity_prepare_setattr()` call, and restore the JFS Unicode
  declaration header from the pinned upstream Linux source into the
  original JFS adapter with an explicitly local include.
- Run Debian Module.symvers-backed linking checks on all 27 imported
  Linux formats, with separate diagnostics for still-unqualified modules.
- Exercise the imported Minix module's actual VFS behaviour in a QEMU
  boot with Minix absent from the built-in kernel.
- These build and runtime qualification experiments do not silently
  add imported modules to the installable native manifest.


## 0.5.58 — 2026-10-23

- Restore the exact 38-file original Linux APFS out-of-tree module source
  (including Kbuild, DKMS, licences and upstream authorship) under the
  single canonical APFS format identity; independently compare each Git
  blob hash against its original upstream source.
- Expand the actual editable-source baseline from 34 to 35 of the 41
  catalogue formats. Six complete source imports still remain.
- Introduce Linux 6.12 compilation probes for eight original legacy
  filesystem drivers and strict exported-symbol checking against
  Debian 13's packaged kernel headers. Compilation is not claimed
  as mounted runtime or safe write-path qualification.
- Preserve explicit native install gates; no imported module is silently
  promoted to deployable kernel support.


## 0.5.57 — 2026-10-23

- Complete the editable-original-source layout: preserve the copied legacy SFS, SFS2 and PFS3 source snapshots under per-filesystem `original/` trees, retaining their exact Git objects, while keeping the qualified independent `linux/` drivers unchanged.
- Enforce the absence of all obsolete `reference/` filesystem source directories rather than only `reference/linux/`.
- Maintain the separate native driver deployment manifest and the version/tag immutability policy. No new filesystem capability is claimed.


## 0.5.56 — 2026-10-23

- Restore 1,239 original copied Linux source files for 25 in-scope
  filesystem formats from the reference quarantine to editable per-filesystem
  `linux/` working directories without altering their implementation blobs.
- Recover the original NTFS3 Linux implementation from Filesystems Git history
  as the single NTFS format's editable source baseline. Existing source
  attribution and licensing remain intact.
- Remove automatic upstream source replacement/pushing and update CI to require
  actual editable imported code, separated from the eight independently
  qualified and deployable native modules.
- Restore documentation of the original-source development model and report the
  remaining seven unimported complete source baselines honestly.
- This source-recovery release does not claim newly qualified native drivers,
  physical-media capability, Windows drivers or firmware boot behaviour.


## 0.5.55 — 2026-10-23

- Refuse package removals during support installation, native-module build-prerequisite installation and standalone package installation. APT enforces `--no-remove` against its actual transaction, so a conflict cannot silently turn an installation into removal of unrelated applications or bypass the wrapper's earlier simulation.
- Extend the private real-APT transaction regression to capture all three production installation commands. Verify that ordinary installations reach unpack while conflicting plans stop before any dpkg mutation; retain the existing exact-removal approval and package-protection cases.
- Update the maintained installation-authority and validation contracts. This correction does not complete the recorded EXT3/EXT4 implementation migration or qualify physical OFS boot, writable PFS3 or Windows runtime support.

## 0.5.54 — 2026-10-22

- Return OFS/FFS resize failures through the VFS setattr boundary instead of reporting success after restoring the previous size. Stop subsequent attribute updates on failure and preserve last-release resize status.
- Separate SFS/SFS2 allocated filesystem-block counts from Linux's 512-byte-sector accounting across growth, truncation, close and inode reload.
- Bind package-removal approval to installed package names and architectures. Enforce the exact final removal set and protection policy with an installed APT transaction guard under the frontend lock, closing the authentication/concurrent-package-change race without a separate package.
- Add resize fault injection, real-APT tests with an inert dpkg executor and mounted SFS/SFS2 accounting regressions. Update the maintained validation and privileged-operation contracts.
- EXT3/EXT4 inherited migration units remain recorded in the provenance ledger; this defect repair does not claim their implementations have been rewritten.

## 0.5.53 — 2026-10-22

- Remove speculative EFI BlockIo read-ahead: physical Dell 0.5.52 testing demonstrated DEVICE_ERROR at 8, 4, and 2 sectors, with 30-second stalls before falling back to one-sector reads. The failed optimisation also made the measured 55% kernel transfer 386 seconds slower than 0.5.51.
- Restore a single required 512-byte OFS block per firmware call, preserving canonical linked-block traversal, checksum validation, aligned DMA storage, exact failed-sector reporting and the existing one-retry bound.
- Diagnose slow actual-sector transfers: capture EFI RTC times around each BlockIo call, report calls taking at least two seconds with sector, attempt, elapsed seconds and raw status, capped at twelve reports per controller. This observes synchronous firmware latency without pretending to interrupt it.
- No changes to disk format, persistent data, boot configuration or external EFI filesystem interfaces. Physical throughput and full Linux EFI handoff remain unverified.


## 0.5.52 — 2026-10-22

- Avoid permanently disabling eight-sector EFI read-ahead after a single speculative EFI_DEVICE_ERROR. Step down the per-controller batch 8 to 4 to 2 to 1 sectors for successive transient bulk failures, retaining a useful smaller batch when supported.
- Disable batching directly on size/protocol rejections; always retry the failed speculative operation as the exact requested 512-byte sector using the unchanged bounded error path. Emit a diagnostic whenever the bulk window changes.
- Preserve read-only operation, full block validation, MediaId cache checks and no separate filesystem logic in Boot. Record that the Dell successfully read its complete 17 MB kernel in 1,091 seconds but Linux EFI-stub handoff and the 86 MB initramfs read remain unverified.


## 0.5.51 — 2026-10-22

- Opportunistically fetch up to eight adjacent 512-byte EFI BlockIo sectors during OFS file reads, caching by sector and MediaId while preserving canonical OFS checksums and linked-block traversal.
- On firmware device/size rejection of speculative I/O, disable read-ahead for the controller and use the existing precise single-sector read and bounded retry. Preserve hard failure on media changes, access errors and real data-validation faults.
- Extend EFI source-contract checks. Dell Latitude 5550 physical kernel-read throughput and Linux handoff remain unverified.


## 0.5.50 — 2026-10-22

- Record physical Dell Latitude 5550 EFI `BlockIo` `EFI_DEVICE_ERROR` failures on distinct partition-relative sectors 125092067 and 125093772; Boot 0.1.28 reached 1,310,720 bytes before its fallback transfer failed. The status is a transport error, not proof of invalid OFS blocks.
- Implement one bounded, delayed, read-only retry only for `EFI_DEVICE_ERROR` while the device remains present; maintain fail-closed behaviour for persistent errors, media loss, unsupported operations, metadata and checksums, and preserve raw sector diagnostics.
- Extend the source regression contract. Firmware qualification and end-to-end OFS boot remain pending.


## 0.5.49 — 2026-10-21

- Respond to physical Dell Latitude 5550 OFS evidence: Boot 0.1.28 read the first 524,288 bytes of vmlinuz but the following 64 KiB EFI file transfer failed with EFI_DEVICE_ERROR.
- Use aligned partition-relative BlockIo.ReadBlocks instead of unnecessary DiskIo.ReadDisk for canonical 512-byte OFS sectors, with bounded reusable alignment buffer and explicit probe/start/stop cleanup. Preserve DiskIo BY_DRIVER driver-binding ownership from prior OVMF validation.
- Print the failing firmware sector and raw BlockIo status on file-read transport errors; retain strict on-disk validation and format-versus-I/O error attribution.
- Update and run the EFI error contract and OVMF driver regression workflow. Full physical Dell reads and Linux boot remain unverified pending retest.


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
