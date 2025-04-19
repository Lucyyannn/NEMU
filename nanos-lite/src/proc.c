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

//static int ratio = 0;
_RegSet* schedule(_RegSet *prev) {
  // save the context pointer
  current->tf = prev;

  // always select pcb[0] as the new process
  // take 0 and 1 

 // Log(" a schedule!");
  current = &pcb[0];
  _switch(&current->as);

  //Log("pcb[0] cr3: %08X, tf: %08X",(uintptr_t)pcb[0].as.ptr,(uintptr_t)pcb[0].tf);
  //Log("pcb[1] cr3: %08X, tf: %08X",(uintptr_t)pcb[1].as.ptr,(uintptr_t)pcb[1].tf);

  return (current->tf);


  // TODO: switch to the new address space,
  // then return the new context
  // if(current==&pcb[0]){
  //   if(ratio==100000){ratio=0;}
  //   ++ratio;
  //   _switch(&pcb[0].as);
  //   return (pcb[0].tf);
  // }else if(current==&pcb[1]){
  //   _switch(&pcb[1].as);
  //   return (pcb[1].tf);
  // }


  // current =((current == &pcb[0]) ? &pcb[1] : &pcb[0]);

  // // TODO: switch to the new address space,
  // // then return the new context
  // if(current==&pcb[0]){
  //   _switch(&pcb[0].as);
  //   return (pcb[0].tf);
  // }else if(current==&pcb[1]){
  //   _switch(&pcb[1].as);
  //   return (pcb[1].tf);
  // }
  return NULL;
}
