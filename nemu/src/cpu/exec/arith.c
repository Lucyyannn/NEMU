#include "cpu/exec.h"

make_EHelper(add) {
  rtl_add(&t2, &id_dest->val, &id_src->val);//t2=dest+src
  rtl_sltu(&t3, &t2, &id_dest->val);//t3=1(t2<dest, ji carry), otherwise 0
  operand_write(id_dest, &t2);//WB
  
  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&t3);
  

  rtl_xor(&t0, &id_dest->val, &id_src->val);//requirement 1: have the same sign
  rtl_not(&t0);
  rtl_xor(&t1, &id_dest->val, &t2);//requirement 2: dest and sum have different sign
  rtl_and(&t0, &t0, &t1);//both 1 and 2 satisfied
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);
  
  print_asm_template2(add);
}

make_EHelper(sub) {
  rtl_sub(&t2, &id_dest->val, &id_src->val);//t2=dest-src
  rtl_sltu(&t3, &id_dest->val, &t2); //t3=1 if jiewei else 0
  operand_write(id_dest, &t2);//WB

  rtl_update_ZFSF(&t2, id_dest->width);
  rtl_set_CF(&t3);

  rtl_xor(&t0, &id_dest->val, &id_src->val);// msb(t0)=1 if dest and src have different sign (r1)
  rtl_xor(&t1, &id_dest->val, &t2);//msb(t1)=1 if dest and t2 have different sign (r2)
  rtl_and(&t0, &t0, &t1);//both 1 and 2 satisfied
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(sub);
}

make_EHelper(cmp) {
  rtl_sub(&t2,&id_dest->val,&id_src->val);// t2 stores the result
  
  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_sltu(&t3, &id_dest->val, &t2); //t3=1 if jiewei else 0
  rtl_set_CF(&t3);

  //OF: the first code is above
  rtl_xor(&t0, &id_dest->val, &id_src->val);// msb(t0)=1 if dest and src have different sign (r1)
  rtl_xor(&t1, &id_dest->val, &t2);//msb(t1)=1 if dest and t2 have different sign (r2)
  rtl_and(&t0, &t0, &t1);//both 1 and 2 satisfied
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);


  print_asm_template2(cmp);
}

make_EHelper(inc) {
  rtl_addi(&t2,&id_dest->val,1);
  rtl_sltu(&t3, &t2, &id_dest->val);
  operand_write(id_dest,&t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_set_CF(&t3);

  rtl_msb(&t0,&id_dest->val,id_dest->width);
  rtl_msb(&t1,&t0,id_dest->width);
  if(t0==0&&t1==1){
    rtl_set_OF(&t0);
  }

  print_asm_template1(inc);
}

make_EHelper(dec) {
  rtl_subi(&t2,&id_dest->val,1);
  rtl_sltu(&t3, &id_dest->val,&t2);
  operand_write(id_dest,&t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_set_CF(&t3);

  rtl_msb(&t0,&id_dest->val,id_dest->width);
  rtl_msb(&t1,&t0,id_dest->width);
  if(t0==1&&t1==0){
    rtl_set_OF(&t1);
  }

  print_asm_template1(dec);
}

make_EHelper(neg) {
  int rmval=id_dest->val;
  int negval = -rmval;
  t2 = negval;
  operand_write(id_dest,&t2);

  //set SF ZF
  rtl_update_ZFSF(&t2,id_dest->width);
  //set CF
  t0 = (rmval==0)?0:1;
  rtl_set_CF(&t0);
  //set OF  OF= (msb(result)==msb(val))
  rtl_msb(&t1,&t2,id_dest->width);
  rtl_msb(&t0,&id_dest->val,id_dest->width);
  t0 = (t1==t0);
  rtl_set_OF(&t0);
  

  print_asm_template1(neg);
}


// if t2<dest, carry
make_EHelper(adc) {//dest+src+CF
  rtl_add(&t2, &id_dest->val, &id_src->val);//t2=dest+src
  rtl_sltu(&t3, &t2, &id_dest->val);//t3=1(t2<dest, ji carry), otherwise 0
  rtl_get_CF(&t1);//t1(CF)
  rtl_add(&t2, &t2, &t1);//t2=t2+t1
  operand_write(id_dest, &t2);//WB
  
  rtl_update_ZFSF(&t2, id_dest->width);
  // then: t2=id_dest= dest+src+CF
  rtl_sltu(&t0, &t2, &id_dest->val);//t0=1 or 0 [0]  //now id_dest->val!=id_dest
  rtl_or(&t0, &t3, &t0);//t0 = t3|t0
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_not(&t0); // if dest and src have the same sign, the msb(t0)==1
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(adc);
}

make_EHelper(sbb) {
  rtl_sub(&t2, &id_dest->val, &id_src->val);//t2=dest-src
  rtl_sltu(&t3, &id_dest->val, &t2); //t3=1 if jiewei else 0
  rtl_get_CF(&t1);
  rtl_sub(&t2, &t2, &t1);
  operand_write(id_dest, &t2);//WB

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_sltu(&t0, &id_dest->val, &t2);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);// msb(t0)=1 if dest and src have different sign (r1)
  rtl_xor(&t1, &id_dest->val, &t2);//msb(t1)=1 if dest and t2 have different sign (r2)
  rtl_and(&t0, &t0, &t1);//both 1 and 2 satisfied
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(sbb);
}

make_EHelper(mul) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_mul(&t0, &t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr_w(R_AX, &t1);
      break;
    case 2:
      rtl_sr_w(R_AX, &t1);
      rtl_shri(&t1, &t1, 16);
      rtl_sr_w(R_DX, &t1);
      break;
    case 4:
      rtl_sr_l(R_EDX, &t0);
      rtl_sr_l(R_EAX, &t1);
      break;
    default: assert(0);
  }

  print_asm_template1(mul);
}

// imul with one operand
make_EHelper(imul1) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_imul(&t0, &t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr_w(R_AX, &t1);
      break;
    case 2:
      rtl_sr_w(R_AX, &t1);
      rtl_shri(&t1, &t1, 16);
      rtl_sr_w(R_DX, &t1);
      break;
    case 4:
      rtl_sr_l(R_EDX, &t0);
      rtl_sr_l(R_EAX, &t1);
      break;
    default: assert(0);
  }

  print_asm_template1(imul);
}

// imul with two operands
make_EHelper(imul2) {
  rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  rtl_imul(&t0, &t1, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t1);

  print_asm_template2(imul);
}

// imul with three operands
make_EHelper(imul3) {
  rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_sext(&id_src2->val, &id_src2->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  rtl_imul(&t0, &t1, &id_src2->val, &id_src->val);
  operand_write(id_dest, &t1);

  print_asm_template3(imul);
}

make_EHelper(div) {
  switch (id_dest->width) {
    case 1:
      rtl_li(&t1, 0);
      rtl_lr_w(&t0, R_AX);
      break;
    case 2:
      rtl_lr_w(&t0, R_AX);
      rtl_lr_w(&t1, R_DX);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_li(&t1, 0);
      break;
    case 4:
      rtl_lr_l(&t0, R_EAX);
      rtl_lr_l(&t1, R_EDX);
      break;
    default: assert(0);
  }

  rtl_div(&t2, &t3, &t1, &t0, &id_dest->val);

  rtl_sr(R_EAX, id_dest->width, &t2);
  if (id_dest->width == 1) {
    rtl_sr_b(R_AH, &t3);
  }
  else {
    rtl_sr(R_EDX, id_dest->width, &t3);
  }

  print_asm_template1(div);
}

make_EHelper(idiv) {
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  switch (id_dest->width) {
    case 1:
      rtl_lr_w(&t0, R_AX);
      rtl_sext(&t0, &t0, 2);
      rtl_msb(&t1, &t0, 4);
      rtl_sub(&t1, &tzero, &t1);
      break;
    case 2:
      rtl_lr_w(&t0, R_AX);
      rtl_lr_w(&t1, R_DX);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_msb(&t1, &t0, 4);
      rtl_sub(&t1, &tzero, &t1);
      break;
    case 4:
      rtl_lr_l(&t0, R_EAX);
      rtl_lr_l(&t1, R_EDX);
      break;
    default: assert(0);
  }

  rtl_idiv(&t2, &t3, &t1, &t0, &id_dest->val);

  rtl_sr(R_EAX, id_dest->width, &t2);
  if (id_dest->width == 1) {
    rtl_sr_b(R_AH, &t3);
  }
  else {
    rtl_sr(R_EDX, id_dest->width, &t3);
  }

  print_asm_template1(idiv);
}
