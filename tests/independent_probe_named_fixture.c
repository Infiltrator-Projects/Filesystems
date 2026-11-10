// SPDX-License-Identifier: GPL-3.0-or-later
// Non-installable ELF inspection fixture: never execute this as a filesystem.
#if defined(IFS_TEST_FFS)
#define IFS_TEST_FORMAT_ID "ffs"
#elif defined(IFS_TEST_EXFAT)
#define IFS_TEST_FORMAT_ID "exfat"
#else
#error "Choose a single fixture format"
#endif
__attribute__((used,section(".ifsmeta")))
static const char test_identity[]="IFS-MODULE:" IFS_TEST_FORMAT_ID ":common2";
__attribute__((visibility("default")))
const char *infiltrator_filesystem_api_v2(void) { return test_identity; }
