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
