#include "common.h"
#include "syscall.h"


uintptr_t sys_none(){
  return 1;
}

uintptr_t sys_exit(int status){
  _halt(status);
  panic("Not reach here!");
  return 1;
}

int mm_brk(uint32_t new_brk);

int fs_open(const char *pathname, int flags, int mode);
ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  switch (a[0]) {
    case SYS_none:
      SYSCALL_ARG1(r) = sys_none();
      break;
    case SYS_open:
      SYSCALL_ARG1(r) = fs_open((char*)SYSCALL_ARG2(r),SYSCALL_ARG3(r),SYSCALL_ARG4(r));
      break;
    case SYS_read:
      SYSCALL_ARG1(r) = fs_read(SYSCALL_ARG2(r),(void*)SYSCALL_ARG3(r),SYSCALL_ARG4(r));
      break;
    case SYS_write:
      SYSCALL_ARG1(r) = fs_write(SYSCALL_ARG2(r),(void*)SYSCALL_ARG3(r),SYSCALL_ARG4(r));
      break;
    case SYS_exit:
      SYSCALL_ARG1(r) = sys_exit(SYSCALL_ARG2(r));
      break;
    case SYS_close:
      SYSCALL_ARG1(r) = fs_close(SYSCALL_ARG2(r));
      break;
    case SYS_lseek:
      SYSCALL_ARG1(r) = fs_lseek(SYSCALL_ARG2(r),SYSCALL_ARG3(r),SYSCALL_ARG4(r));
      break;
    case SYS_brk:
      SYSCALL_ARG1(r) = mm_brk(SYSCALL_ARG2(r));
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  return r;
}

