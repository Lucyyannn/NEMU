#include "common.h"
#include <stdlib.h>

#define DEFAULT_ENTRY ((void *)0x4000000)

int fs_open(const char *pathname, int flags, int mode);
size_t fs_filesz(int fd);
ssize_t fs_read(int fd, void *buf, size_t len);


uintptr_t loader(_Protect *as, const char *filename) {
  //size_t len = get_ramdisk_size();

  int fd = fs_open(filename,0,0);
  fs_read(fd,DEFAULT_ENTRY,fs_filesz(fd));

  return (uintptr_t)DEFAULT_ENTRY;
}
