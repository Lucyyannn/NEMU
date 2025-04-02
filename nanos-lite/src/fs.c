#include "fs.h"
void _putc(char ch);

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

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

off_t _lseek(int fd, off_t offset, int whence);

int fs_open(const char *pathname, int flags, int mode){
  int fd=0;
  while(fd<NR_FILES){
    if(strcmp(pathname, file_table[fd].name)==0){
      file_table[fd].open_offset = 0;
      // Log("[in fs_open] file name: %s",pathname);
      // Log("[in fs_open] file discripter: %d",fd);
      // Log("[in fs_open] file size: %d",file_table[fd].size);
      // Log("[in fs_open] disk_offset: %d",file_table[fd].disk_offset);
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
  assert(len>=0 && len<=fs_filesz(fd));
  off_t offset = file_table[fd].disk_offset+file_table[fd].open_offset;// careful!!
  ramdisk_read(buf,offset,len);
  file_table[fd].open_offset += len;

  return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len){
  // Log("[in fs_write]  fd: %d ",fd);
  // Log("[in fs_write]  write len: %d \n",len);
  // Log("[in fs_write]  filesz: %d \n",fs_filesz(fd));
  if(fd==1||fd==2){//stdout stderr
    char* buffer = (char*)buf;
    for(int i=0;i<len;i++){
      _putc( *(buffer+i));
    }
    ssize_t reval = (fd==1)?len:-1;
    return reval;
  }
  assert(len>=0 && len<=fs_filesz(fd));
  off_t offset = file_table[fd].disk_offset+file_table[fd].open_offset;// careful!!
  ramdisk_write(buf,offset,len);
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


