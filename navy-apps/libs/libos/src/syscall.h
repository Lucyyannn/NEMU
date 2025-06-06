#ifndef __SYSCALL_H__
#define __SYSCALL_H__


enum {
  SYS_none,//0
  SYS_open,//1
  SYS_read,//2
  SYS_write,//3
  SYS_exit,//4
  SYS_kill,
  SYS_getpid,
  SYS_close,//7
  SYS_lseek,//8
  SYS_brk,//9
  SYS_fstat,
  SYS_time,
  SYS_signal,
  SYS_execve,
  SYS_fork,
  SYS_link,
  SYS_unlink,
  SYS_wait,
  SYS_times,
  SYS_gettimeofday
};


#endif
