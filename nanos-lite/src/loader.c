#include "common.h"

#define DEFAULT_ENTRY ((void *)0x8048000)
#define PAGE_SIZE 4096

int fs_open(const char *pathname, int flags, int mode);
size_t fs_filesz(int fd);
ssize_t fs_read(int fd, void *buf, size_t len);
int fs_close(int fd);
void* new_page(void);
void _map(_Protect *p, void *va, void *pa);

uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename,0,0);
  size_t filesz = fs_filesz(fd);
  int page_num = filesz/PAGE_SIZE; //total_pages+rest_content
  if(filesz%PAGE_SIZE>0){page_num+=1;}

  void* va = DEFAULT_ENTRY;//load the program by DEFAULT_ENTRY in vitual logic
  for(int i=0;i<page_num;i++){
    void* pa = new_page();
    fs_read(fd,pa,PAGE_SIZE);//load
    //Log("load file: %s , at physical addr: %08X",filename,pa);
    _map(as, va, pa);//record the reflection
    va+=PAGE_SIZE;
  }
  fs_close(fd);
  
  return (uintptr_t)DEFAULT_ENTRY;
 }
// uintptr_t loader(_Protect *as, const char *filename) {
//   int fd = fs_open(filename,0,0);
//   int file_size = fs_filesz(fd);
//   Log("%d,%x,%x-%x",file_size,as->ptr,as->area.start,as->area.end);
//   void* cur_va = DEFAULT_ENTRY;
//   void* cur_pa = NULL;
//   for(int i=0;i<file_size;i+=4096){
//     int c=0;
//     cur_pa = _map(as,cur_va,&c);
//     assert(cur_pa);
//     //Log("va:%x,pa:%x,%d",(uint32_t)cur_va,(uint32_t)cur_pa,c);
//     fs_read(fd,cur_pa,4096);
//     cur_va += 4096;
//   }
//   return (uintptr_t)DEFAULT_ENTRY;
// }
