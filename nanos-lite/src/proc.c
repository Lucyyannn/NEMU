#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  // TODO: remove the following three lines after you have implemented _umake()
  // _switch(&pcb[i].as);
  // current = &pcb[i];
  // ((void (*)(void))entry)();

  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
  Log("pcb[%d]: cr3: %08X, tf: %08X",i,(uintptr_t)pcb[i].as.ptr,(uintptr_t)pcb[i].tf);
}

int current_game = 0;
static int ratio = 0;

_RegSet* schedule(_RegSet *prev) {
  // save the context pointer
  current->tf = prev;

  if(current==&pcb[current_game]){
    if(ratio==99){
      current=&pcb[1];
    }
    ratio=(ratio+1)%100;
  }else if(current==&pcb[1]){
    current=&pcb[current_game];
  }else{
    assert(0);
  }
  _switch(&current->as);
  return (current->tf);
}
