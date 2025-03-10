#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

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

  {"\\+", '+'},         // plus
  {"\\-",'-'},          //sub
  {"\\*",'*'},          //multi
  {"/",'/'},            //div
  {"%",'%'},            //mod
  {"\\(",'('},
  {"\\)",')'},

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
  int precedence;
} Token;

Token tokens[32];// the tokens that have already been recognized
int nr_token;    //the number of tokens above

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;// to locate
  // pmatch.rm_so, pmatch_rm_eo
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

        switch (rules[i].token_type) {
          case TK_PLUS://+
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV4;
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
            break;
          case TK_DIV://  /
          case TK_MOD://  /
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV3;
            ++nr_token;
            break;
          case TK_LPARENTHESIS:// (
          case TK_RPARENTHESIS:// )
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV1;
            ++nr_token;
            break;
          case TK_REGISTER:
            // if()
          case TK_NUMBER:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV0;
            ++nr_token;
            break;
          case TK_EQ:
          case TK_NEQ:
          case TK_LEQ:
          case TK_BEQ:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV7;
            ++nr_token;
            break;
          case TK_AND:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV11;
            ++nr_token;
            break;
          case TK_OR:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].precedence = OP_LV12;
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
  return false;
}
// p: start position
// q: end position
uint32_t eval(int p, int q){
  // if(p>q){
  //   panic("It's impossible that p>q.")
  // }else if(p==q){
  //   //sigle token ,return its value
  //   // if(tokens[p]){

  //   // }

  // }else if(checkparenthesis(p,q)){

  // }else{

  // }
  return 0;
}


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  if(nr_token==0){
    return 0;
  }
  int p=0,q=nr_token-1;

  return eval(p,q);
}

// (0)all the results are uint32_t
// (1)tell apart the two possibilities of returning false
// (2)about -x
// (3)pay attention to the priority and the associativity
// (4)to add entirely

// 1.define the rules                          ~
// 2.regular expressiong --> identify tokens   ~
// 3.make token()                              ~
// 4.eval(recursive computing)                                     
// 5.expand expressions to P30,especially '*'  ~
