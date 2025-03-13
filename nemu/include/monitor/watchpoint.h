#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char* expression;

} WP;

void print_wp_pool_info();

WP* new_wp();
void free_wp(int number);

#endif
