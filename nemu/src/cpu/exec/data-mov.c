// #include "cpu/exec.h"

// make_EHelper(mov) {
//   operand_write(id_dest, &id_src->val);
//   print_asm_template2(mov);
// }

// make_EHelper(mov_cr){
//   if(id_src->reg==0){
//     operand_write(id_dest,&cpu.cr0);
//   }else if(id_src->reg==3){
//     operand_write(id_dest,&cpu.cr3);
//   }
//   print_asm_template2(mov);
// }

// make_EHelper(mov_rc){
//   if(id_dest->reg==0){
//     cpu.cr0=id_src->val;
//   }else if(id_dest->reg==3){
//     cpu.cr3=id_src->val;
//   }
//   print_asm_template2(mov);
// }


// //push imm8指令需要对立即数进行符号扩展
// make_EHelper(push) {

//   if(id_dest->width==1&&id_dest->type==OP_TYPE_IMM){
//     rtl_sext(&t0,&id_dest->val,id_dest->width);
//     rtl_push(&t0);
//   }else{
//     rtl_push(&id_dest->val);
//   }
  
//   print_asm_template1(push);
// }

// make_EHelper(pop) {
//   rtl_pop(&t0);
//   operand_write(id_dest,&t0);

//   print_asm_template1(pop);
// }

// make_EHelper(pusha) {

//   rtlreg_t temp = cpu.esp;
//   rtl_push(&cpu.eax);
//   rtl_push(&cpu.ecx);
//   rtl_push(&cpu.edx);
//   rtl_push(&cpu.ebx);
//   rtl_push(&temp);
//   rtl_push(&cpu.ebp);
//   rtl_push(&cpu.esi);
//   rtl_push(&cpu.edi);
  
//   print_asm("pusha");
// }

// make_EHelper(popa) {
//   rtlreg_t temp = cpu.esp;
//   rtl_pop(&cpu.edi);
//   rtl_pop(&cpu.esi);
//   rtl_pop(&cpu.ebp);
//   rtl_pop(&temp);
//   rtl_pop(&cpu.ebx);
//   rtl_pop(&cpu.edx);
//   rtl_pop(&cpu.ecx);
//   rtl_pop(&cpu.eax);
  

//   print_asm("popa");
// }

// make_EHelper(leave) {
//   cpu.esp = cpu.ebp;//esp <- ebp
//   rtl_pop(&t0);
//   cpu.ebp=t0;       //ebp <- pop()

//   print_asm("leave");
// }

// make_EHelper(cltd) {
//   if (decoding.is_operand_size_16) {
//     if((int)reg_w(0)<0){
//       cpu.gpr[2]._16 = 0xFFFF;
//     }else{
//       cpu.gpr[2]._16 = 0;
//     }
//   }
//   else {
//     if((int)reg_l(0)<0){
//       cpu.edx=0xFFFFFFFF;
//     }else{
//       cpu.edx=0;
//     }
//   }

//   print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
// }

// make_EHelper(cwtl) {
//   if (decoding.is_operand_size_16) {
//     TODO();
//   }
//   else {
//     if((int)reg_w(0)<0){
//       cpu.gpr[2]._16 = 0xFFFF;
//     }else{
//       cpu.gpr[2]._16 = 0;
//     }
//   }

//   print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
// }

// make_EHelper(movsx) {
//   id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
//   rtl_sext(&t2, &id_src->val, id_src->width);

//   operand_write(id_dest, &t2);
//   print_asm_template2(movsx);
// }

// make_EHelper(movzx) {
//   id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
//   operand_write(id_dest, &id_src->val);
//   print_asm_template2(movzx);
// }

// make_EHelper(lea) {
//   rtl_li(&t2, id_src->addr);
//   operand_write(id_dest, &t2);
//   print_asm_template2(lea);
// }

#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}
make_EHelper(mov_cr) {
  if(id_src->reg==0){
    operand_write(id_dest, &cpu.cr0);
  }else if(id_src->reg==3){
    assert(0);
  }else{
    assert(0);
  }
  print_asm_template2(movcr);
}
make_EHelper(mov_rc) {
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
