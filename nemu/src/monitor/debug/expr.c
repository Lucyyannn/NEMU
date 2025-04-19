#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM,TK_HEX,TK_REG,TK_NE,TK_AND,TK_OR,TK_NOT,
  /* TODO: Add more token types */
  TK_NEG,TK_DEREF
};

//int 10
//+-*/
//()
// 
static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\-", '-'},         // sub
  {"\\*", '*'},         // mul
  {"\\/", '/'},         // div
  {"\\(", '('},
  {"\\)", ')'},
  {"0x[0-9a-fA-F]+",TK_HEX},
  {"[0-9]+", TK_NUM},
  {"\\$[A-Za-z0-9]+",TK_REG},
  {"==", TK_EQ},         // equal
  {"!=", TK_NE},
  {"&&", TK_AND},
  {"\\|\\|", TK_OR},
  {"!",TK_NOT},
  {"\\-", TK_NEG},
  {"\\*", TK_DEREF}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool is_binary_op(int k){
  if(k==0){
    return false;
  }
  if(tokens[k-1].type==TK_NUM||tokens[k-1].type==TK_HEX||tokens[k-1].type==TK_REG||tokens[k-1].type==')'){
    return true;
  }
  return false;
}

static bool is_oprand(int k){
  return tokens[k].type==TK_NUM||tokens[k].type==TK_HEX||tokens[k].type==TK_REG;
}
static bool is_single_opcode(int k){
  return tokens[k].type==TK_NOT||tokens[k].type==TK_NEG||tokens[k].type==TK_DEREF;
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case '+':
            assert(nr_token<32);
            tokens[nr_token++].type = '+';
            break;
          case '-':
            assert(nr_token<32);
            tokens[nr_token++].type = '-';
            break;
          case '*':
            assert(nr_token<32);
            tokens[nr_token++].type = '*';
            break;
          case '/':
            assert(nr_token<32);
            tokens[nr_token++].type = '/';
            break;
          case '(':
            assert(nr_token<32);
            tokens[nr_token++].type = '(';
            break;
          case ')':
            assert(nr_token<32);
            tokens[nr_token++].type = ')';
            break;
          case TK_NOTYPE:
            break;
          case TK_NUM:
            assert(nr_token<32);
            tokens[nr_token].type = TK_NUM;
            if(substr_len<sizeof(tokens[nr_token].str)){
              strncpy(tokens[nr_token].str,substr_start,substr_len);
              tokens[nr_token].str[substr_len]='\0';
              nr_token++;
            }else{
              assert(0);
            }
            break;
          case TK_EQ:
            assert(nr_token<32);
            tokens[nr_token++].type = TK_EQ;
            break;
          case TK_NE:
            assert(nr_token<32);
            tokens[nr_token++].type = TK_NE;
            break;
          case TK_AND:
            assert(nr_token<32);
            tokens[nr_token++].type = TK_AND;
            break;
          case TK_OR:
            assert(nr_token<32);
            tokens[nr_token++].type = TK_OR;
            break;
          case TK_NOT:
            assert(nr_token<32);
            tokens[nr_token++].type = TK_NOT;
            break;
          case TK_HEX://0x is in str!
            assert(nr_token<32);
            tokens[nr_token].type = TK_HEX;
            if(substr_len<sizeof(tokens[nr_token].str)){
              strncpy(tokens[nr_token].str,substr_start,substr_len);
              tokens[nr_token].str[substr_len]='\0';
              nr_token++;
            }else{
              assert(0);
            }
            break;
          case TK_REG://$ is not in str!
            assert(nr_token<32);
            tokens[nr_token].type = TK_REG;
            if(substr_len<sizeof(tokens[nr_token].str)){
              strncpy(tokens[nr_token].str,substr_start+1,substr_len-1);
              tokens[nr_token].str[substr_len-1]='\0';
              nr_token++;
            }else{
              assert(0);
            }
            break;
          default:assert(0);
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }
  //NEG + DEREF
  for(int k=0;k<nr_token;k++){
    if(tokens[k].type=='-'){
      if(!is_binary_op(k)){
        tokens[k].type = TK_NEG;
      }
    }
    if(tokens[k].type=='*'){
      if(!is_binary_op(k)){
        tokens[k].type = TK_DEREF;
      }
    }
  }

  return true;
}

static bool check_parentheses(int b,int e){
  int cnt = 0;
  int i=b;
  if(tokens[i].type=='('){
    cnt++;
  }
  i++;
  while(cnt>0&&i<=e){
    if(tokens[i].type=='('){
      cnt++;
    }
    else if(tokens[i].type==')'){
      cnt--;
    }
    i++;
  }
  assert(cnt==0);
  if(i==e+1){
    return true;
  }
  return false;
}

static int find_dominant(int b,int e){
  int curr_dominant_level=-1;
  int curr_index=-1;
  int i=b;
  while(i<=e){
    if(is_oprand(i)){
      i++;
    }else if(tokens[i].type=='('){
      int cnt=1;
      i++;
      while(cnt>0&&i<=e){
        if(tokens[i].type=='('){
          cnt++;
        }
        else if(tokens[i].type==')'){
          cnt--;
        }
        i++;
      }
      assert(cnt==0);
    }else if(is_single_opcode(i)){
      if(curr_index==-1){
        curr_dominant_level=0;
        curr_index = i;
      }
      i++;
    }else{
      if(tokens[i].type==TK_OR){
        curr_dominant_level=5;
        curr_index = i;
      }else if(tokens[i].type==TK_AND&&curr_dominant_level<5){
        curr_dominant_level=4;
        curr_index = i;
      }else if((tokens[i].type==TK_EQ||tokens[i].type==TK_NE)&&curr_dominant_level<4){
        curr_dominant_level=3;
        curr_index = i;
      }else if((tokens[i].type=='+'||tokens[i].type=='-')&&curr_dominant_level<3){
        curr_dominant_level=2;
        curr_index = i;
      }else if((tokens[i].type=='*'||tokens[i].type=='/')&&curr_dominant_level<2){
        curr_dominant_level=1;
        curr_index = i;
      }
      i++;
    }
  }
  return curr_index;
}

static uint32_t eval(int b,int e){
  if(b > e){
    printf("Bad Expression!\n");
    assert(0);
  }else if(b == e){
    switch (tokens[b].type){
      case TK_NUM:{
        char *endptr;
        uint32_t num = strtol(tokens[b].str,&endptr,10);
        assert(*endptr=='\0');
        return num;
      }
      case TK_HEX:{
        char *endptr;
        uint32_t num = strtol(tokens[b].str,&endptr,16);
        assert(*endptr=='\0');
        return num;
      }
      case TK_REG:
        for(int i=0;i<8;i++){
          for(int j=1;j<=4;j*=2){
            if(strcmp(reg_name(i,j),tokens[b].str)==0){
              switch (j) {
                case 4: return reg_l(i);
                case 1: return reg_b(i);
                case 2: return reg_w(i);
                default:;
              }
            }
          }
        }
        if(strcmp("eip",tokens[b].str)==0)
          return cpu.eip;
        else if(strcmp("eflags",tokens[b].str)==0)
          return cpu.eflags;
      default:assert(0);
    }
  }else if(check_parentheses(b,e)){
    return eval(b+1,e-1);
  }else{
    int d = find_dominant(b,e);
    if(tokens[d].type==TK_NOT||tokens[d].type==TK_NEG||tokens[d].type==TK_DEREF){
      uint32_t val = eval(d+1,e);
      switch(tokens[d].type){
        case TK_NEG:return -val;
        case TK_DEREF:return vaddr_read(val,4);
        case TK_NOT:return !val;
        default:assert(0);
      }
    }else{
      uint32_t val1 = eval(b,d-1);
      uint32_t val2 = eval(d+1,e);
      switch(tokens[d].type){
        case '+':return val1+val2;
        case '-':return val1-val2;
        case '*':return val1*val2;
        case '/':
          assert(val2!=0);
          return val1/val2;
        case TK_EQ:return val1==val2;
        case TK_NE:return val1!=val2;
        case TK_AND:return val1&&val2;
        case TK_OR:return val1||val2;
        default:assert(0);
      } 
    }
  }
}
uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  uint32_t ret = eval(0,nr_token-1);
  *success = true;
  return ret;
}
