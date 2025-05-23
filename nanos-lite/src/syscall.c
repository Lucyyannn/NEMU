#include "common.h"
#include "syscall.h"
#include "fs.h"

int mm_brk(uint32_t new_brk) ;
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
ssize_t fs_read(int fd, void *buf, size_t len);
int fs_open(const char *pathname, int flags, int mode);
int fs_close(int fd);

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

    case 1:{//SYS_open
      r->eax = fs_open((const char*)a[1],(int)a[2],(int)a[3]);
      break;
    }

    case 2:{//SYS_read
      r->eax = fs_read((int)a[1],(char*)a[2],(ssize_t)a[3]);
      break;
    }

    case 3:{//SYS_write
      r->eax = fs_write((int)a[1],(const void*)a[2],(ssize_t)a[3]);
      break;
    }

    case 4:{//SYS_exit
      _halt(a[1]); 
      r->eax = 1;
      break;
    }

    case 7:{//SYS_close
      r->eax = fs_close((int)a[1]);
      break;

    }
    case 8:{//SYS_lseek
      r->eax = fs_lseek((int)a[1],(off_t)a[2],(int)a[3]);
      break;
    }

    case 9:{//SYS_brk
      r->eax = mm_brk(a[1]);//always return 0
      break;
    }

    default: 
      panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
