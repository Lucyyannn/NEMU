#include "common.h"
#include "syscall.h"
#include "fs.h"

int mm_brk(uint32_t new_brk) ;
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
ssize_t fs_read(int fd, void *buf, size_t len);

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);
  switch (a[0]) {
    case 0://SYS_none
      r->eax = 1;
      break;

    case 2:{//SYS_read
      int fd       = (int)a[1];
      char *buf    = (char*)a[2];
      ssize_t count = (ssize_t)a[3];
      r->eax = fs_read(fd,buf,count);
      break;
    }

    case 3:{//SYS_write
      int fd       = (int)a[1];
      char *buf    = (char*)a[2];
      ssize_t count = (ssize_t)a[3];
      //r->eax = fs_write(fd,buf,count);
      //if(fd==2){ r->eax = -1;}
      if(fd==1||fd==2){
        Log("SYS_write.\n");
        for(int i=0;i<count;i++){
          _putc(*(buf+i));
          Log("%c" ,*(buf+i));
        }
      }
      r->eax = (fd==1)?count:-1;
      
      break;
    }

    case 4:{//SYS_exit
      _halt(a[1]); 
      r->eax = 1;
      break;
    }

    case 8:{//SYS_lseek
      int fd = (int)a[1];
      off_t offset = (off_t)a[2];
      int whence = (int)a[3];
      r->eax = fs_lseek(fd,offset,whence);
      break;
    }

    case 9:{//SYS_brk
      r->eax = mm_brk(a[1]);//always return 0
      if(r->eax != 0){
        panic("SYS_brk must return 0!\n");
      }
      break;
    }

    default: 
      panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
