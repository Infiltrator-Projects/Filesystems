# AmigaDOS OFS firmware reader and UEFI driver

**Owner: Filesystems.** Boot never embeds or compiles OFS algorithms.

This directory contains two distinct layers:

- `reader.{c,h}`: portable, freestanding, read-only DOS\\0 OFS parser using the canonical Filesystems core. The shared `ofs_core` and `ofs_primitives` remain authoritative for on-disk rules.
- `uefi_driver.c`: standalone x86-64 UEFI Driver Binding and Simple File System adapter, with explicit read-only open, read, enumerate and metadata behaviour. It speaks standard EFI protocols and has no dependency on Infiltrator Boot code.

Build on Ubuntu with `gnu-efi`, GCC and binutils:

```bash
bash scripts/build-ofs-efi.sh dist-ofs-efi
```

The sole firmware payload is `dist-ofs-efi/OFS_x64.efi`, verified as a PE32+ x86-64 EFI boot-service driver (subsystem 11). The script never installs to the ESP or writes firmware variables. Infiltrator Boot loads external filesystem drivers from `drivers_x64/` **beside the currently running Boot executable** (for an `EFI/BOOT/BOOTX64.EFI` layout, that is `EFI/BOOT/drivers_x64/`). Use the actual existing installation path, not a new or duplicate Boot tree. The Filesystems Debian release stages this driver under `/usr/lib/filesystems/efi/experimental/`. It remains experimental and is never automatically installed into Boot's active driver directory.

**OVMF firmware regression:** `scripts/test-ofs-uefi-ovmf.sh` boots a disposable GPT disk under QEMU/OVMF. A UEFI application loads the actual standalone OFS driver, requests Driver Binding via `ConnectController`, opens `\\boot\\kernel.efi` through EFI Simple File System and checks the five-byte test file. A second test partition uses an FFS DOS type and must not bind as OFS. The dedicated CI lane builds the normal release-style driver independently of an opt-in serial-instrumented driver and archives its stage log. Run `OFS_EFI_TRACE=1 bash scripts/build-ofs-efi.sh dist-ofs-efi-diagnostic` only for virtual diagnostics; the normal driver does not contain the serial tracing code. Both the serial-instrumented diagnostic build and the standard uninstrumented EFI binary must pass the OVMF acceptance cases. The positive case also tests protocol teardown by DisconnectController and successful reconnection. The harness does not run Infiltrator Boot, transfer a real EFI-stub kernel, exercise a physical SD controller or establish a firmware I/O timeout on the Dell.

**Qualification:** experimental. CI compilation and native portable-reader tests do **not** prove OVMF firmware binding, full kernel-file loading, media read integrity on the physical Dell Latitude 5550, or Secure Boot compatibility. A live EFI read test is mandatory before a production rollout. The UEFI adapter rejects unsupported/malformed volumes and writes nothing.

**Firmware probe boundary:** only UEFI logical-partition BlockIo handles are eligible. Whole-disk handles and incompatible logical blocks are rejected before firmware disk reads. Each format probe reads at most two 512-byte sectors (boot block and root block); the start path repeats validation and may also read the volume label. A synchronous firmware `ReadDisk()` cannot be interrupted by this driver, so the I/O count is bounded but firmware latency is not. Directory lookup and enumeration stop after 512 entries, rejecting cyclic or overlong chains instead of walking a volume-sized chain. This is a boot-reader limit, not a filesystem format limit.

**Initial scope:** partition-relative DOS\\0, 512-byte logical sectors, names up to 30 Latin-1 bytes, case-insensitive OFS directory lookup, linked 488-byte OFS data blocks, files at most 4 GiB. No Amiga RDB whole-disk partition extraction, links, writable EFI operations or general Linux root qualification. EFI-stage readers and Linux `ofs.ko` have different ABIs; Linux requires its own kernel module (often inside the initramfs) to mount OFS after EFI handoff.

## Physical-media, read-only firmware-reader reproduction

Boot 0.1.25 on a Dell Latitude 5550 discovered OFSBOOT and Linux images on a
119 GiB OFS SD card, but UEFI enumeration subsequently returned
`EFI_VOLUME_CORRUPTED` while scanning `\\boot`. That status is a firmware
adapter's general conversion of the canonical reader's error and **does not
prove actual media corruption**. Existing CI exercises a single nested small
file, not a populated full-size physical volume.

Use this diagnostic on Linux with the OFS partition mounted or unmounted. It
opens the partition **O_RDONLY** and traverses the *same canonical OFS EFI
reader* used by the firmware adapter; it does not write, format, repair, or
change any EFI or boot configuration. It reports the directory, hash bucket,
entry count and sector reads. A passing result does not prove UEFI DiskIo
reliability, and a failing result distinguishes a host-reproducible reader
rejection from a firmware-only failure, without claiming why an individual
sector was rejected.

From the Filesystems repository root:

```bash
cc -std=c11 -O2 -Wall -Wextra -Werror \
  tests/ofs_efi_media_probe.c \
  native/filesystems/ofs/efi/reader.c \
  native/filesystems/ofs/core/ofs_core.c \
  native/filesystems/ofs/core/ofs_primitives.c \
  -o /tmp/ofs-efi-media-probe
sudo /tmp/ofs-efi-media-probe /dev/mmcblk0p1
```

The probe performs metadata enumeration only; it is not a full filesystem
checker and cannot establish the integrity of every kernel or initramfs block.

## 0.5.47: EFI I/O versus metadata failures

The Dell Latitude 5550 reported `Volume Corrupt` both while scanning
`/boot` and while loading `vmlinuz` through Boot 0.1.25 with the 0.5.46
OFS EFI driver. The real 249,733,120-sector OFSBOOT partition passed
read-only Linux-hosted enumeration using this repository's own canonical
EFI reader (three entries at root, four under `/boot`). That evidence
isolates no firmware defect by itself.

Previously, the adapter translated **all** canonical reader failures into
`EFI_VOLUME_CORRUPTED`, even when `EFI_DISK_IO.ReadDisk` returned a transport
error or a removable device disappeared. The adapter now preserves an EFI
transport error within each synchronous filesystem operation and only
reports `EFI_VOLUME_CORRUPTED` when the on-disk format checks themselves
reject the metadata. If DiskIo labels a low-level read failure
`EFI_VOLUME_CORRUPTED`, the adapter exposes it as `EFI_DEVICE_ERROR` to
avoid falsely asserting a corrupt filesystem. Neither Boot nor OFS writes
to the SD card, performs repair or retries a failing read.

This is **diagnostic correctness**, not a qualified OFS boot fix. CI
builds and exercises the EFI driver but cannot prove the Dell's physical
firmware DiskIo implementation or kernel handoff succeeds. The 0.5.46
executable already installed on a physical machine is not automatically
replaced by changes to this repository.


## Physical Dell BlockIo transport change — 0.5.49

On 21 October 2026, Dell Latitude 5550 firmware bound OFSBOOT and
successfully read eight consecutive 64 KiB chunks of a 17,029,512-byte
kernel. The next EFI file-protocol Read (starting at offset 524,288) returned
EFI_DEVICE_ERROR. This is a **failing transfer start offset**, not proof
that the exact disk sector is corrupt. The same source passed a complete
Linux-hosted canonical-reader comparison.

The driver formerly used DiskIo.ReadDisk for each aligned 512-byte sector.
It now uses partition-relative BlockIo.ReadBlocks with a single reusable,
Media.IoAlign-compliant sector buffer, avoiding the arbitrary-byte DiskIo
transport layer. Supported() releases its probe buffer, and Start failure,
successful Stop and protocol-install errors release the controller buffer.
DiskIo remains claimed BY_DRIVER: the partition driver may already own
BlockIo, and claiming it is known to fail under OVMF. Filesystem parsing,
error attribution and read-only semantics are unchanged.

On an actual file-read failure from BlockIo, the driver prints the raw
firmware status and partition-relative failing sector. The transport change
and diagnostic require physical Dell qualification; neither virtual tests
nor host data comparison proves complete Linux boot.


## Dell SD firmware transient read recovery — 0.5.50

With Boot 0.1.28 and OFS EFI 0.5.49 on the Dell Latitude 5550,
BlockIo returned `EFI_DEVICE_ERROR` at partition-relative sector
125092067 during the initial loader operation and at 125093772 during
the later fallback read, which reached file byte offset 1310720.
These are distinct firmware transport failures, not established OFS
data corruption. The canonical OFS reader previously compared the
complete files successfully using Linux reads; firmware-specific I/O
remains a separate qualification boundary.

Version 0.5.50 bounds recovery to **one** additional read-only
512-byte `BlockIo.ReadBlocks` attempt after a 1 ms delay, only on
`EFI_DEVICE_ERROR` while the media is present. Other failures stop
immediately; persistent errors preserve the exact failing sector
and final raw firmware status. Canonical OFS checks are unchanged,
the controller is never reset, and no disk writes are performed.
This is a targeted mitigation, not proof of Dell firmware reliability
or successful kernel/initramfs loading.
