#include "fs.h"
void _putc(char ch);
off_t _lseek(int fd, off_t offset, int whence);
void dispinfo_read(void *buf, off_t offset, size_t len);
void fb_write(const void *buf, off_t offset, size_t len);
size_t events_read(void *buf, size_t len) ;

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;  
  off_t open_offset;  // 文件被打开之后的读写指针

} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))
#define FB_FD 3

void init_fs() {
  file_table[FB_FD].size = _screen.width*_screen.height*sizeof(int);
}


int fs_open(const char *pathname, int flags, int mode){
  int fd=0;
  while(fd<NR_FILES){
    if(strcmp(pathname, file_table[fd].name)==0){
      file_table[fd].open_offset = 0;
      Log("[in fs_open] file name: %s",pathname);
      Log("[in fs_open] file discripter: %d",fd);
      Log("[in fs_open] file size: %d",file_table[fd].size);
      Log("[in fs_open] disk_offset: %d",file_table[fd].disk_offset);
      return fd;
    }
    ++fd;
  }
  assert(0);
  return -1;
}

int fs_close(int fd){
  return 0; //always succeed
}

size_t fs_filesz(int fd){
  assert(fd>=0&&fd<NR_FILES);
  return file_table[fd].size;
}

ssize_t fs_read(int fd, void *buf, size_t len){

  int real_len = file_table[fd].size-file_table[fd].open_offset;
  if(real_len<=0){
      return 0;
  }else{
      real_len = (real_len<len)?real_len:len;
  }
  off_t offset = file_table[fd].open_offset+file_table[fd].disk_offset;

  switch(fd){
    case FD_EVENTS:{
      Log("[in fs_read] reach events!");
      return events_read(buf, len);
    }
    case FD_DISPINFO:{
      dispinfo_read(buf, offset, real_len) ;
      break;
    }
    default:{
      ramdisk_read(buf,offset,real_len);
      break;
    }
  }
  file_table[fd].open_offset += len;
  return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len){
  // Log("[in fs_write]  fd: %d ",fd);
  // Log("[in fs_write]  write len: %d \n",len);
  // Log("[in fs_write]  filesz: %d \n",fs_filesz(fd));
  switch(fd){
    case FD_STDOUT:
    case FD_STDERR:{
      char* buffer = (char*)buf;
      for(int i=0;i<len;i++){
        _putc( *(buffer+i));
      }
      ssize_t reval = (fd==1)?len:-1;
      return reval;
    }
    case FD_FB:{
      int real_len = (int)file_table[fd].size-(int)file_table[fd].open_offset;
      if(real_len<=0){
        return 0;
      }else{
        real_len = (real_len<len)?real_len:len;
      }

      off_t offset = file_table[fd].open_offset+file_table[fd].disk_offset;
      //printf("len:%d   ; real_len :%d  \n",len,real_len);
      fb_write(buf, offset, real_len);
      break;
    }
    default:{
      int real_len = file_table[fd].size-file_table[fd].open_offset;
      if(real_len<=0){
        return 0;
      }else{
        real_len = (real_len<len)?real_len:len;
      }

      off_t offset = file_table[fd].open_offset+file_table[fd].disk_offset;
      ramdisk_write(buf,offset,real_len);
      break;
    }
  }
  file_table[fd].open_offset += len;
  return len;
}


off_t fs_lseek(int fd, off_t offset, int whence){
  switch(whence){
    case SEEK_SET:
      file_table[fd].open_offset=offset;
      break;
    case SEEK_CUR:
      file_table[fd].open_offset+=offset;
      break;
    case SEEK_END:
      file_table[fd].open_offset=offset+file_table[fd].size;
      break;
    default:
      assert(0);
    }
  return file_table[fd].open_offset;
}


