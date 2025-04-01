#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

int fs_open(const char *pathname, int flags, int mode);
size_t fs_filesz(int fd);
ssize_t fs_read(int fd, void *buf, size_t len);
int fs_close(int fd);
void initSize(int fd,size_t size);

uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename,0,0);
  fs_read(fd,DEFAULT_ENTRY,fs_filesz(fd));
  initSize(fd,fs_filesz(fd));
  printf("filesz when load: %d \n",fs_filesz(fd));

  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
