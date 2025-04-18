#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(mov_cr){
  if(id_src->reg==0){
    operand_write(id_dest,&cpu.cr0);
  }else if(id_src->reg==3){
    operand_write(id_dest,&cpu.cr3);
  }
  print_asm_template2(movrc);
}

make_EHelper(mov_rc){
  if(id_dest->reg==0){
    cpu.cr0=id_src->val;
  }else if(id_src->reg==3){
    Log("the value to set cr3: %08X ",id_src->val);
    cpu.cr3=id_src->val;
  }
  print_asm_template2(movcr);
}


//push imm8指令需要对立即数进行符号扩展
make_EHelper(push) {

  if(id_dest->width==1&&id_dest->type==OP_TYPE_IMM){
    rtl_sext(&t0,&id_dest->val,id_dest->width);
    rtl_push(&t0);
  }else{
    rtl_push(&id_dest->val);
  }
  
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t0);
  operand_write(id_dest,&t0);

  print_asm_template1(pop);
}

make_EHelper(pusha) {

  rtlreg_t temp = cpu.esp;
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&temp);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);
  
  print_asm("pusha");
}

make_EHelper(popa) {
  rtlreg_t temp = cpu.esp;
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&temp);
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);
  

  print_asm("popa");
}

make_EHelper(leave) {
  cpu.esp = cpu.ebp;//esp <- ebp
  rtl_pop(&t0);
  cpu.ebp=t0;       //ebp <- pop()

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    if((int)reg_w(0)<0){
      cpu.gpr[2]._16 = 0xFFFF;
    }else{
      cpu.gpr[2]._16 = 0;
    }
  }
  else {
    if((int)reg_l(0)<0){
      cpu.edx=0xFFFFFFFF;
    }else{
      cpu.edx=0;
    }
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    TODO();
  }
  else {
    if((int)reg_w(0)<0){
      cpu.gpr[2]._16 = 0xFFFF;
    }else{
      cpu.gpr[2]._16 = 0;
    }
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
