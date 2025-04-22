#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  //return ((a*b)>>16);
  int res=a*b;
  int* res_ptr=&res;
  int res_pre = *(res_ptr+1);//the overflow 32bits
  int result = ((res_pre&(0xFFFF-1))<<16) | ((res&0xFFFF0000)>>16);
  if((a>>31)!=(b>>31)){result=-result;}
  return result;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  return ((a/b)<<16);
}

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */

  uint32_t *floatptr = (uint32_t*)&a;
  uint32_t ori=*floatptr;
  if(ori==0||ori==(1<<31)){return 0;}
  //(1) S
  uint32_t neg=(ori>>31)&1;//1 -  
  //(2) Exponent->k
  uint32_t ExpoMask = 0xFF;
  int k = ((ori>>23)&ExpoMask)-127;
  //(3) 1+Mantissa -->ss
  uint32_t MantiMask = (1<<23)-1;
  uint32_t ss= ((1<<23) | (ori&MantiMask));
  int PointStatus=23-k;// point position
  //(4) ss-->FLOAT(Fabs)
  if(PointStatus>32){PointStatus=32;}
  uint32_t fab_res=0;
  fab_res |= ((ss>>PointStatus)<<16);//integer part
  fab_res |= ((ss<<(32-PointStatus))>>16);
  //(5) --> FLAOT(with sign)
  return ((neg==1)? (-(int)fab_res):(int)fab_res);
}

FLOAT Fabs(FLOAT a) {

  if(a>>31){// -
    return -a;
  }else{    // +
    return a; // C stores it by buma
  }
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);//t2=t^2
    dt = (F_div_F(x, t2) - t) / 3;// (x/t^2-t)/3
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));//0.0001

  return t;
}
