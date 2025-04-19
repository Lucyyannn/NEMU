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

// /* The brk() system call handler. */
// int mm_brk(uint32_t new_brk) {
//   if (current->cur_brk == 0) {
//     current->cur_brk = current->max_brk = new_brk;
//   }
//   else {
//     if (new_brk > current->max_brk) {
//       // map memory region [current->max_brk, new_brk)into address space current->as
//       void *va_begin = (void *)((current->max_brk-1)&~0xfff) + PAGE_SIZE;
//       void *va_end = (void *)((new_brk-1)&~0xfff);
//       void *va = va_begin;
//       for(va = va_begin; va <= va_end; va += PAGE_SIZE){
//         void* pa = new_page();
//         _map(&current->as, va, pa);
//       }
//       current->max_brk = new_brk;
//     }
//     current->cur_brk = new_brk;
//   }

//   return 0;
// }

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) {
  if (current->cur_brk == 0) {
    current->cur_brk = current->max_brk = new_brk;
  }
  else {
    if (new_brk > current->max_brk) {
      // TODO: map memory region [current->max_brk, new_brk)
      // into address space current->as
      uint32_t align_left = PGROUNDDOWN(current->max_brk);
      uint32_t align_right = PGROUNDDOWN(new_brk-1);
      //Log("[%x,%x)--[%x,%x)",current->max_brk,new_brk,align_left,align_right);
      void* cur_va = (void*)align_left;
      void* cur_pa = NULL;
      for(;align_left<=align_right;align_left+=4096){
        int c=0;
        cur_pa = _map(&current->as,cur_va,&c);
        assert(cur_pa);
        cur_va += 4096;
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
