#include "common.h"
#include "syscall.h"

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
    case 3://SYS_write
    // a[1]:fd     a[2]:buf    a[3]:count
      if(a[1]==1||a[1]==2){
        for(int i=0;i<a[3];i++){
          _putc(a[2]+i);
        }
    //return value: stdout(1):count; stderr(2):-1
        if(a[1]==1){
          r->eax = a[3];
        }else{
          r->eax = -1;
        }
      }
      break;
    case 4://SYS_exit
      _halt(r->eax); 
      r->eax = 1;
      break;
    default: 
      panic("Unhandled syscall ID = %d", a[0]);
  }

  return r;
}
