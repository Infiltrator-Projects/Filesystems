# AmigaDOS OFS firmware reader and UEFI driver

**Owner: Filesystems.** Boot never embeds or compiles OFS algorithms.

This directory contains two distinct layers:

- `reader.{c,h}`: portable, freestanding, read-only DOS\\0 OFS parser using the canonical Filesystems core. The shared `ofs_core` and `ofs_primitives` remain authoritative for on-disk rules.
- `uefi_driver.c`: standalone x86-64 UEFI Driver Binding and Simple File System adapter, with explicit read-only open, read, enumerate and metadata behaviour. It speaks standard EFI protocols and has no dependency on Infiltrator Boot code.

Build on Ubuntu with `gnu-efi`, GCC and binutils:

```bash
bash scripts/build-ofs-efi.sh dist-ofs-efi
```

The sole firmware payload is `dist-ofs-efi/OFS_x64.efi`, verified as a PE32+ x86-64 EFI boot-service driver (subsystem 11). The script never installs to the ESP or writes firmware variables. Infiltrator Boot can load this file from its existing `EFI/InfiltratorBoot/drivers_x64/` directory once runtime-qualified.

**Qualification:** experimental. CI compilation and native portable-reader tests do **not** prove OVMF firmware binding, full kernel-file loading, media read integrity on the physical Dell Latitude 5550, or Secure Boot compatibility. A live EFI read test is mandatory before a production rollout. The UEFI adapter rejects unsupported/malformed volumes and writes nothing.

**Initial scope:** partition-relative DOS\\0, 512-byte logical sectors, names up to 30 Latin-1 bytes, case-insensitive OFS directory lookup, linked 488-byte OFS data blocks, files at most 4 GiB. No Amiga RDB whole-disk partition extraction, links, writable EFI operations or general Linux root qualification. EFI-stage readers and Linux `ofs.ko` have different ABIs; Linux requires its own kernel module (often inside the initramfs) to mount OFS after EFI handoff.
