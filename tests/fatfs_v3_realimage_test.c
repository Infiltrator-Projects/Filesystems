/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Independent DOS-formatted FAT12/FAT16 image interoperability (no devices).
 * Both media files are produced by dosfstools, not the reader under test.
 */
#define _POSIX_C_SOURCE 200809L
#include "fatfs_module.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

static int read_at(void *u,uint64_t at,void *dst,uint32_t bytes) {
    FILE *f=(FILE *)u;
    if(at>INT64_MAX || fseeko(f,(off_t)at,SEEK_SET)!=0) return -1;
    return fread(dst,1,bytes,f)==bytes ? 0 : -1;
}
static int write_at(void *u,uint64_t at,const void *src,uint32_t bytes) {
    FILE *f=(FILE *)u;
    if(at>INT64_MAX || fseeko(f,(off_t)at,SEEK_SET)!=0) return -1;
    return fwrite(src,1,bytes,f)==bytes ? 0 : -1;
}
static int flush_at(void *u) {
    FILE *f=(FILE *)u;
    return fflush(f)==0 && fsync(fileno(f))==0 ? 0 : -1;
}
static uint32_t gettime_at(void *u) {
    (void)u;
    /* FAT 2026-10-28 12:30:00 local. */
    return ((uint32_t)(2026-1980)<<25) | (10u<<21) | (28u<<16) |
           (12u<<11) | (30u<<5);
}
static int die(const char *msg) {fprintf(stderr,"FAT-IMAGE-FAIL: %s\n",msg);return 1;}
static int setup(const char *path,FILE **f,IfsFatfsIo *io)
{
    off_t length;
    *f=fopen(path,"r+b");
    if(!*f || fseeko(*f,0,SEEK_END)!=0 ||
       (length=ftello(*f))<=0) return -1;
    io->user=*f;
    io->partition_bytes=(uint64_t)length;
    io->sector_bytes=512u;
    io->read_at=read_at;
    io->write_at=write_at;
    io->flush=flush_at;
    io->fat_timestamp=gettime_at;
    return 0;
}
int main(int argc,char **argv) {
    const IfsFatfsModuleV3 *api;
    const char *name="0:/SUBDIR/A Genuine Long Filename Example.txt";
    const char *renamed="0:/SUBDIR/Verified Long Filename Data.txt";
    const char *dir="0:/SUBDIR";
    FILE *f=0;
    IfsFatfsIo io;
    IfsFatfsEntry metadata,entry;
    uint8_t data[3072],out[3072];
    void *handle;
    int found=0;
    const IfsFatfsModuleV3 *(*resolve)(void)=0;
    if(argc!=4) return die("arguments: module own-image other-variant-image");
    handle=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);
    if(!handle) {fprintf(stderr,"%s\n",dlerror());return die("dlopen");}
    *(void **)(&resolve)=dlsym(handle,"infiltrator_filesystem_module_v3");
    api=resolve?resolve():0;
    if(!api || api->abi_version!=3u || api->structure_bytes!=sizeof(*api) ||
       !api->mount || !api->unmount || !api->create || !api->read ||
       !api->write || !api->truncate || !api->mkdir ||
       !api->unlink || !api->rename || !api->stat || !api->directory_entry)
        return die("V3 functions not present");
    if(setup(argv[3],&f,&io)!=0) return die("wrong image setup");
    if(api->mount(&io)==0) return die("mounted incorrect FAT variant");
    if(fclose(f)!=0) return die("close wrong media");
    if(setup(argv[2],&f,&io)!=0) return die("own image setup");
    if(api->mount(&io)!=0) return die("cannot mount correctly formatted image");
    if(api->mkdir(dir)!=0) return die("create nested directory");
    if(api->create(name)!=0) return die("create long-name file");
    for(unsigned i=0;i<sizeof data;i++) data[i]=(uint8_t)((i*17u+23u)&255u);
    if(api->write(name,0u,data,2048u)!=0 ||
       api->write(name,2048u,data+2048u,1024u)!=0)
        return die("write/allocate/append");
    memset(out,0,sizeof out);
    if(api->read(name,0,out,sizeof out)!=0 ||
       memcmp(out,data,sizeof data)!=0) return die("immediate readback");
    if(api->rename(name,renamed)!=0 || api->stat(renamed,&metadata)!=0 ||
       metadata.bytes!=3072u) return die("rename and stat");
    for(uint32_t i=0;i<64u;++i) {
        const int r=api->directory_entry(dir,i,&entry);
        if(r==1) break;
        if(r!=0) return die("directory enumeration failed");
        if(strcmp(entry.name,"Verified Long Filename Data.txt")==0) found=1;
    }
    if(!found) return die("long filename not independently enumerated");
    if(api->unmount()!=0 || flush_at(f)!=0) return die("unmount/flush");
    if(api->mount(&io)!=0) return die("remount same image");
    memset(out,0,sizeof out);
    if(api->read(renamed,0u,out,sizeof out)!=0 ||
       memcmp(out,data,sizeof data)!=0) return die("remount persistence");
    if(api->truncate(renamed,1000u)!=0 ||
       api->stat(renamed,&metadata)!=0 || metadata.bytes!=1000u)
        return die("truncate");
    if(api->unmount()!=0 || api->mount(&io)!=0 ||
       api->stat(renamed,&metadata)!=0 || metadata.bytes!=1000u)
        return die("truncate persistence");
    if(api->unlink(renamed)!=0 || api->unlink(dir)!=0)
        return die("delete file and directory");
    if(api->unmount()!=0 || flush_at(f)!=0 || fclose(f)!=0)
        return die("final flush");
    if(dlclose(handle)!=0) return die("close module");
    printf("FAT12-FAT16-CROSS-FORMAT-DOSFS-REAL-IMAGE-READ-WRITE-PASS: %s\n",argv[1]);
    return 0;
}
