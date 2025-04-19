#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#define WP_NUMBER 100

static int wp_cnt=1;
static WP* wp_runs[WP_NUMBER+1];

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);//nemu/src/monitor/cpu-exec.c
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
  uint64_t N = 1;
  if(args!=NULL){
    char *endptr;
    N = strtoul(args,&endptr,10);
    if(*endptr != '\0'){
      printf("The si cmd incurs a fault!\n");
      return 0;
    }
  }
  cpu_exec(N);
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  if(arg!=NULL){
    if(strcmp("r",arg)==0){
      for(int i=0;i<8;i++){
        printf("%s : 0x%08x\n",regsl[i],reg_l(i));
      }
      printf("eip : 0x%08x\n",cpu.eip);
      printf("cr0 : 0x%08x\n",cpu.cr0);
      printf("cr3 : 0x%08x\n",cpu.cr3);
      printf("idtr_base: 0x%08x idtr_limit: 0x%04x\n",cpu.idtr_base,cpu.idtr_limit);
      printf("eflags:O-%x|I-%x|S-%x|Z-%x|C-%x\n",cpu.OF,cpu.IF,cpu.SF,cpu.ZF,cpu.CF);
    }else if(strcmp("w",arg)==0){
      for(int i=1;i<=WP_NUMBER;i++){
        if(wp_runs[i]!=NULL){
          printf("The watchpoint %u. expr:%s value:0x%x\n",wp_runs[i]->NO,wp_runs[i]->expr,wp_runs[i]->value);
        }
      }
    }else{
      printf("Unknown sub-command '%s'\n", arg);
    }
  }else{
    printf("No sub-command\n");
  }
  return 0;
}


static int cmd_x(char *args) {
  char *arg_N = strtok(NULL, " ");
  char *arg_EXPR = strtok(NULL, "\n");
  if(arg_N==NULL){
    printf("No N arg\n");
  }
  if(arg_EXPR==NULL){
    printf("No EXPR arg\n");
  }
  if(arg_N!=NULL&&arg_EXPR!=NULL){
    char *endptr1;
    int N = strtol(arg_N,&endptr1,10);

    bool flag;
    uint32_t val=expr(arg_EXPR,&flag);
    if(*endptr1 != '\0'||!flag){
      printf("The x cmd incurs a fault!\n");
    }else{
      uint32_t value = vaddr_read(val,4);
      for(int i=1;i<=N;i++){
        printf("0x%x-0x%x : %08x\n",val+3,val,value);
        val+=4;
        value = vaddr_read(val,4);
      }
    }
  }
  return 0;
}

static int cmd_p(char *args){
  bool flag;
  uint32_t val=expr(args,&flag);
  if(flag){
    printf("d:%d|u:%u|x:0x%x\n",val,val,val);
  }else{
    printf("%s is a wrong expr!\n",args);
  }
  return 0;
}

static void init_wp_runs(){
  for(int i=0;i<WP_NUMBER;i++){
    wp_runs[i]=NULL;
  }
}

static int cmd_w(char *args){
  assert(wp_cnt<=WP_NUMBER);
  bool flag;
  WP* wp = new_wp();
  wp->NO = wp_cnt;
  wp->expr = strdup(args);
  wp->value = expr(wp->expr,&flag);
  assert(flag);
  wp_runs[wp_cnt++]=wp;
  printf("The watchpoint %u has built. expr:%s value:0x%x\n",wp->NO,wp->expr,wp->value);
  return 0;
}

static int cmd_d(char *args){
  int NO;
  char *endptr;
  NO = strtol(args,&endptr,10);
  assert(*endptr == '\0');
  assert(wp_runs[NO]!=NULL);
  free_wp(wp_runs[NO]);
  wp_runs[NO]=NULL;
  printf("The watchpoint %u has removed.\n",NO);
  return 0; 
}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "format:si [N].Let the program step through N instructions and then pause execution, If N is not given, the default value is 1",cmd_si },
  { "info", "format:info SUBCMD.Print program status.Parameter r prints register status and parameter w prints monitor information",cmd_info },
  { "x", "format:x N EXPR.Evaluate the expression EXPR and use the result as the starting memory Address, output N consecutive 4 bytes in hexadecimal form",cmd_x },
  { "p", "format:p EXPR.Evaluate the expression EXPR",cmd_p},
  { "w", "format:w EXPR.When the value of EXPR changes, program execution is suspended",cmd_w},
  { "d", "format:d N.Delete a watchpoint with NO.N",cmd_d},
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }
  init_wp_runs();
  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
