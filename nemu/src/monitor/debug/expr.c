#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

// additional functions
long long powcomp(int base, int exponent){
  long long res=1;
  for(int i=0;i<exponent;++i){
    res*=base;
  }
  return res;
}

void cpy(char* dest, char* src){
  printf("Hi ! \n");
  assert(strlen(src)>0);
  printf("Hello ! \n");
  int n=strlen(src);
  dest = (char*)malloc((n+1)*sizeof(char));
  for(int i=0;i<n;++i){
    *(dest+i)=*(src+i);
  }
  if(dest[n-1]!='\0'){
    dest[n]='\0';
  }
  return;
}

void ncpy(char* dest, char* src, int n){
  printf("Hi ! \n");
  assert(strlen(src)>=n);
  printf("Hello ! \n");
  dest = (char*)malloc((n+1)*sizeof(char));
  for(int i=0;i<n;++i){
    *(dest+i)=*(src+i);
  }
  if(dest[n-1]!='\0'){
    dest[n]='\0';
  }
  printf("How are you? \n");
  return ;
}

//token type
enum {
  TK_NOTYPE = 256, 
  TK_PLUS,  // +
  TK_SUB,   // -
  TK_MULTI, // *
  TK_DIV,   // /
  TK_MOD,   // %
  TK_LPARENTHESIS, // (
  TK_RPARENTHESIS, // )
  TK_REGISTER,
  TK_NUMBER,
  TK_EQ,
  TK_NEQ,
  TK_LEQ,
  TK_BEQ,
  TK_AND,
  TK_OR,
  TK_U, // unary -
  TK_P  //deref *
};
//precedence level
enum {
  OP_LV0 = 0 ,    // number , register
  OP_LV1 = 10 ,   // ()
  OP_LV2_1 = 21 , // unary, - 
  OP_LV2_2 = 22 , // deference, *
  OP_LV3 = 30 ,   // *  /   %
  OP_LV4 = 40 ,   // + , − 
  OP_LV7 = 70 ,   // == , != ,>= ,<=
  OP_LV11 = 110 , // &&
  OP_LV12 = 120 , // ||

 } ;

static struct rule {
  char *regex;
  int token_type;
}  rules[] = {
  {" +", TK_NOTYPE},    // spaces

  {"\\+", TK_PLUS},         // plus
  {"\\-",TK_SUB},          //sub
  {"\\*",TK_MULTI},          //multi
  {"/",TK_DIV},            //div
  {"%",TK_MOD},            //mod
  {"\\(",TK_LPARENTHESIS},
  {"\\)",TK_RPARENTHESIS},

  {"\\$[a-zA-Z]+",TK_REGISTER},
  {"(0|[1-9][0-9]*)",TK_NUMBER},
  {"0[xX][0-9a-fA-F]+",TK_NUMBER},//hex
  
  {"==", TK_EQ},       
  {"!=",TK_NEQ},        
  {"<=",TK_LEQ},
  {">=",TK_BEQ},

  {"&&",TK_AND},        //and
  {"\\|\\|",TK_OR}      //or
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
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED); //compile the regexes
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}


typedef struct token {
  int type;
  char str[32];
  uint32_t value;
  int precedence;
} Token;

Token tokens[32];// the tokens that have already been recognized
int nr_token;    //the number of tokens above

uint32_t hex_to_num(char numstr){
  if(numstr>='0'&&numstr<='9'){
    return (numstr-'0');
  }
  if(numstr>='a'&&numstr<='f'){
    return (numstr-'a'+10);
  }
  if(numstr>='A'&&numstr<='F'){
    return (numstr-'A'+10);
  }
  assert(0);
  return 0;
}

uint32_t comp_value_by_string(char* number,int length){
  uint32_t value=0;
  int start_p=0,end_p=length-1;
  //hex
  if((end_p+1)>2&&(*(number+1)=='x'||*(number+1)=='X')){
    start_p=2;
    for(int i=end_p;i>=start_p;i--){
      uint32_t temp=hex_to_num(number[i]);
      value+=temp*powcomp(16,(end_p-i));
    }
    return value;
  }
  //dec
  else{
    for(int i=end_p;i>=start_p;i--){
      uint32_t temp = number[i]-'0';
      value+=temp*powcomp(10,(end_p-i));
    }
    return value;
  }

  assert(0);
  return 0;
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;// to locate
  // pmatch.rm_so, pmatch._rm_eo
  nr_token = 0;
  uint32_t value=0;
  char *substr='\0';

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        printf("substr_start:%s",substr_start);

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len,substr_len,substr_start);
        position += substr_len;


        switch (rules[i].token_type) {
          case TK_PLUS://+
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV4;
            cpy(tokens[nr_token].str,"+\0");
            ++nr_token;
            break;
          case TK_SUB:
            //unary
            if(nr_token==0||(nr_token>0&&tokens[nr_token-1].precedence!=OP_LV0)){
              tokens[nr_token].type = TK_U;
              tokens[nr_token].precedence = OP_LV2_1;
            }else{
            // -
              tokens[nr_token].type = rules[i].token_type;
              tokens[nr_token].precedence = OP_LV4;
            }
            cpy(tokens[nr_token].str,"-\0");
            ++nr_token;
            break;
          case TK_MULTI:
            //deref
            if(nr_token==0||(nr_token>0&&tokens[nr_token-1].precedence!=OP_LV0)){
              tokens[nr_token].type = TK_P;
              tokens[nr_token].precedence = OP_LV2_2;
            }else{
            // *
              tokens[nr_token].type = rules[i].token_type;
              tokens[nr_token].precedence = OP_LV3;
            }
            cpy(tokens[nr_token].str,"*\0");
            ++nr_token;
            break;
          case TK_DIV://  /
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV3;
            cpy(tokens[nr_token].str,"/\0");
            ++nr_token;
            break;
          case TK_MOD://  %
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV3;
            cpy(tokens[nr_token].str,"%\0");
            ++nr_token;
            break;
          case TK_LPARENTHESIS:// (
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV1;
            cpy(tokens[nr_token].str,"(\0");
            ++nr_token;
            break;
          case TK_RPARENTHESIS:// )
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV1;
            cpy(tokens[nr_token].str,")\0");
            ++nr_token;
            break;
          case TK_REGISTER:// register
            ncpy(substr,substr_start,substr_len);
            value=get_reg_value(substr);
            printf("Nice to meet you,too \n");
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV0;
            tokens[nr_token].value=value;
            cpy(tokens[nr_token].str,substr);
            ++nr_token;
            break;
          case TK_NUMBER: // number   hex or dec
            value=comp_value_by_string(substr_start,substr_len);
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV0;
            tokens[nr_token].value = value;
            cpy(tokens[nr_token].str,substr);
            ++nr_token;
            break;
          case TK_EQ:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV7;
            cpy(tokens[nr_token].str,"==\0");
            ++nr_token;
            break;
          case TK_NEQ:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV7;
            cpy(tokens[nr_token].str,"!=\0");
            ++nr_token;
            break;
          case TK_LEQ:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV7;
            cpy(tokens[nr_token].str,">=\0");
            ++nr_token;
            break;
          case TK_BEQ:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV7;
            cpy(tokens[nr_token].str,"<=\0");
            ++nr_token;
            break;
          case TK_AND:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV11;
            cpy(tokens[nr_token].str,"&&\0");
            ++nr_token;
            break;
          case TK_OR:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV12;
            cpy(tokens[nr_token].str,"||\0");
            ++nr_token;
            break;
          default:
            break;
        }
        break;
      }
    }
    // recognize failed
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool checkparenthesis(int p, int q){
  // (    )
  if(tokens[p].type==TK_LPARENTHESIS&&
    tokens[q].type==TK_RPARENTHESIS){
      // if ( , depth++; if ) , depth--
      // only when depth==0 at last, it's true
      int depth=0;
      for(int i=p+1;i<q;++i){
        if(tokens[i].type==TK_LPARENTHESIS){
          ++depth;
        }else if(tokens[i].type==TK_RPARENTHESIS){
          --depth;
          if(depth<0){
            return false;
          }
        }
      }
      if(depth==0){
        return true;
      }
  }
  return false;
}
// p: start position
// q: end position
uint32_t eval(int p, int q){
  if(p>q){
    panic("It's impossible that p>q.");
  }else if(p==q){
    // only number or register possible
    assert(tokens[p].precedence==OP_LV0);
    return tokens[p].value;

  }else if(checkparenthesis(p,q)){
    // ( sub_expr )  --> cut off the ()
    return eval(p+1,q-1);

  }else{
    // look for the dominant operator
    int d=0;//position of the dominant operator
    int depth=0;
    for(int i=p;i<=q;i++){
      // identify the ()
      if(tokens[i].type==TK_LPARENTHESIS){
        ++depth;
      }else if(tokens[i].type==TK_RPARENTHESIS){
        --depth;
      }
      // only for +-*/%, and not in ()
      else if(depth==0&&(tokens[i].precedence==OP_LV3||tokens[i].precedence==OP_LV4)){
        if(d==0||tokens[i].precedence>=tokens[d].precedence){
          d=i;
        }
      }
    }
    assert(depth==0);
    assert(d!=0);
    //compute substr and combine
    uint32_t val1 = eval(p,d-1);
    uint32_t val2 = eval(d+1,q);
    switch(tokens[d].type){
      case TK_PLUS:
        return val1+val2;
      case TK_SUB:
        return val1-val2;
      case TK_MULTI:
        return val1*val2;
      case TK_DIV:
        assert(val2!=0);
        return val1/val2;
      case TK_MOD:
        assert(val2!=0);
        return val1%val2;
      default:
        assert(0);
    }

  }
  assert(0);
  return 0;
}


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  printf("nr_token:%d \n",nr_token);
  for(int i=0;i<nr_token;i++){
    printf("type:%d \n",tokens[i].type);
  }

  if(nr_token==0){
    return 0;
  }
  int p=0,q=nr_token-1;

  uint32_t res=eval(p,q);
  printf("res:%d \n",res);
  return res;
}

// (0)all the results are uint32_t
// (1)tell apart the two possibilities of returning false
// (2)about -x
// (3)pay attention to the priority and the associativity
// (4)to add entirely

// 1.define the rules                          ~
// 2.regular expressiong --> identify tokens   ~
// 3.make token()                              ~
// 4.eval(recursive computing)                 ~                       
// 5.expand expressions to P30,especially '*'  ~

// with '-'
