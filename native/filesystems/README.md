# Filesystem implementation source

The Linux source in each filesystem's `linux/` directory is editable
implementation code, whether copied from a mature upstream implementation or
subsequently modified by Filesystems. Imported code is **not** a disposable
reference cache. Its actual original authorship and licence notices remain
intact unless and until the affected implementation is independently replaced.

There are 26 independently copied Linux source baselines enumerated by
`data/imported-source-filesystems.txt`, including NTFS copied from the
original historical NTFS3 tree into the single NTFS identity.
These directories are available for direct code modifications.

The eight independently managed native Linux drivers are listed instead in
`data/native-filesystems.tsv`. They are built and deployed only through
their existing qualification and installation workflows. Copying an original
working driver into the working tree is **not** an automatic claim that its
out-of-tree compilation, integration or current runtime is qualified.

A future canonical `core/` can separate platform-independent semantics from
the existing Linux adapter *without discarding the working source*. The
requirement is correct functionality, stable provenance and evidenced
qualification, not rewriting code simply to change attribution.
