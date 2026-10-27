# FAT12 and FAT16: genuine full-implementation qualification

Target deliverables are **two independently loadable filesystem identities**, each with a read/write-capable `.so`, `.efi`, and `.ko`. A successful compilation is not a release approval.

## Full original source incorporated for UEFI

The complete TianoCore EDK2 `FatPkg/EnhancedFatDxe` implementation is copied into `native/filesystems/fat/efi/FatPkg/EnhancedFatDxe`, with package build metadata in the adjacent FatPkg directory. The exact source is from [tianocore/edk2 commit 447c415c2678a2bab107d5d529b879e3d7713830](https://github.com/tianocore/edk2/commit/447c415c2678a2bab107d5d529b879e3d7713830). Copyright and SPDX headers remain in the actual code. All driver C, H, UNI, and INF implementation files are local—**not references or remote source used in place of source ownership**.

The only modifications are format-specific build descriptors `Fat12.inf` and `Fat16.inf`, distinct module GUIDs, an explicit type guard in `Init.c`, and a package DSC that builds those two targets. The upstream runtime implements file and directory read, write, create, deletion, directory changes, allocation and flush; unlike the existing EFI FSW read-only adapter it is designed to provide writable `EFI_SIMPLE_FILE_SYSTEM_PROTOCOL`. The shared original implementation is compiled separately inside each image. No FAT12/FAT16 impersonation or generic FAT driver masquerading is permitted.

`scripts/build-fat12-fat16-efi.sh` fetches pinned EDK2 **toolchain/framework** sources for MdePkg and BaseTools but REPLACES FatPkg with the local Filesystems-owned driver sources. The output is `build-fat-efi/FAT12.efi` and `FAT16.efi`. No disk writes or ESP mutations occur.

## Source ownership / validation gates

- The native Linux FAT source under `native/filesystems/fat/linux` is copied Linux v6.12.107 FAT/VFAT/MSDOS implementation, containing genuine allocation, nested directories, long names, create, rename, delete and read/write. **It has not yet been built as independently verified FAT12.ko / FAT16.ko**. The normal FAT kernel implementation is shared among all three formats; enforcing independently loadable format-specific kernel registration requires native adapter work.
- The existing `FAT12.so` and `FAT16.so` remain limited classic FAT modules. Bounded overwrite capability in the separate V2 work is **not** full read/write filesystem functionality. Replacing the limited reader with the genuine portable engine, not reimplementing it a third time, is required.
- The two new EFI images must be tested in real UEFI firmware/QEMU with independently generated FAT12/FAT16 images: reject each other's variant, root and nested directory create/read/rename/delete, long names, noncontiguous file expansion, volume-full and media-error cases, flush/remount, external verification and recovery after interrupted writes.
- For each `.ko`, qualify genuine Linux 6.12 kernel load/mount, full file/directory create/delete/rename, persistent read/write/remount, independently created images and corruption handling. A shared `fat.ko`+ `vfat.ko` stack is not silently renamed to two format-specific `.ko` files.
- For each `.so`, validate equivalent operations against authoritative tools, with bounded partition I/O, no uncontrolled host writes, LFN and geometry variants, flushing and failure injection.

**Status:** original full EFI sources integrated, EFI compilation/PE qualification underway. Full release target NOT MET until all six artifacts pass their actual runtime matrices. Do not deploy to a boot disk as if this qualification had already passed.
