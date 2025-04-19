#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},//VGA write lseek
  [FD_EVENTS] = {"/dev/events", 0, 0},//read
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},//read
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

size_t fs_filesz(int fd){
  return file_table[fd].size;
};

int fs_open(const char *pathname, int flags, int mode){//flags mode unuse!
  for(int i=0;i<NR_FILES;i++){
    if(strcmp(file_table[i].name,pathname)==0){
      file_table[i].open_offset=0;
      return i;
    }
  }
  assert(0);
  return -1;
};

void ramdisk_read(void *buf, off_t offset, size_t len);
void ramdisk_write(const void *buf, off_t offset, size_t len);

ssize_t dispinfo_read(void *buf, off_t offset, size_t len);
ssize_t events_read(void *buf, size_t len);

ssize_t fb_write(const void *buf, off_t offset, size_t len);

ssize_t sys_write(int fd,char* buf,size_t count){
  int i;
  for(i=0;i<count;i++){
    _putc(*(buf+i));
  }
  return i;
}

ssize_t fs_read(int fd, void *buf, size_t len){
  //ignore stdin stdout stderr
  assert(fd>=0&&fd<NR_FILES);
  if(fd==FD_DISPINFO){
    int read_bytes = dispinfo_read(buf,file_table[fd].open_offset,len);
    file_table[fd].open_offset+=read_bytes;
    return read_bytes;
  }else if(fd==FD_EVENTS){
    return events_read(buf,len);
  }
  size_t file_sz    = file_table[fd].size;
  off_t offset      = file_table[fd].disk_offset;
  off_t open_offset = file_table[fd].open_offset;
  size_t read_len = ((file_sz-open_offset)>len?len:(file_sz-open_offset));
  ramdisk_read(buf,offset+open_offset,read_len);
  file_table[fd].open_offset += read_len;
  return read_len;
};

ssize_t fs_write(int fd, const void *buf, size_t len){
  assert(fd>=0&&fd<NR_FILES);
  if(fd==1||fd==2){
    return sys_write(fd,(char*)buf,len);
  }else if(fd==FD_FB){
    int write_bytes = fb_write(buf,file_table[fd].open_offset,len);
    file_table[fd].open_offset+=write_bytes;
    return write_bytes;
  }
  size_t file_sz    = file_table[fd].size;
  off_t offset      = file_table[fd].disk_offset;
  off_t open_offset = file_table[fd].open_offset;
  size_t write_len = ((file_sz-open_offset)>len?len:(file_sz-open_offset));
  ramdisk_write(buf,offset+open_offset,write_len);
  file_table[fd].open_offset += write_len;
  return write_len;
};
off_t fs_lseek(int fd, off_t offset, int whence){
  assert(fd>=0&&fd<NR_FILES);
  if(whence==SEEK_SET){
    file_table[fd].open_offset = offset;
  }else if(whence==SEEK_CUR){
    file_table[fd].open_offset += offset;
  }else if(whence==SEEK_END){
    file_table[fd].open_offset = file_table[fd].size + offset;
  }else{
    assert(0);
    return -1;
  }
  if(file_table[fd].open_offset > file_table[fd].size){
    file_table[fd].open_offset = file_table[fd].size;
  }
  return file_table[fd].open_offset;
};
int fs_close(int fd){
  return 0;
};



void init_fs() {
  // TODO: initialize the size of /dev/fb
  int vga_w = _screen.width;
  int vga_h = _screen.height;
  Log("%d*%d",vga_w,vga_h);
  file_table[FD_FB].size = vga_h*vga_w*4;
  

}
