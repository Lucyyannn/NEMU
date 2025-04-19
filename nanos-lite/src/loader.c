#include "common.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

size_t get_ramdisk_size();

int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);
size_t fs_filesz(int fd);

void* _map(_Protect *p, void *va, int* cnt);

uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename,0,0);
  int file_size = fs_filesz(fd);
  Log("%d,%x,%x-%x",file_size,as->ptr,as->area.start,as->area.end);
  void* cur_va = DEFAULT_ENTRY;
  void* cur_pa = NULL;
  for(int i=0;i<file_size;i+=4096){
    int c=0;
    cur_pa = _map(as,cur_va,&c);
    assert(cur_pa);
    //Log("va:%x,pa:%x,%d",(uint32_t)cur_va,(uint32_t)cur_pa,c);
    fs_read(fd,cur_pa,4096);
    cur_va += 4096;
  }
  return (uintptr_t)DEFAULT_ENTRY;
}
