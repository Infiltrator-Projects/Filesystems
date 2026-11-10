// SPDX-License-Identifier: GPL-3.0-or-later
// Non-installable ELF inspection fixture with the exact V2 format marker.
#ifndef IFS_TEST_WIDTH
#error "Must choose one FAT width"
#endif
#define IFS_STRINGIFY_INNER(x) #x
#define IFS_STRINGIFY(x) IFS_STRINGIFY_INNER(x)
__attribute__((used,section(".ifsmeta")))
static const char test_identity[]=
    "IFS-MODULE:fat" IFS_STRINGIFY(IFS_TEST_WIDTH) ":common2";
__attribute__((visibility("default")))
const char *infiltrator_filesystem_api_v2(void) { return test_identity; }
