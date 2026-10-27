# FAT12 / FAT16 / FAT32 design and ownership

FAT12, FAT16 and FAT32 are related on-disk formats with different allocation
entry widths. Filesystems presents them as distinct selectable modules. A
module for one width must not be accepted as another by renaming the file.

## Source in this directory

- `portable/` contains the editable FatFs FAT implementation and the V3/V4
  standalone adapters. `ff.c` identifies itself as R0.16 with patch 2;
  `ffunicode.c` identifies an R0.13c conversion implementation adapted to the
  R0.16 interface. Existing notices remain in the copied source.
- `module/` contains the legacy V1 FAT12/16 adapters and independent FAT32
  reader. Each FAT12/16 shared object embeds the common portable sources and
  exports its own format identity; neither needs the other to be installed.
- `linux/` contains the actual copied Linux 6.12.107 FAT/VFAT/MSDOS code and
  the experimental FAT12/FAT16 namespace builds. The two modules depend on a
  compatible Linux FAT core. They are not self-contained replacement kernels.
- `efi/FatPkg/` contains the editable TianoCore EnhancedFatDxe source and
  distinct FAT12/FAT16 build descriptions. The EFI images are independent
  Driver Binding and Simple File System drivers; the local build uses a pinned
  EDK2 framework. Exact provenance and qualification are recorded in
  `docs/FAT12_FAT16_FULL_RWW_AUDIT.md`.

## FAT12 and FAT16 shared objects

The V1 API supplies classic FAT probing, root-file reading and fragment counts.
V3 supplies long names, nested directories, file/directory creation and
removal, reading, writing, growth, truncation and rename. V4 adds formatting,
attributes, modified-time read/write and capacity/free-space queries. Its
formatter tests the resulting FAT width without writing before it formats the
caller-supplied partition. A failed format may still leave partially formatted
media if an actual write or flush fails.

Each module permits one active volume. Callers provide partition-relative
reads, writes, flushing and timestamps, must serialize access, and must hold
exclusive access to writable media. Truncated volumes and the ambiguous
4,085-cluster boundary are rejected. The selector helpers compile only their
chosen module into `/System/Filesystems/`; they do not install a kernel module,
EFI driver, another FAT variant or a separate package.

## FAT32 shared object

`FAT32.so` remains a separate experimental read-only parser with FAT32 chain
reading and root-file fragmentation analysis. Its helper installs only that
module. FAT12/16's V3/V4 operations are not a FAT32 implementation.

## Qualification and remaining work

Use independent formatters, checkers and readers as well as disposable-image
and virtual-machine tests. Compilation alone is not runtime qualification.
The test suites exercise the modules' actual symbols and EFI driver bindings;
the Linux test boots the actual two modules against a compatible FAT core.
Check the workflow results for the exact source commit and distinguish QEMU
results from physical-device or firmware results.

These interfaces do not yet provide Defragmenter's relocation and journal
recovery operations, nor a common consumer dispatcher. Defragmenter currently
uses one FAT worker for FAT12, FAT16 and FAT32; migration must preserve all
three formats and recovery semantics before that worker can be removed.
Corrupt-media, I/O failure and interruption behaviour require further testing;
there is no claim of transactional crash recovery.
