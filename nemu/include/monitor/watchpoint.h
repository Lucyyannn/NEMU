#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  char* expression;

} WP;

void print_wp_pool_info();
WP* find_wp(char* e);

WP* new_wp();
void free_wp(WP* wp);

#endif
