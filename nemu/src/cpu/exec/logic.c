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
  rtl_sar(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);
  
  rtl_update_ZFSF(&t2,id_dest->width);

  print_asm_template2(sar);
}

// unnecessary to update CF and OF in NEMU
make_EHelper(shl) {
  rtl_shl(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t2);

  rtl_update_ZFSF(&t2,id_dest->width);
  
  print_asm_template2(shl);
}

// unnecessary to update CF and OF in NEMU
make_EHelper(shr) {

  rtl_shr(&t2,&id_dest->val,&id_src->val);
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

make_EHelper(rol){
  int count=id_src->val; //count
  uint32_t value = id_dest->val;
  uint32_t hbit=0;
  while(count!=0){
    hbit = (value>>(id_dest->width *8-1))&1;
    value = (value<<1) +hbit;
    --count;
  }
  operand_write(id_dest,&value);
  hbit= (value>>(id_dest->width *8-1))&1;
  //OF
  if(id_src->val==1){
    rtl_get_CF(&t0);
    t1 = (hbit!=t0)?1:0;
    rtl_set_OF(&t1);
  }
  //CF
  rtl_get_CF(&t0);
  rtl_xor(&t2,&hbit,&t0);
  rtl_set_CF(&t2);


  print_asm_template2(rol);
}

make_EHelper(ror){
  TODO();
  print_asm_template2(ror);
}

make_EHelper(rcl){
  TODO();
  print_asm_template2(rcl);
}

make_EHelper(rcr){
  TODO();
  print_asm_template2(rcr);
}