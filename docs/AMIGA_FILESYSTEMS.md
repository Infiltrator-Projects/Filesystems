# Amiga filesystem architecture

Filesystems maintains five independent canonical Amiga filesystem
implementations:

- OFS
- FFS
- SFS
- SFS2
- PFS3

Historical relationships do not make their on-disk semantics interchangeable.

## Permanent layout

Each filesystem owns its own `core/` and host adapters.  The Linux result is
one independently deployable module per filesystem: `ofs.ko`, `ffs.ko`,
`sfs.ko`, `sfs2.ko` and `pfs3.ko`.

OFS and FFS have independent project-owned cores and Linux VFS adapters.
SFS and SFS2 have independent project-owned cores and Linux VFS adapters with
their respective format widths and identities.  PFS3 has a project-owned
canonical core plus a native Linux adapter; the current PFS3 Linux mutation
surface remains intentionally narrower than its read surface and must not be
described as full read/write parity until runtime qualification proves it.

## Incremental rewrite rule

A working/reference implementation is evidence for a rewrite, not shipping
source.  It must remain available until every replacement responsibility has
equivalent behavioural evidence.  Rewriting a source unit never authorises
removing a capability merely because the replacement compiles.

The repository therefore retains non-shipping reference baselines for the
active Amiga rewrite work:

- the historical ASFS Linux SFS source used to establish Linux VFS behaviour;
- the SFS2 format/tool references used to establish the SFS2-specific layout;
- the PFS3aio source used to establish PFS3 on-disk and recovery semantics.

Reference source is never linked into the project modules. CI enforces that
active source contains project-owned implementations and that the module
Makefiles/CMake graph do not consume `reference/`.

OFS/FFS are continuously checked against the independently maintained Linux
AFFS behaviour and AmigaDOS disk-format invariants; the project-owned OFS and
FFS implementations remain separate rather than sharing one semantic engine.

## First format-and-mount milestone: OFS

`filesystems-ofs-image` is a deliberately limited, non-destructive
developer-preview creation tool. It exclusively creates previously
nonexistent **regular image files** (not block devices), builds a genuine
AmigaDOS DOS\0 OFS root and bitmap layout from the project's canonical OFS
geometry, and verifies the resulting metadata before returning success.

Its regression suite checks geometry spanning root bitmap extensions,
duplicate creation, symlink rejection, invalid labels/sizes, damaged boot
and metadata blocks, bitmap corruption and refusal of block devices.
The release Amiga runtime workflow formats the OFS media with this
tool, compares it byte-for-byte against an independently maintained,
pinned Defragmenter OFS test formatter, then boots a QEMU Linux kernel
with host AFFS disabled and the independent project `ofs.ko` to exercise
file operations, remount and committed-data recovery.

This is **not yet** a general-purpose `mkfs` utility or qualification
to format physical storage or serve as a full operating-system root.
The image-only entry point is intentional until larger-size, failure
injection, on-media interoperability and power-loss recovery evidence
justifies a destructive, partition-aware formatter. It also does not
supply a UEFI OFS reader: Boot's AmigaDOS kernel-loading experiment
is separately pending.

## Qualification rule

Compilation is necessary but is not filesystem qualification.

For every Amiga filesystem, qualification requires as applicable:

1. build the module against supported kernel APIs;
2. load the project module with the host's competing driver disabled;
3. mount independently-created genuine media;
4. traverse directories and verify known payload bytes;
5. exercise create/write/grow/truncate/link/symlink/rename/unlink/rmdir where
   the implementation claims those operations;
6. unmount and remount;
7. verify the resulting image with an independent implementation or checker;
8. exercise malformed-media rejection;
9. for transactional filesystems, exercise interrupted-update recovery before
   writable support is called qualified.

A green compile-only CI job must never be used as evidence that a filesystem
works on media.

## Independence rule

OFS, FFS, SFS, SFS2 and PFS3 remain implementation-independent.  Filesystem
semantics are not shared between their directories merely because arithmetic
or structures look similar. Only genuinely filesystem-neutral infrastructure
may live in `native/core/` or Libraries.
