# OFS firmware reader (experimental)

This is a freestanding, callback-based **read-only** parser for authentic 512-byte AmigaDOS DOS\\0 OFS **partition-relative** volumes. It consumes the canonical OFS primitives and format classifier; it does not invoke Linux VFS, allocate memory, mutate media, or depend on an EFI toolkit.

Implemented: root recognition at the canonical midpoint; bounded hash-table directory lookup/enumeration; strict checksums, parent and block validation; Latin-1 byte-name case folding; OFS linked data-block reading with an optional sequential cursor; explicit rejection of unsupported non-OFS variants, links and damaged blocks.

Not yet implemented: the EFI Driver Binding / Simple File System protocol adapter, firmware-sized volume probing, RDB partition discovery, UTF-16 path translation, signed PE/COFF driver build, actual kernel reads or OVMF/physical boot proof. This component is **not** a finished UEFI driver or a qualified Boot release artifact. Boot must not enable an OFS driver in its manifest until those stages pass.

The reader deliberately starts with the narrow `DOS\\0`, 512-byte GPT/MBR-partition test surface created by Filesystems' guarded formatter. Filesystem identity, root-volume suitability and root mounting are separate from reading a kernel payload before the OS starts.
