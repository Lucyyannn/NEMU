#include "proc.h"
#include "memory.h"

static void *pf = NULL;
#define PAGE_SIZE 4096

void* new_page(void) {
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) {
  if (current->cur_brk == 0) {
    current->cur_brk = current->max_brk = new_brk;
  }
  else {
    if (new_brk > current->max_brk) {
      // map memory region [current->max_brk, new_brk)into address space current->as
      uintptr_t va_begin = PGROUNDUP(current->max_brk);
      uintptr_t va_end = PGROUNDDOWN(new_brk-1);
      Log("[mm_brk] max_brk: %08X, new_brk: %08X ,va_begin=%08X,va_end=%08X",current->max_brk,new_brk,va_begin,va_end);
      for(uintptr_t va = va_begin; va <= va_end; va += PAGE_SIZE){
        void* pa = new_page();
        _map(&current->as, (void*)va, pa);
        Log("reflect: va:%08X",(uintptr_t)va);
      }
      current->max_brk = new_brk;
    }
    current->cur_brk = new_brk;
  }

  return 0;
}


void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
}
