#ifndef __MONITOR_H__
#define __MONITOR_H__

enum { NEMU_STOP, NEMU_RUNNING, NEMU_END };
extern int nemu_state;

char* w_tokens[32]; // from w_tokens[0] to w_tokens[31]
int w_nr;

#endif
