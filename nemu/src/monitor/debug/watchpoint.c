#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

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

// add a new node to the list, at the beginning
void add_to_list(WP* list,WP* newnode){
  if(list==NULL){
    list=newnode;
    list->next=NULL;
    return;
  }
  newnode->next=list;
  list=newnode;
  return;
}

// release a node from free_ at the beginning
WP* new_wp(){
  if(free_!=NULL){
    WP* des=free_;
    free_=free_->next;
    return des;
  }
  assert(0);
}
// wp-->free_
void free_wp(WP* wp){
  WP* temp = head;
  // if wp is the head
  if(temp->NO==wp->NO){
    head=head->next;// release
    add_to_list(free_,temp);//add
    return;
  }
  //else
  WP* pre = head;
  temp=temp->next;
  while(temp!=NULL){
    if(temp->NO==wp->NO){
      pre->next=temp->next;//release
      add_to_list(free_,temp);//add
      return ;
    }
    // ++ 
    pre=temp;
    temp=temp->next;
  }
  panic("the wp to free cannot be found!");
  return ;
}

//print the information of all the watchpoints, to debug
void print_wp_pool_info(){
  printf("watchpoint information:\n");
  for (int i=0;i<NR_WP;i++){
    printf("%d\n", wp_pool[i].NO);
  }
  return ;
}



