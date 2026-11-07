// SPDX-License-Identifier: GPL-3.0-or-later
// Tiny synthetic ELF identity fixture, NOT a functional filesystem driver.
#if defined(IFS_TEST_FFS)
__attribute__((visibility("default")))
int infiltrator_filesystem_ffs_defrag_v1(void) { return 1; }
#elif defined(IFS_TEST_EXFAT)
__attribute__((visibility("default")))
int infiltrator_filesystem_exfat_defrag_v1(void) { return 1; }
#else
#error "Choose a single fixture format"
#endif
