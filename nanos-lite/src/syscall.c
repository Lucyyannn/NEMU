#include "common.h"
#include "syscall.h"

int do_SYSnone(){
  return 1;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);

  switch (a[0]) {
    case 0://SYS_none
      do_SYSnone();
    case 4://SYS_exit
      _halt(r->eax);
    default: 
      panic("Unhandled syscall ID = %d", a[0]);
  }

  return r;
}
