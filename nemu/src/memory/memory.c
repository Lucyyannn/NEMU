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
/*
   31----------22 21----------12 11----------0
      DIR(10)         PAGE(10)      OFFSET(12)
 */

paddr_t page_translate(vaddr_t vaddr){
  // level 1
  paddr_t PDE_addr = cpu.cr3 + (vaddr>> VADDR_DIR_OFFSET);
  paddr_t PTE_addr_base = (paddr_t)paddr_read(PDE_addr,PTE_LEN);
  //level 2
  paddr_t PTE_addr = PTE_addr_base + ((vaddr<<10)>>(32-10));
  paddr_t PADDR_addr_base = (paddr_t)paddr_read(PTE_addr,PTE_LEN);
  //to paddr
  return (PADDR_addr_base+((vaddr<<VADDR_OFFSET_OFFSET)>>VADDR_OFFSET_OFFSET));
}

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
  //data cross page
  if ((addr/PAGE_SIZE)!=((addr+len)/PAGE_SIZE)) {
        assert(0);
        // int first_len = PAGE_SIZE-(addr%PAGE_SIZE);
        // int second_len = len-fist_len;
        // uint32_t readdata=0;
        // paddr_t paddr1 = page_translate(addr);
        // readdata = (paddr_read(paddr1,first_len)<<second_len);
        // paddr_t paddr2 = page_translate(addr+first_len);
        // readdata |= paddr_read(paddr2,second_len);
        // return readdata;
  }//data in one page
  else {
        paddr_t paddr = page_translate(addr);
        return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if ((addr/PAGE_SIZE)!=((addr+len)/PAGE_SIZE)) {
        assert(0);
        // int first_len = PAGE_SIZE-(addr%PAGE_SIZE);
        // int second_len = len-first_len;
        // uint32_t data1 = data;
        // uint32_t data2 = data<<first_len;
        // paddr_t paddr1 = page_translate(addr1);
        // paddr_write(paddr1,first_len,data1);
        // paddr_t paddr2 = page_translate(addr1+first_len);
        // paddr_write(paddr2,second_len,data2);
        // return ;
  }else {
        paddr_t paddr = page_translate(addr);
        paddr_write(paddr, len, data);
        return ;
  }
}
