#include "monitor/watchpoint.h"
#include "monitor/expr.h"

WP wp_pool[NR_WP];
WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(){
  assert(free_!=NULL);
  WP* wp=free_;
  free_ = free_->next;
  wp->next = NULL;

  wp->next = head;
  head = wp;
  return head;
};
void free_wp(WP *wp){
  assert(head!=NULL);
  if(head->next==NULL){
    if(head->NO==wp->NO){
      head = NULL;
    }else{
      assert(0);
    }
  }else{
    if(head->NO==wp->NO){
      head = head->next;
    }else{
      WP* curr_pre = head;
      WP* curr = head->next;
      while(curr!=NULL){
        if(curr->NO==wp->NO){
          curr_pre->next = curr->next;
          break;
        }else{
          curr_pre = curr;
          curr = curr->next;
        }
      }
      assert(curr!=NULL);
    }
  }
  wp->next = free_;
  free_ = wp;
};

