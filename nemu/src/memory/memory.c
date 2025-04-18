#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)
#define PAGE_SIZE 4096
#define PTE_LEN 4
#define PGSHFT    12      // log2(PGSIZE)
#define PTXSHFT   12      // Offset of PTX in a linear address
#define PDXSHFT   22      // Offset of PDX in a linear address
#define PDX(va)     (((uint32_t)(va) >> PDXSHFT) & 0x3ff)
#define PTX(va)     (((uint32_t)(va) >> PTXSHFT) & 0x3ff)
#define OFF(va)     ((uint32_t)(va) & 0xfff)
#define PTE_ADDR(pte)   ((uint32_t)(pte) & ~0xfff)
#define PTE_P     0x001     // Present
#define PTE_A     0x020     // Accessed
#define PTE_D     0x040     // Dirty


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

paddr_t page_translate(vaddr_t vaddr,bool write){
  /*  P mode */
  if (!cpu.PG){
    return (paddr_t)vaddr;
  }
  /* V mode */
  // level 1
  //Log("[in page_translate] cr3: %08X, vaddr:%08X",cpu.cr3,vaddr);
  paddr_t PDE_addr = (paddr_t)(cpu.cr3 + PTE_LEN*PDX(vaddr));
  uint32_t PDE_read = paddr_read(PDE_addr,PTE_LEN);
  //Log("[in page_translate] PDE_addr: %08X, PDE_read:%08X",PDE_addr,PDE_read);
  assert(PDE_read&PTE_P);// assert the PTE exists

  paddr_write(PDE_addr,PTE_LEN,(PDE_read|PTE_A));//set Accessed

  //level 2
  paddr_t PTE_addr = PTE_ADDR(PDE_read) + PTE_LEN*PTX(vaddr);
  uint32_t PTE_read = paddr_read(PTE_addr,PTE_LEN);
  assert(PTE_read&PTE_P);

  uint8_t DA_bits = write?(PTE_A|PTE_D):(PTE_A);
  paddr_write(PTE_addr,PTE_LEN,(PTE_read|DA_bits));//set Accessed/Dirty
  
  //to paddr
  return (paddr_t)(PTE_ADDR(PTE_read)+OFF(vaddr));
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
        int first_len = PAGE_SIZE-(addr%PAGE_SIZE);
        int second_len = len-first_len;
        uint32_t readdata=0;
        paddr_t paddr1 = page_translate(addr,false);
        readdata = paddr_read(paddr1,first_len);//lower bits
        paddr_t paddr2 = page_translate(addr+first_len,false);
        readdata |= (paddr_read(paddr2,second_len)<<(first_len*8));//higher bits
        return readdata;
  }//data in one page
  else {
        paddr_t paddr = page_translate(addr,false);
        return paddr_read(paddr, len);
  }
}
// xiaoduan 


void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  int rest_size = PAGE_SIZE-1 - addr%(PAGE_SIZE-1);
  if (rest_size<len) {
        int first_len = PAGE_SIZE-(addr%PAGE_SIZE);
        int second_len = len-first_len;
        assert(second_len>0&&first_len>0);
        uint32_t data1 = (data<<(second_len*8))>>(second_len*8);
        uint32_t data2 = data>>(first_len*8);
        Log("data:%08X , data1:%08X , data2:%08X , first_len:%d ,len:%d",data,data1,data2,first_len,len);
        paddr_t paddr1 = page_translate(addr,true);
        paddr_write(paddr1,first_len,data1);
        paddr_t paddr2 = page_translate(addr+first_len,true);
        paddr_write(paddr2,second_len,data2);
        return ;
  }else {
        paddr_t paddr = page_translate(addr,true);
        paddr_write(paddr, len, data);
        return ;
  }
}
