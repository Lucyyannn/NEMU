#include <x86.h>
#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;//dir 1024pt
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;//pt all
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

  PTE *ptab = kptabs;//every pte
  for (i = 0; i < NR_KSEG_MAP; i ++) {
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
  set_cr0(get_cr0() | CR0_PG);//start page!
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
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

void* _map(_Protect *p, void *va,int* cnt) {
  PDE *dir = (PDE*)(p->ptr);//get the PDE 
  dir += PDX(va);
  PTE* uppte=NULL; 
  if(*dir&PTE_P){ 
    uppte = (PTE*)PTE_ADDR(*dir);
  }else{//not exist!
    uppte = (PTE*)(palloc_f());
    *dir = ((uint32_t)uppte&(~0xfff))|PTE_P;
    *cnt+=1;
  }
  uppte += PTX(va);
  void* pa=NULL;
  if(*uppte&PTE_P){
    pa = (void*)PTE_ADDR(*uppte);
  }else{//not exist!
    pa = (void*)(palloc_f());
    *uppte = ((uint32_t)pa&(~0xfff))|PTE_P;
    *cnt+=1;
  }
  pa += OFF(va);
  return pa;
}

void _unmap(_Protect *p, void *va) {
}

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  *((char**)(ustack.end -4))=NULL;
  *((char**)(ustack.end -8))=NULL;
  *((int*)(ustack.end -12))=0;
  _RegSet* rs = (_RegSet*)(ustack.end - 12 - sizeof(_RegSet));
  rs->cs = 0x8;
  rs->eip = (uintptr_t)entry;
  rs->eflags = (1<<9)+2;
  return rs;
}
