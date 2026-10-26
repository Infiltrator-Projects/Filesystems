# Filesystem implementation source

The Linux source in each filesystem's `linux/` directory is editable
implementation code, whether copied from a mature upstream implementation or
subsequently modified by Filesystems. Imported code is **not** a disposable
reference cache. Its actual original authorship and licence notices remain
intact unless and until the affected implementation is independently replaced.

There are **22 additional imported Linux source trees** enumerated by
`data/imported-source-filesystems.txt`. They are available for direct
engineering changes. The original NTFS3 implementation is held under the
single NTFS identity as part of the **13 managed native Linux drivers**,
rather than as an additional imported-only source identity.

Those 13 managed filesystem/module pairs are enumerated exclusively by
`data/native-filesystems.tsv`. They are built and deployed only through
their existing qualification and installation workflows. Copying an original
working driver into the working tree is **not** an automatic claim that its
out-of-tree compilation, integration or current runtime is qualified.

A canonical `core/` can separate platform-independent semantics from
an existing Linux adapter *without discarding working source*. An imported
implementation may be modified directly and still retain its original
authorship. A project-authored rewrite is a distinct, provenance-evidenced
engineering milestone, not the automatic consequence of copying, moving or
renaming code. Correct functionality and independently demonstrated
qualification are required in either case.

The **41 catalogue identities**, **13 managed Linux modules**, **22
imported-only Linux trees**, **two original userspace baselines**, and
**four not-in-this-repository source baselines** are different inventories.
See `docs/SOURCE_BASELINE_STATUS.md`; these counts are derived from their
respective manifests, not separate driver-completeness claims.

For SFS, SFS2 and PFS3, retained copies of the earlier working upstream implementations are under `original/` as separately editable code snapshots. Their qualified native `linux/` adapters are unchanged. No `reference/` source quarantine remains.

The Minix module was promoted from the original Linux Kbuild source after Debian header symbol linking and a real Minix V3 QEMU read/write/remount test. It intentionally has no invented empty `core/` tree; its complete format implementation remains in `linux/`. Other filesystem imports stay outside the managed manifest until independently qualified.
