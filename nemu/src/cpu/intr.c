// #include "cpu/exec.h"
// #include "memory/mmu.h"

// #define HALF_GD 4

// void raise_intr(uint8_t NO, vaddr_t ret_addr) {
//   /* TODO: Trigger an interrupt/exception with ``NO''.
//    * That is, use ``NO'' to index the IDT.
//    */

//   if(NO>cpu.idtr_limit){
//     panic("the NO in raise_intr is beyond IDT's limit!");
//     return ;
//   }

//   //push eflags, cs,eip
//   t2 = cpu.cs;
//   rtl_push(&cpu.eflags);
//   rtl_push(&t2);
//   rtl_push(&ret_addr);

//   //read the base of IDT , index the idt to find the GD
//   uint32_t idt_index = cpu.idtr_base + NO*8;//8 bytes for each GD
//   uint32_t gd_l = vaddr_read(idt_index, HALF_GD);
//   uint32_t gd_h = vaddr_read(idt_index+HALF_GD,HALF_GD);

//   //combine the offset , get the destination
//   uint32_t destination = (gd_l&0x0000FFFF)|(gd_h&0xFFFF0000);
//   //jmp to the destination
//   decoding.is_jmp = true;
//   decoding.jmp_eip = destination;
//   return ;
// }

// void dev_raise_intr() {
// }
#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  
  t2 = cpu.cs;
  rtl_push(&cpu.eflags);
  cpu.IF = 0;
  rtl_push(&t2);
  rtl_push(&ret_addr);
  vaddr_t idx = cpu.idtr_base+ NO*8;
  t2 = vaddr_read(idx,4);
  t3 = vaddr_read(idx+4,4);
  decoding.jmp_eip =(t3&0xffff0000) + (t2&0x0000ffff);
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
  cpu.INTR = 1;
}
