/* SPDX-License-Identifier: GPL-3.0-or-later
 * Adapter for source-owned ChaN FatFs R0.16 patch2. One active image per
 * standalone module: caller must ensure exclusive media ownership and never
 * concurrently invoke this ABI. A .so is not a Linux VFS mount or EFI driver.
 * Formatting is a separate capability; no format function advertised.
 */
#include "fatfs_module.h"
#include "ff.h"
#include "diskio.h"

#if defined(IFS_FATFS_ONLY) && IFS_FATFS_ONLY == 12
#define REQUIRED_FS FS_FAT12
#define FS_ID "fat12"
#elif defined(IFS_FATFS_ONLY) && IFS_FATFS_ONLY == 16
#define REQUIRED_FS FS_FAT16
#define FS_ID "fat16"
#else
#error "Compile one FAT12 or FAT16 standalone module only"
#endif

static IfsFatfsIo device;
static FATFS volume;
static int selected;
static int bound;
static int sector_valid(void)
{
    return device.read_at && device.write_at && device.flush &&
           device.fat_timestamp &&
           device.sector_bytes >= 512u && device.sector_bytes <= 4096u &&
           (device.sector_bytes & (device.sector_bytes - 1u)) == 0u &&
           device.partition_bytes >= 16u * device.sector_bytes &&
           device.partition_bytes % device.sector_bytes == 0u &&
           device.partition_bytes / device.sector_bytes <= UINT32_MAX;
}
static int io_valid(uint64_t sector, UINT count)
{
    uint64_t n = (uint64_t)count * device.sector_bytes;
    return bound && sector_valid() && count &&
           sector <= device.partition_bytes / device.sector_bytes &&
           n <= device.partition_bytes - sector * device.sector_bytes &&
           n <= UINT32_MAX;
}
DSTATUS disk_initialize(BYTE pdrv)
{
    return pdrv == 0 && bound && sector_valid() ? 0 : STA_NOINIT;
}
DSTATUS disk_status(BYTE pdrv)
{
    return disk_initialize(pdrv);
}
DRESULT disk_read(BYTE pdrv, BYTE *dst, LBA_t sector, UINT count)
{
    if (pdrv || !dst || !io_valid((uint64_t)sector, count))
        return RES_PARERR;
    return device.read_at(device.user, (uint64_t)sector*device.sector_bytes,
                          dst, count*device.sector_bytes) == 0 ?
           RES_OK : RES_ERROR;
}
DRESULT disk_write(BYTE pdrv, const BYTE *src, LBA_t sector, UINT count)
{
    if (pdrv || !src || !io_valid((uint64_t)sector, count))
        return RES_PARERR;
    return device.write_at(device.user, (uint64_t)sector*device.sector_bytes,
                           src, count*device.sector_bytes) == 0 ?
           RES_OK : RES_ERROR;
}
DRESULT disk_ioctl(BYTE pdrv, BYTE command, void *out)
{
    if (pdrv || !bound || !sector_valid()) return RES_PARERR;
    switch (command) {
    case CTRL_SYNC:
        return device.flush(device.user) == 0 ? RES_OK : RES_ERROR;
    case GET_SECTOR_COUNT:
        if (!out) return RES_PARERR;
        *(LBA_t *)out = (LBA_t)(device.partition_bytes/device.sector_bytes);
        return RES_OK;
    case GET_SECTOR_SIZE:
        if (!out) return RES_PARERR;
        *(WORD *)out = (WORD)device.sector_bytes;
        return RES_OK;
    case GET_BLOCK_SIZE:
        if (!out) return RES_PARERR;
        *(DWORD *)out = 1u;
        return RES_OK;
    default: return RES_PARERR;
    }
}
DWORD get_fattime(void)
{
    return bound && device.fat_timestamp ?
           (DWORD)device.fat_timestamp(device.user) : 0u;
}
static int attach(const IfsFatfsIo *io)
{
    if (bound || selected || !io) return -1;
    device = *io;
    bound = 1;
    if (!sector_valid() || f_mount(&volume, "0:", 1) != FR_OK ||
        volume.fs_type != REQUIRED_FS) {
        (void)f_mount(0, "0:", 0);
        bound = 0;
        return -1;
    }
    selected = 1;
    return 0;
}
static int detach(void)
{
    if (!selected) return -1;
    if (f_mount(0, "0:", 0) != FR_OK) return -1;
    selected = 0;
    bound = 0;
    return 0;
}
static int create_file(const char *path)
{
    FIL f;
    if (!selected || !path || f_open(&f, path, FA_WRITE | FA_CREATE_NEW) != FR_OK)
        return -1;
    return f_close(&f) == FR_OK ? 0 : -1;
}
static int file_read(const char *path, uint32_t offset, void *buffer, uint32_t n)
{
    FIL f;
    UINT got=0;
    int good;
    if (!selected || !path || (!buffer && n) ||
        f_open(&f, path, FA_READ) != FR_OK) return -1;
    good=(offset <= f_size(&f)) &&
         (n <= f_size(&f)-(FSIZE_t)offset) &&
         f_lseek(&f, (FSIZE_t)offset) == FR_OK &&
         f_read(&f,buffer,n,&got)==FR_OK && got==n;
    if (f_close(&f)!=FR_OK) good=0;
    return good?0:-1;
}
static int file_write(const char *path, uint32_t offset, const void *buffer, uint32_t n)
{
    FIL f;
    UINT written=0;
    int good;
    if (!selected || !path || (!buffer && n) || n>UINT32_MAX-offset ||
        f_open(&f,path,FA_WRITE|FA_OPEN_EXISTING)!=FR_OK) return -1;
    good=f_lseek(&f,(FSIZE_t)offset)==FR_OK &&
         f_write(&f,buffer,n,&written)==FR_OK && written==n &&
         f_sync(&f)==FR_OK;
    if (f_close(&f)!=FR_OK) good=0;
    return good?0:-1;
}
static int file_truncate(const char *path, uint32_t bytes)
{
    FIL f;
    int good;
    if (!selected || !path ||
        f_open(&f,path,FA_WRITE|FA_OPEN_EXISTING)!=FR_OK) return -1;
    good=f_lseek(&f,(FSIZE_t)bytes)==FR_OK &&
         f_truncate(&f)==FR_OK && f_sync(&f)==FR_OK;
    if (f_close(&f)!=FR_OK) good=0;
    return good?0:-1;
}
static int make_dir(const char *path) { return selected && path && f_mkdir(path)==FR_OK?0:-1; }
static int remove_path(const char *path) { return selected && path && f_unlink(path)==FR_OK?0:-1; }
static int rename_path(const char *from,const char *to) {
    return selected && from && to && f_rename(from,to)==FR_OK?0:-1;
}
static void copy_name(char out[768],const char *in)
{
    unsigned i=0;
    while (i<767u && in[i]) {out[i]=in[i];++i;}
    out[i]='\0';
}
static int stat_path(const char *path, IfsFatfsEntry *out)
{
    FILINFO i;
    if (!selected || !path || !out || f_stat(path,&i)!=FR_OK) return -1;
    copy_name(out->name,i.fname);
    out->bytes=(uint32_t)i.fsize;
    out->attributes=i.fattrib;
    return 0;
}
static int nth_dir_entry(const char *path,uint32_t nth,IfsFatfsEntry *out)
{
    DIR d;
    FILINFO i;
    int ok=0;
    if(!selected || !path || !out || f_opendir(&d,path)!=FR_OK) return -1;
    for(uint32_t n=0;n<=nth;++n) {
        if(f_readdir(&d,&i)!=FR_OK) goto done;
        if(i.fname[0]=='\0') {ok=1;goto done;}
    }
    copy_name(out->name,i.fname);
    out->bytes=(uint32_t)i.fsize;
    out->attributes=i.fattrib;
    ok=2;
done:
    if(f_closedir(&d)!=FR_OK) return -1;
    return ok==2?0:ok==1?1:-1;
}
__attribute__((used,section(".ifsmeta")))
static const char identity[]="IFS-MODULE:" FS_ID ":3";
static const IfsFatfsModuleV3 module={
    IFS_FATFS_MODULE_ABI,sizeof(IfsFatfsModuleV3),FS_ID,
    IFS_FATFS_CAP_READ | IFS_FATFS_CAP_WRITE_EXISTING |
    IFS_FATFS_CAP_CREATE | IFS_FATFS_CAP_DELETE | IFS_FATFS_CAP_RENAME |
    IFS_FATFS_CAP_DIRECTORY | IFS_FATFS_CAP_TRUNCATE,
    attach,detach,create_file,file_read,file_write,file_truncate,
    make_dir,remove_path,rename_path,stat_path,nth_dir_entry
};
__attribute__((visibility("default")))
const IfsFatfsModuleV3 *infiltrator_filesystem_module_v3(void) {return &module;}
