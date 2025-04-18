#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)
#define PAGE_SIZE 4096
#define VADDR_DIR_OFFSET (32-10)
#define VADDR_OFFSET_OFFSET (32-10)
#define PTE_LEN 4

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];
/*  VADDR:

   31----------22 21----------12 11----------0
      DIR(10)         PAGE(10)      OFFSET(12)

    PADDR:

    31------------------------12 11----6 5---0
            address(20)                D A   P
 */
/*
 - set Accessed:
   0001 0000  -->16
 - set Dirty(already Accessed):
   0011 0000  -->48

 */

// paddr_t page_translate(vaddr_t vaddr,bool write){
//   // level 1
//   paddr_t PDE_addr = cpu.cr3 + (vaddr>> VADDR_DIR_OFFSET);
//   uint32_t PDE_read = paddr_read(PDE_addr,PTE_LEN);
//   assert(PTE_read&1);
//   paddr_t PTE_addr_base = (paddr_t)(PTE_read>>12);
//   paddr_write(PDE_addr,1,16);//set Accessed
//   //level 2
//   paddr_t PTE_addr = PTE_addr_base + ((vaddr<<10)>>(32-10));
//   uint32_t PTE_read = paddr_read(PTE_addr,PTE_LEN);
//   assert(PTE_read&1);
//   paddr_t PADDR_addr_base = (paddr_t)(PTE_read>>12);
//   uint8_t DA_bits = write?48:16;
//   paddr_write(PTE_addr,1,DA_bits);//set Accessed/Dirty
//   //to paddr
//   return (PADDR_addr_base+((vaddr<<VADDR_OFFSET_OFFSET)>>VADDR_OFFSET_OFFSET));
// }

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int num = is_mmio(addr);
  if(num!=-1){
    return mmio_read(addr,len,num);
  }else{
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int num = is_mmio(addr);
  if(num!=-1){
    mmio_write(addr,len,data,num);
    return;
  }else{
    memcpy(guest_to_host(addr), &data, len);
    return ;
  }
}

uint32_t vaddr_read(vaddr_t addr, int len) {

  return paddr_read(addr, len);
  
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {

    paddr_write(addr, len, data);
    return ;
  
}
