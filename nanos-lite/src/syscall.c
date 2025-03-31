#include "common.h"
#include "syscall.h"

ssize_t fs_write(int fd, const void *buf, size_t len);

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
  a[2] = SYSCALL_ARG3(r);
  a[3] = SYSCALL_ARG4(r);
  switch (a[0]) {
    case 0://SYS_none
      SYSCALL_ARG1(r) = 1;
      break;
    case 3:{//SYS_write
      int fd       = (int)a[1];
      //char *buf    = (void*)a[2];
      size_t count = (size_t)a[3];

      if(fd==1||fd==2){
        //r->eax = fs_write(fd, buf,count);
        //_putc(*buf);
        // for(int i=0;i<count;i++){
        //   _putc(*(buf+i));
        // }
      //return value: stdout(1):count; stderr(2):-1
        if(fd ==1){
          SYSCALL_ARG1(r) = count;
        }else{
          SYSCALL_ARG1(r) = -1;
        }
      }
      break;
    }
    case 4:{//SYS_exit
      _halt(a[1]); 
      SYSCALL_ARG1(r) = 1;
      break;
    }
    default: 
      panic("Unhandled syscall ID = %d", a[0]);
  }

  return r;
}
