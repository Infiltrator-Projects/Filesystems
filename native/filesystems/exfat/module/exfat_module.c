/* SPDX-License-Identifier: GPL-3.0-or-later
 * Independent exFAT shared-object entry point. Compile the same audited FatFs
 * source into this one format-specific binary; never proxy to Linux exfat.ko.
 */
#if !defined(IFS_FATFS_ONLY) || IFS_FATFS_ONLY != 64
#error "This translation unit is only for the exFAT engine"
#endif
#include "../../fat/portable/fatfs_module.c"
