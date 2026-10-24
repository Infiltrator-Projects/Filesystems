# DOS\\0 OFS — feature-completeness acceptance contract

**Status (24 October 2026): NOT FEATURE COMPLETE.** This is the per-variant release gate for classic AmigaDOS Old File System DOS\\0, on-disk DOSType `0x444F5300`. Unlike a defect list, this checklist establishes objective evidence required to use the label **feature complete**. The [canonical OFS format](../native/filesystems/ofs/OFS_SPECIFICATION.md) defines the required structures and semantics. This document records which implementations have proved compliance; neither a green compile nor an empty image by itself changes any unchecked status.

## Definition of complete

A complete **DOS0** implementation must support safe, interoperable **read/write** filesystems on supported physical partitions; correct names/metadata, allocation, format/reformat, all documented logical block sizes, all valid unsigned partition addresses and ordinary file lengths, EFI read/boot payloads, and Linux mount/use as a system root, with independent provenance and both normal and adversarial testing. We must not relabel DOS0 compatibility limits as restrictions of the underlying specification.

The `--variant dos0` formatter default must remain backward compatible throughout this work. DOS2, DOS4 and DOS6 are distinct follow-up qualification targets and cannot substitute for unfinished DOS0.

| DOS0 requirement | Present evidence | Completion gate / current result |
| --- | --- | --- |
| Accurate root, boot, bitmap, metadata layouts and checksums | Shared format primitives, independent odd/even image layout checks and root verification | **PARTIAL:** validate every structure against independently generated original/Amiga media |
| Formatting blank file-backed DOS0 volumes | Image formatter supports seven 512..32768 byte sizes; 120-GiB sparse QEMU fixture | **PARTIAL:** round-trip independently on Amiga-compatible readers and corrupted images |
| Format a physical partition safely | Exclusive, explicit confirmation; 512-byte logical OFS blocks and 1-TiB implementation cap | **PARTIAL:** widen only after *same geometry* Linux/EFI read/write + real media qualification |
| DOS0 case-insensitive 30-byte names, collisions, directory entries and enumeration | Linux VFS namespace operations and EFI tests | **PARTIAL:** test full namespace edge cases, collision chains, links and cycle rejection independently |
| Create/read/write/truncate/append, metadata extensions, partial data blocks | Linux driver plus QEMU read/write/remount and static ELF | **PARTIAL:** full pointer-extension and randomized cross-block write/read/truncate regression with checked sizes/contents |
| T_DATA=8 header, owner, sequence, next and checksums on every block | Portable checksums, Linux and EFI source | **PARTIAL:** cross-read real multi-extension files with a separate OFS parser and adversarial damaged blocks |
| Bitmap extension chains, free space, near-full disk, errors | 120-GiB read-only mount, QEMU bitmap and committed crash recovery | **PARTIAL:** allocation stress and power-loss/error recovery with independent bitmap and loss accounting |
| 32-bit **unsigned** block keys up to format boundary | EFI 512-byte standalone reader supports unsigned addresses | **OPEN:** Linux adapter historically stores `s_partition_size` as signed `int` and rejects volumes over INT_MAX blocks; eliminate narrowing and prove high-key actual Linux block I/O |
| Seven logical block sizes 512..32768 | Image formatter handles seven; Linux supports at most PAGE_SIZE; EFI currently only 512 | **OPEN:** Linux multi-page metadata buffers and EFI full block-size discovery/I/O; validate all seven in QEMU/OVMF |
| EFI DOS0 reader with bounded safe traversal and boot payload | Standalone read-only UEFI driver, OVMF test, 512-byte media | **PARTIAL:** full multi-size, >4-GiB physical address, alignment and actual EFI Linux kernel/initramfs read |
| Linux executing ELF and root suitability | QEMU successfully executes static BusyBox ELF directly from mounted OFS | **PARTIAL:** boot Linux **with OFS as actual root filesystem**, validate symlinks, permission semantics and early-boot module/initramfs recovery |
| Physical SD card and Boot manager integration | Separate Filesystems EFI driver and Boot loader architecture | **OPEN:** conscious disposable/physical test with recovery boot entry preserved; no changes authorized by this checklist |
| Error semantics | Root, checksum, bitmap tests and bounded EFI corruption cases | **PARTIAL:** exhaustive arithmetic/buffer/cycle/IO-failure fuzz and consistent failures across Linux/EFI |
| Publish/release evidence | Filesystems version 0.5.78 in main; earlier PR CI passed | **PARTIAL:** current change green for all required checks, signed package provenance, main push CI and verified publication; only declare DOS0 complete after every row is closed |

## Priority order

1. **Remove signed-address bugs** throughout Linux DOS0 without adding artificial limits; preserve fail-closed behavior while runtime evidence is absent.
2. Extend Linux metadata I/O beyond PAGE_SIZE and EFI filesystem-block sizes to 32768.
3. Test independent DOS0-created images and nontrivial files in both directions, including full file extension tables, root geometry, sparse/high-block addresses, filesystem corruption and allocation failures.
4. Demonstrate boot-source OFS payload loading and **separately** a real Linux **OFS root**, recoverable from a failed boot; validate Linux required semantics rather than silently asserting POSIX compatibility.
5. Widen physical formatting and packaging only for the tested geometries, leaving unrelated physical disks, GPT, NVMe, EFI/NVRAM and Boot entries alone.
6. Close each table row with exact CI run/fixture/hash/physical test evidence. Then and only then label DOS0 **feature complete**.

The existing [implementation compliance matrix](OFS_SPECIFICATION_COMPLIANCE.md), [OFS conformance issue #27](https://github.com/Infiltrator-Projects/Filesystems/issues/27) and [boot filesystem contract](BOOTABLE_FILESYSTEM_CONTRACT.md) remain in force. A later code change cannot redefine any of these completion gates without a source-backed format reason and regression evidence.
