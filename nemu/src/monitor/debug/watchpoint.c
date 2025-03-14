#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "monitor/monitor.h"

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
    WP* des=free_; // release from free_
    free_=free_->next;

    des->next=head;//add to head
    head=des;

    return des;
  }
  assert(0);
}
// wp-->free_
void free_wp(int number){
  WP* temp = head;
  // if wp is the head
  if(temp->NO==number){
    head=head->next;// release
    add_to_list(free_,temp);//add
    return;
  }
  //else
  WP* pre = head;
  temp=temp->next;
  while(temp!=NULL){
    if(temp->NO==number){
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
  printf("watchpoints: \n");
  if(w_nr==0){
    printf("No watchpoints.\n");
    return ;
  }
  for(int i=0;i<w_nr;++i){
    bool success=true;
    uint32_t value = expr(w_tokens[i],&success);
    printf("NO: %d , EXPRESSION: %s , value: %d ,",i,w_tokens[i],value);
    printf("hex: %08x \n",value);
  } 
  return ;
}



