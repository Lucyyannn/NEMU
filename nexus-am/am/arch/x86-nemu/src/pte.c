#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))
#define PAGE_SIZE 4096
#define PTE_LEN 4

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {//PGSIZE*NR_PTE=memory size of all PTs of each PD
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}
  

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());//alloc a Ppage for PDE

  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) { 
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void _map(_Protect *p, void *va, void *pa) {
  //the value of va and pa is already difinited. What I should do is to set the PDE and PTE to support the bridge
  PDE* pde = (PDE*)p->ptr + PDX(va);
  // (1)if PTE not exists
  if(!(*pde&PTE_P)){
    PTE* ptepage = (PTE*)palloc_f();//alloc a page for PTE
    *pde = PTE_ADDR((uintptr_t)ptepage)| PTE_P;// set the PTE value in PDE

    PTE* pte = ptepage + PTX(va);
    *pte = PTE_ADDR(pa) | PTE_P; //update PTE
    return ;

  }else{//(2)PTE exists
    PTE* pte = (PTE*)PTE_ADDR(*pde)+PTX(va);
    *pte = PTE_ADDR(pa) | PTE_P; //update PTE
    return ;
  }
  return ;
}
// void _map(_Protect *p, void *va, void *pa) {
//   PDE *pde = ((PDE *)p->ptr) + PDX(va);
//   PTE *ptab;
//   if ((*pde & PTE_P) == 0) {
//     ptab = (PTE *)(palloc_f());
//     *pde = ((uint32_t)ptab & ~0xfff) | PTE_P;   
//   }
//   else 
//     ptab = (PTE *)PTE_ADDR(*pde);
//   ptab[PTX(va)] = ((uint32_t)pa & ~0xfff) | PTE_P;
// }

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, 
              void *entry, char *const argv[], char *const envp[]) {
                /*
                 stack frame : high address
                                              return value
                                              argv n
                                              argv n-1
                                              ...
                                              argv 1
                               low            old ebp
                 */
  //(1) set the stack of _start
  uintptr_t StartStack = (uintptr_t)ustack.end;
  *(uintptr_t*)StartStack=0; //return value
  *((char**)(--StartStack)) = NULL;//arguments
  *((char**)(--StartStack))=NULL;
  *((int*)(--StartStack))=0;
  
  //(2) init trapframe
  //_RegSet* tf = (_RegSet*)(StartStack - TF_SPACE/sizeof(int));
  //_RegSet* tf = (_RegSet*)(ustack.end-sizeof(uintptr_t)*4-TF_SPACE);
  _RegSet* tf = (_RegSet*)(StartStack-TF_SPACE);
  tf->edi=0;
  tf->esi=0;
  tf->ebp=0;
  tf->esp=0;
  tf->ebx=0;
  tf->edx=0;
  tf->ecx=0;
  tf->eax=0;
  tf->irq=0;
  tf->error_code=0;
  tf->eip=(uintptr_t)entry;
  tf->cs=8;
  tf->eflags=0x0202;// enable time intr
//  0000 0010 0000 0010
  return tf;
}
