#include "nemu.h"
#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
 int flag = is_mmio(addr);
 if(flag==-1){
   return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
 }
  return mmio_read(addr,len,flag);
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int flag = is_mmio(addr);
  if(flag==-1){
    memcpy(guest_to_host(addr), &data, len);
  }else{
    mmio_write(addr,len,data,flag);
  }
}
//static int cnt =0;
paddr_t page_translate(vaddr_t va,int write){
  if(cpu.cr0>>31){
    paddr_t dir_base = PTE_ADDR(cpu.cr3);
    
    uint32_t dir_idx = PDX(va);
    paddr_t pde_addr = dir_base + dir_idx * 4;
    uint32_t pde = paddr_read(pde_addr, 4);
    assert(pde&PTE_P);
    if(!(pde&PTE_A)){
      pde |= PTE_A;
      paddr_write(pde_addr, 4, pde);
    }

    paddr_t pt_base = PTE_ADDR(pde);
    uint32_t pt_idx = PTX(va);
    paddr_t pte_addr = pt_base + pt_idx*4;
    uint32_t pte = paddr_read(pte_addr, 4);
    // if(!(pte&PTE_P)){
    //   printf("va:%x,w/r:%x\n",va,write);
    //   printf("dir_idx:%x,pde:%x\n",dir_idx,pde);
    //   printf("pt_base:%x,pt_idx:%x,pte:%x\n",pt_base,pt_idx,pte);
    // }
    assert(pte&PTE_P);

    if(!(pte&PTE_A)){
      pte |= PTE_A;
      paddr_write(pte_addr, 4, pte);
    }
    if(write&&!(pte&PTE_D)){
      pte |= PTE_D;
      paddr_write(pte_addr, 4, pte);
    }

    paddr_t page_base = PTE_ADDR(pte);
    uint32_t page_off = OFF(va);
    paddr_t data_addr = page_base + page_off;
    // if(cnt++<10){
    //   printf("The va:%x|pd:%x|pde:%x|pt:%x|pte:%x|pa:%x\n",va,dir_base,pde,pt_base,pte,data_addr);
    // }
    return data_addr;
  }else{
    return va;
  }
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if(len > 4){
    len = 4;
  }
  if(0){
    assert(0);
    return 0;
  }else{
    paddr_t paddr = page_translate(addr,0);
    int r_len = 4096-(OFF(paddr));
    if(r_len < len){
      vaddr_t la_va = PGROUNDUP(addr);
      paddr_t la_pa = page_translate(la_va,0);
      uint32_t lo_data = paddr_read(paddr, r_len);
      uint32_t hi_data = paddr_read(la_pa, len - r_len);
      uint32_t data = (hi_data << (r_len*8))+ lo_data;
      return data;
    }else{
      return paddr_read(paddr, len);
    }
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(len > 4){
    len = 4;
  }
  if(0){
    assert(0);
  }else{
    paddr_t paddr = page_translate(addr,1);
    int r_len = 4096-(OFF(paddr));
    if(r_len < len){
      vaddr_t la_va = PGROUNDUP(addr);
      paddr_t la_pa = page_translate(la_va,1);
      paddr_write(paddr, r_len, data);
      uint32_t hi_data = data >> (r_len*8);
      paddr_write(la_pa, len - r_len, hi_data);
    }else{
      paddr_write(paddr, len, data);
    }
  }
}
