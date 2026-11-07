// SPDX-License-Identifier: GPL-3.0-or-later
// Tiny synthetic ELF metadata fixture, NOT a functional filesystem driver.
#ifndef IFS_TEST_WIDTH
#error "Must choose a FAT width for the fixture"
#endif
#define IFS_STRINGIFY_INNER(x) #x
#define IFS_STRINGIFY(x) IFS_STRINGIFY_INNER(x)
__attribute__((used, section(".ifsmeta")))
static const char kFormatIdentity[] = "IFS-MODULE:fat" IFS_STRINGIFY(IFS_TEST_WIDTH) ":1";
__attribute__((visibility("default")))
int infiltrator_filesystem_module_v1(void) { return IFS_TEST_WIDTH; }
