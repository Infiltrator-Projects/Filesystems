# Amiga OFS canonical implementation

**Normative disk format and behavioral contract:** [OFS_SPECIFICATION.md](OFS_SPECIFICATION.md).

**DOS0 feature-completion qualification:** [DOS0_COMPLETION.md](DOS0_COMPLETION.md). **Evidence and implementation status:** [DOS0 completion matrix](../../../docs/OFS_DOS0_COMPLETION_MATRIX.md). This is the evidence checklist for claiming original OFS functionality complete; it is not a completion claim. The source must be evaluated against that external-source-backed specification; implementation details do not redefine authentic DOS0/DOS2/DOS4/DOS6 on-disk rules.

OFS in this repository is the canonical Infiltrator Projects OFS implementation. The source tree is authoritative.

There must be one OFS engine only. No FUSE, vendored, reference, compatibility, or alternate implementation belongs in the active source tree or application catalogue.

OFS is separate from FFS and SFS. Shared AmigaDOS format concepts do not imply a shared filesystem engine.

Qualification requires supported-target builds, destructive read/write verification, independently created test media, and source review confirming that the active implementation is project-authored code.
