#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  uint32_t value;
  char * expr;

} WP;

WP* new_wp();
void free_wp(WP *wp);

extern WP wp_pool[NR_WP];  
extern WP *head;           
extern WP *free_;          

#endif
