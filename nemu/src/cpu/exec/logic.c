#include "cpu/exec.h"

// similar to and, but only eflags impacted
make_EHelper(test) {
  rtl_and(&t2,&id_dest->val,&id_src->val);

  rtl_update_ZFSF(&t2,id_dest->width);
  t0 = 0;
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);

  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);

  t0 = 0;
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);
  rtl_update_ZFSF(&t2,id_dest->width);

  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);

  t0 = 0;
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);
  rtl_update_ZFSF(&t2,id_dest->width);

  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);

  t0 = 0;
  rtl_set_CF(&t0);
  rtl_set_OF(&t0);
  rtl_update_ZFSF(&t2,id_dest->width);

  print_asm_template2(or);
}

// unnecessary to update CF and OF in NEMU
make_EHelper(sar) {
  int count = id_src->val;
  t2 = id_dest->val;
  rtl_msb(&t1,&id_dest->val,id_dest->width);//t1 stores the sign
  int sign_mask = (t1<<(id_dest->width*8-1));
  while(count!=0){
    t2 = (t2>>1);
    t2 = t2 | sign_mask;
    --count;
  }
  operand_write(id_dest,&t2);
  rtl_update_ZFSF(&t2,id_dest->width);

  print_asm_template2(sar);
}

// unnecessary to update CF and OF in NEMU
make_EHelper(shl) {
  int count = id_src->val;
  t2 = id_dest->val;
  while(count!=0){
    t2 = (t2<<1);
    --count;
  }
  operand_write(id_dest,&t2);
  rtl_update_ZFSF(&t2,id_dest->width);
  

  print_asm_template2(shl);
}

// unnecessary to update CF and OF in NEMU
make_EHelper(shr) {
  int count = id_src->val;
  t2 = id_dest->val;
  while(count!=0){
    t2 = (t2>>1);
    --count;
  }
  operand_write(id_dest,&t2);
  rtl_update_ZFSF(&t2,id_dest->width);
  

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  t0 = ~(id_dest->val);
  operand_write(id_dest,&t0);
  
  print_asm_template1(not);
}
