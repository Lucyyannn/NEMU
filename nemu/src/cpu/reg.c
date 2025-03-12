#include "nemu.h"
#include <time.h>
#include <stdlib.h>

CPU_state cpu;

const char *regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};


void reg_test() {
  srand(time(0));
  uint32_t sample[8];
  uint32_t eip_sample = rand();
  cpu.eip = eip_sample;

  int i;
  for (i = R_EAX; i <= R_EDI; i ++) {
    sample[i] = rand();
    reg_l(i) = sample[i];
    assert(reg_w(i) == (sample[i] & 0xffff));
  }

  assert(reg_b(R_AL) == (sample[R_EAX] & 0xff));
  assert(reg_b(R_AH) == ((sample[R_EAX] >> 8) & 0xff));
  assert(reg_b(R_BL) == (sample[R_EBX] & 0xff));
  assert(reg_b(R_BH) == ((sample[R_EBX] >> 8) & 0xff));
  assert(reg_b(R_CL) == (sample[R_ECX] & 0xff));
  assert(reg_b(R_CH) == ((sample[R_ECX] >> 8) & 0xff));
  assert(reg_b(R_DL) == (sample[R_EDX] & 0xff));
  assert(reg_b(R_DH) == ((sample[R_EDX] >> 8) & 0xff));


  assert(sample[R_EAX] == cpu.eax);
  assert(sample[R_ECX] == cpu.ecx);
  assert(sample[R_EDX] == cpu.edx);
  assert(sample[R_EBX] == cpu.ebx);
  assert(sample[R_ESP] == cpu.esp);
  assert(sample[R_EBP] == cpu.ebp);
  assert(sample[R_ESI] == cpu.esi);
  assert(sample[R_EDI] == cpu.edi);

  assert(eip_sample == cpu.eip);
}

// compare two char*
uint32_t strcomp(char* a,const char* b){
  //get length
  int i,j;
  for(i=0;*(a+i)!='\0';++i){ printf("he\n");}
  for(j=0;*(b+j)!='\0';++j){printf("xxx\n");}
  // compare length
  if(i!=j){
    return -1;
  }
  //compare content
  for(i=0;i<=j;++i){
    if(*(a+i)!=*(b+i)){return -1;}
  }
  return 0;
}


void print_reg_info(){
  for(int i=0;i<8;i++){
    printf("| %s  ",reg_name(i,4));
    printf("%08X     ",reg_l(i));
    printf("| %s  ",reg_name(i,2));
    printf("%08X     ",reg_w(i));
    printf("| %s  ",reg_name(i,1));
    printf("%08X      |\n",reg_b(i));
  }
  printf("pc     %08X\n",cpu.eip);
  return ;
}

// get reg value by name
uint32_t get_reg_value(char* name){
  printf("miya\n");
  //char* name = prename+1;
  uint32_t result = 0;
  for(int i=0;i<8;i++){
    if(strcomp(name,regsl[i])==0){

      printf("miyaya\n");
      result = reg_l(i);
      return result;
    }
  }
  for(int i=0;i<8;i++){
    if(strcomp(name,regsw[i])==0){
      result = reg_w(i);
      return result;
    }
  }
  for(int i=0;i<8;i++){
    if(strcomp(name,regsb[i])==0){
      result = reg_b(i);
      return result;
    }
  }
  assert(0);
  return 0;
}