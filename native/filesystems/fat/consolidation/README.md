# FAT16 consolidation migration sources

These files are actual editable copies of our existing working source from
Infiltrator-Projects/Defragmenter, not external runtime dependencies or source
links. Imported on 26 October 2026 as the beginning of the move to one
authoritative FAT family implementation.

* fat_volume.c/.h: FAT12/16/32 geometry, FAT table reads and mutations.
* fat_directory.c/.h: recursive directory catalogue, VFAT LFN decoding.
* fat_analysis.c/.h: fragmentation and allocation-map analysis.
* fat_io.c/.h: buffered relocation I/O interface.
* fat_journal.c/.h: durable transaction journal support.
* fat_relayout.c/.h: placement/relayout planning.
* fat_relocation.c/.h: relocation mechanics and recovery.
* writer.c: existing integrated FAT12/16/32 Defragmenter worker.
* test_media_raw_fat.c: bounded FAT12/16/32 fixture format and verifier.

These original files deliberately remain **uncompiled** by FAT16.so for now:
the Defragmenter POSIX/COMMON dependencies and standalone privileged I/O
must be separated from the portable, freestanding host-I/O ABI. The first
port of directory traversal and the LFN decoder is compiled in
../module/fat16_namespace.c and has independently generated malformed-media
tests. Port remaining source functions incrementally without losing the
Defragmenter and Test Media regression coverage. Do not simply link a POSIX
worker into a firmware/root-filesystem module.

This is a temporary migration staging directory, NOT the intended
steady-state parallel implementation. The original Defragmenter code remains
active and must not be deleted until consumers switch to the qualified module.
FAT16 write, format, mounted VFS, EFI, defragmentation and recovery remain
unqualified.
