# AmigaDOS OFS firmware reader and UEFI driver

**Owner: Filesystems.** Boot never embeds or compiles OFS algorithms.

This directory contains two distinct layers:

- `reader.{c,h}`: portable, freestanding, read-only DOS\\0 OFS parser using the canonical Filesystems core. The shared `ofs_core` and `ofs_primitives` remain authoritative for on-disk rules.
- `uefi_driver.c`: standalone x86-64 UEFI Driver Binding and Simple File System adapter, with explicit read-only open, read, enumerate and metadata behaviour. It speaks standard EFI protocols and has no dependency on Infiltrator Boot code.

Build on Ubuntu with `gnu-efi`, GCC and binutils:

```bash
bash scripts/build-ofs-efi.sh dist-ofs-efi
```

The sole firmware payload is `dist-ofs-efi/OFS_x64.efi`, verified as a PE32+ x86-64 EFI boot-service driver (subsystem 11). The script never installs to the ESP or writes firmware variables. Infiltrator Boot loads external filesystem drivers from `drivers_x64/` **beside the currently running Boot executable** (for an `EFI/BOOT/BOOTX64.EFI` layout, that is `EFI/BOOT/drivers_x64/`). Use the actual existing installation path, not a new or duplicate Boot tree. This driver remains experimental and is not included in Boot's auto-enabled release manifest.

**Qualification:** experimental. CI compilation and native portable-reader tests do **not** prove OVMF firmware binding, full kernel-file loading, media read integrity on the physical Dell Latitude 5550, or Secure Boot compatibility. A live EFI read test is mandatory before a production rollout. The UEFI adapter rejects unsupported/malformed volumes and writes nothing.

**Initial scope:** partition-relative DOS\\0, 512-byte logical sectors, names up to 30 Latin-1 bytes, case-insensitive OFS directory lookup, linked 488-byte OFS data blocks, files at most 4 GiB. No Amiga RDB whole-disk partition extraction, links, writable EFI operations or general Linux root qualification. EFI-stage readers and Linux `ofs.ko` have different ABIs; Linux requires its own kernel module (often inside the initramfs) to mount OFS after EFI handoff.

## Physical Dell OFS boot-source experiment

1. Build the driver from **Filesystems**, using the command above, or retrieve the `ofs-uefi-experimental` artifact from the latest successful **OFS UEFI standalone driver** action on `main`. This artifact is not the ordinary Filesystems Debian package.
2. Confirm the actual current Boot EFI directory. Copy `OFS_x64.efi` into that directory's existing `drivers_x64/`. **Do not** overwrite `BOOTX64.EFI`, modify NVRAM or install a second Boot copy. Retain existing working Boot and GRUB fallback paths.
3. Independently identify an expendable OFS-formatted **partition** (not a disk guess), and mount it via the project-native Linux `ofs` module. The 512-byte DOS\\0 volume must appear as a UEFI BlockIo **partition handle**; raw Amiga RDB disks are outside the current driver scope.
4. Verify `/boot/vmlinuz-$(uname -r)`, `/boot/initrd.img-$(uname -r)` and the **matching** `/lib/modules/$(uname -r)/updates/infiltrator/ofs.ko` exist on the Linux host. The Linux root stays on the existing filesystem and is identified by its own UUID, not the OFS partition.
5. From the Filesystems source tree, first execute **read-only preflight**:

   ```bash
   findmnt -n -o SOURCE,FSTYPE,TARGET --target /mnt/verified-ofs
   findmnt -n -o SOURCE,UUID,TARGET --target /
   bash scripts/stage-ofs-boot-payload.sh --mount /mnt/verified-ofs --root-uuid YOUR_EXISTING_LINUX_ROOT_UUID
   ```

6. Only after checking that the mount is the correct OFS partition, repeat that exact command with `--confirm`. It will create `/boot/` on the OFS mount if needed and copy `vmlinuz`, `initrd.img`, `ofs.ko`, and `refind_linux.conf`. It verifies each copy and **refuses to overwrite different existing files**. It cannot format media or touch the ESP.
7. Reboot into the **existing** Boot installation, verify a Linux candidate appears on the OFS volume, and test the handoff. Stop at a failure and record the exact firmware/kernel error; compilation is not proof of EFI BlockIo binding.

For this first **boot-source** demonstration, the running Linux root can stay on EXT4. The `ofs.ko` file copied onto the card is an evidence/payload copy; Linux can only load it during early startup if that module and its dependencies are actually included in the initramfs. Merely putting `ofs.ko` beside the kernel does **not** cause the kernel to load it or mount OFS as root. Qualifying OFS as an operating-system root is a separate later milestone.

Kernel, initramfs and configuration filenames are deliberately short to respect AmigaDOS's 30-byte filename limit. Do not claim Secure Boot compatibility unless the new external EFI image passes the machine's active trust policy.
