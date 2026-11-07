# Remaining unextracted filesystem implementation source

This directory belongs to the **temporary Filesystems manager repository**,
which is not yet retired. Its actual retained format implementations are listed
in [`data/source-filesystems.txt`](../../data/source-filesystems.txt). At this
revision the manifest names **21 formats**; some contain imported Linux source,
some userspace source and some locally maintained adapters.

The distinct Linux native-module install manifest is
[`data/native-filesystems.tsv`](../../data/native-filesystems.tsv), and
currently lists only **BFS and UDF**. A format's presence in either manifest
does not establish complete functionality, successful out-of-tree kernel
compilation, or safety on physical media.

Already extracted formats (including FFS, exFAT, FAT12/16/32, EXT2/3/4,
OFS, SFS, SFS2, PFS3 and others) must be maintained **only in their own
`Filesystem-*` Git repositories**. They do not belong here, even as source
stubs, copies or provider Git submodules. The independent repository owns its
standalone `.so`, format-specific code, any separately qualified `.ko` or
firmware support, tests and generated manual.

Do not automatically refresh imported source or erase original attribution
and licence notices. Source ownership transfer and runtime qualification are
separate operations.
