#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();

make_EHelper(lidt) {
  printf("limit: %d \n",cpu.idtr.limit);
  printf("base: %d \n",cpu.idtr.base);
  if(id_dest->width==2){
    cpu.idtr.limit = vaddr_read(id_dest->val,2);
    cpu.idtr.base  = vaddr_read(id_dest->val+2,3);
  }else if(id_dest->width==4){
    cpu.idtr.limit = vaddr_read(id_dest->val,2);
    printf("2~4: %x \n",vaddr_read(id_dest->val,4));
    printf("4~6: %x \n",vaddr_read(id_dest->val+4,2));
    printf("4~6: %x \n",vaddr_read(id_dest->val+8,2));
    uint32_t high = vaddr_read(id_dest->val+4,2);
    uint32_t low = vaddr_read(id_dest->val+2,2);
    cpu.idtr.base = (high <<16)|low;
    //cpu.idtr.base  = vaddr_read(id_dest->val+2,4);
  }
  printf("after:\n");
  printf("limit: %x \n",cpu.idtr.limit);
  printf("base: %x \n",cpu.idtr.base);
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  raise_intr(id_dest->val,decoding.seq_eip);

  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  TODO();

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);


make_EHelper(in) {
  t0 = pio_read(id_src->val,id_src->width);
  operand_write(id_dest,&t0);

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  pio_write(id_dest->val,id_src->width,id_src->val);

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
