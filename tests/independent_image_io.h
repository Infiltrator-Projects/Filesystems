/* CI-only disposable image I/O. Reject block devices and symlinks. */
#ifndef IFS_TEST_IMAGE_IO_H
#define IFS_TEST_IMAGE_IO_H
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
typedef struct { int fd; uint64_t bytes; unsigned writes, flushes; } TestDisk;
static int disk_open(TestDisk *d,const char *path) {
 struct stat s;
 d->fd=open(path,O_RDWR|O_CLOEXEC|O_NOFOLLOW); d->writes=d->flushes=0;
 if(d->fd<0)return -1;
 if(fstat(d->fd,&s)!=0 || !S_ISREG(s.st_mode) || s.st_size<0 ||
    (uint64_t)s.st_size>INT64_C(2147483647)) {close(d->fd);d->fd=-1;return -1;}
 d->bytes=(uint64_t)s.st_size;return 0;
}
static int disk_read(void *p,uint64_t off,void *buf,uint32_t len) {
 TestDisk *d=(TestDisk*)p;uint32_t n=0;
 if(!d||d->fd<0||(!buf&&len)||off>d->bytes||len>d->bytes-off)return -1;
 while(n<len) {ssize_t r=pread(d->fd,(char*)buf+n,len-n,(off_t)(off+n));
   if (r < 0 && errno == EINTR) continue;
   if (r <= 0) return -1;
   n += (uint32_t)r;}
 return 0;
}
static int disk_write(void *p,uint64_t off,const void *buf,uint32_t len) {
 TestDisk *d=(TestDisk*)p;uint32_t n=0;
 if(!d||d->fd<0||(!buf&&len)||off>d->bytes||len>d->bytes-off)return -1;
 while(n<len) {ssize_t r=pwrite(d->fd,(const char*)buf+n,len-n,(off_t)(off+n));
   if (r < 0 && errno == EINTR) continue;
   if (r <= 0) return -1;
   n += (uint32_t)r;}
 if (len) ++d->writes;
 return 0;
}
static int disk_flush(void *p) {TestDisk *d=(TestDisk*)p;
 if(!d||d->fd<0||fsync(d->fd)!=0)return -1;++d->flushes;return 0;
}
#endif
