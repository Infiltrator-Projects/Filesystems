/* Independently generated FAT16 image via the compiled FAT16.so. */
#define _POSIX_C_SOURCE 200809L
#include "independent_image_io.h"
#include "../native/filesystems/fat/module/fat16_module.h"
#include <stdio.h>
#include <string.h>
static int fail(const char*s){fprintf(stderr,"FAT16 real-image test: %s\n",s);return 1;}
int main(int argc,char**argv){
 TestDisk d; IfsClassicFatIo rd;IfsClassicFatWritableIo wr;
 IfsFat16Volume vol;IfsFat16File f;uint32_t fragments=0;char check[24];
 const IfsFat16Module*v1=infiltrator_filesystem_module_v1();
 const IfsClassicFatModuleV2*v2=infiltrator_filesystem_module_v2();
 const char patch[]="FAT16-RW-OK";
 if(argc!=2)return fail("usage: tester disposable.img");
 if(disk_open(&d,argv[1])!=0)return fail("refused non-regular disk image");
 rd=(IfsClassicFatIo){disk_read,&d,d.bytes};
 wr=(IfsClassicFatWritableIo){rd,disk_write,disk_flush};
 if(!v1||!v2||strcmp(v1->format_id,"fat16")||v2->readonly_v1!=v1||
    v2->abi_version!=IFS_CLASSIC_FAT_V2_ABI_VERSION)
   return fail("missing genuine V1/V2 ABI");
 if(v1->probe(&rd,&vol)||v1->root_file(&rd,&vol,0,&f)||
    strcmp(f.name,"ROUND.BIN")||f.size_bytes!=16384u)
   return fail("mkfs.fat/mtools image not readable");
 if(v1->root_file_fragments(&rd,&vol,&f,&fragments)||!fragments||
    v1->read_file(&rd,&vol,&f,2046u,check,sizeof(patch)-1u))
   return fail("original data/cluster mapping inaccessible");
 for(size_t i=0;i<sizeof(patch)-1u;i++)if(check[i]!='A')
   return fail("original data mismatch");
 if(v2->overwrite_existing(&wr,&vol,&f,2046u,patch,sizeof(patch)-1u)||
    !d.writes||d.flushes!=1u||v1->read_file(&rd,&vol,&f,2046u,
                                            check,sizeof(patch)-1u)||
    memcmp(check,patch,sizeof(patch)-1u))
   return fail("cross-cluster overwrite/readback/flush failed");
 {
 unsigned before=d.writes; IfsFat16File forged=f;forged.first_cluster++;
 if(v2->overwrite_existing(&wr,&vol,&forged,0u,patch,1u)==0||
    v2->overwrite_existing(&wr,&vol,&f,f.size_bytes-1u,patch,2u)==0||
    d.writes!=before)return fail("forged or out-of-range write accepted");
 }
 if(close(d.fd)!=0)return fail("close");
 puts("FAT16-INDEPENDENT-MKFS-MTOOLS-RW-PASS");return 0;
}
