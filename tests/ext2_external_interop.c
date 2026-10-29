/* Independently generated EXT2 image via compiled EXT2.so. */
#define _POSIX_C_SOURCE 200809L
#include "independent_image_io.h"
#include "../native/filesystems/ext2/module/ext2_module.h"
#include <stdio.h>
#include <string.h>
static int fail(const char*s){fprintf(stderr,"EXT2 real-image test: %s\n",s);return 1;}
int main(int argc,char**argv){
 TestDisk d;IfsExt2DropInIo io;IfsExt2DropInVolume v;
 IfsExt2Inode f;unsigned char scratch[4096],check[24];
 ifs_ext2_u32 readcount=0,written=0;
 const IfsExt2DropInModule*api=infiltrator_filesystem_module_v1();
 const char patch[]="EXT2-RW-OK!";
 if(argc!=2)return fail("usage: tester disposable.img");
 if(disk_open(&d,argv[1])!=0)return fail("refused non-regular disk image");
 io.device=(IfsExt2Io){disk_read,disk_write,disk_flush,&d};
 io.volume_bytes=d.bytes;memset(&v,0,sizeof(v));
 if(!api||strcmp(api->format_id,"ext2")||api->abi_version!=1u||
    !(api->capabilities&IFS_EXT2_CAP_WRITE_EXISTING))
   return fail("missing genuine EXT2.so ABI");
 if(api->mount(&v,&io,1)!=IFS_EXT2_OK||v.fs.block_size>sizeof(scratch))
   return fail("mke2fs image not accepted for bounded writes");
 if(api->lookup(&v,IFS_EXT2_ROOT_INODE,"ROUND.BIN",9u,&f,
                scratch,sizeof(scratch))!=IFS_EXT2_OK||f.size!=16384u)
   return fail("debugfs-created file not found");
 if(api->read(&v,f.number,2046u,check,sizeof(patch)-1u,
              scratch,sizeof(scratch),&readcount)!=IFS_EXT2_OK||
    readcount!=sizeof(patch)-1u)return fail("initial file read");
 for(size_t i=0;i<sizeof(patch)-1u;i++)if(check[i]!='A')
   return fail("initial bytes differ");
 if(api->write_existing(&v,f.number,2046u,patch,sizeof(patch)-1u,
                        scratch,sizeof(scratch),&written)!=IFS_EXT2_OK||
    written!=sizeof(patch)-1u||!d.writes||d.flushes!=1u||
    api->read(&v,f.number,2046u,check,sizeof(patch)-1u,
              scratch,sizeof(scratch),&readcount)!=IFS_EXT2_OK||
    memcmp(check,patch,sizeof(patch)-1u))
   return fail("cross-block write/readback/flush failed");
 {
 unsigned before=d.writes;
 if(api->write_existing(&v,f.number,f.size-1u,patch,2u,
                        scratch,sizeof(scratch),&written)==IFS_EXT2_OK||
    d.writes!=before)return fail("out-of-file write accepted");
 }
 if(close(d.fd)!=0)return fail("close");
 puts("EXT2-INDEPENDENT-MKE2FS-DEBUGFS-RW-PASS");return 0;
}
