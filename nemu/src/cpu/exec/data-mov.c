#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}
make_EHelper(mov_c2r) {
  if(id_src->reg==0){
    operand_write(id_dest, &cpu.cr0);
  }else if(id_src->reg==3){
    assert(0);
  }else{
    assert(0);
  }
  print_asm_template2(movcr);
}
make_EHelper(mov_r2c) {
  if(id_dest->reg==0){
    cpu.cr0 = id_src->val;
  }else if(id_dest->reg==3){
    cpu.cr3 = id_src->val;
  }else{
    assert(0);
  }
  print_asm_template2(movrc);
}
make_EHelper(push) {
  
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  
  rtl_pop(&t2);
  operand_write(id_dest, &t2);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  assert(decoding.is_operand_size_16==0);
  rtl_lr(&t2,4,4);
  for(int i=0;i<8;i++){
    if(i!=4){
      rtl_lr(&t3,i,4);
      rtl_push(&t3);
    }else{
      rtl_push(&t2);
    }
  }
  print_asm("pusha");
}

make_EHelper(popa) {
  assert(decoding.is_operand_size_16==0);
  for(int i=7;i>=0;i--){
    if(i!=4){
      rtl_pop(&t2);
      rtl_sr(i,4,&t2);
    }else{
      rtl_pop(&t2);
    }
  }
  print_asm("popa");
}

make_EHelper(leave) {
  rtl_lr(&t2,5,4);
  rtl_sr(4,4,&t2);
  rtl_pop(&t2);
  rtl_sr(5,4,&t2);
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtl_lr(&t2,0,2);
    rtl_msb(&t3,&t2,2);
    t1=0;
    if(t3){
      rtl_not(&t1);
    }
    rtl_sr(2,2,&t1);
  }else {
    rtl_lr(&t2,0,4);
    rtl_msb(&t3,&t2,4);
    t1=0;
    if(t3){
      rtl_not(&t1);
    }
    rtl_sr(2,4,&t1);
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_lr(&t2,0,1);
    rtl_sext(&t3,&t2,1);
    rtl_sr(0,2,&t3);
  }
  else {
    rtl_lr(&t2,0,2);
    rtl_sext(&t3,&t2,2);
    rtl_sr(0,4,&t3);
  }
  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
