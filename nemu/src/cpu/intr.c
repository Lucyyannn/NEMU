#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  
  t2 = cpu.cs;
  rtl_push(&cpu.eflags);
  rtl_push(&t2);
  rtl_push(&ret_addr);
  vaddr_t idx = cpu.idtr_base+ NO*8;
  t2 = vaddr_read(idx,4);
  t3 = vaddr_read(idx+4,4);
  decoding.jmp_eip =(t3&0xffff0000) + (t2&0x0000ffff);
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
}
