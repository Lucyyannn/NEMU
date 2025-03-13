#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "cpu/reg.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

extern CPU_state cpu;


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
  cpu_exec(-1);
  return 0;
}
static int cmd_q(char *args) {
  return -1;
}
static int cmd_help(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_p(char *args);
static int cmd_x(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);


static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute N instructions, the default number is 1", cmd_si},
  { "info", "Print the state of the program", cmd_info},
  { "p", "Compute the expression", cmd_p},
  { "x", "Scan the memory", cmd_x},
  { "w", "Set the watchpoint", cmd_w},
  { "d", "Delete the watchpoint", cmd_d}
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

static int cmd_si(char *args){
  //default number=1
  if(args==NULL){
    cpu_exec(1);
    return 0;
  }
  //get the valid number
  int n;
  if(sscanf(args, "%d", &n)==EOF){
    printf("Please input a number for si\n");
  }else{
    cpu_exec(n);//execute n steps
  }
  return 0;
}

static int cmd_info(char *args){
  // info r: print all the regs' value
  if(*args=='r'){
    print_reg_info();
  }// info w: print the watchpoint value
  else if(*args=='w'){
    //TODO :print the position and number of all the watchpoints
    print_wp_pool_info();
  }
  return 0;
}

// compute the expression
static int cmd_p(char *args){
  bool success=true;
  int result= expr(args,&success);
  printf("%d \n",result);
  return 0;
}

//scan the memory
static int cmd_x(char *args){
  assert(strlen(args)>=3);
  // (1)args --> num + expression
  char * number = (char*)malloc(5*sizeof(char));
  int i=0;
  for(i=0;args[i]!=32;++i){
    *(number+i)=args[i];
  }
  int num=comp_value_by_string(number,i);
  char * args2=(char*)malloc(32*sizeof(char));
  int j=i+1;
  for(j=i+1;args[j]!='\0';++j){
    *(args2+j-i-1)=args[j];
  }args2[j-i-1]='\0';
  printf("the expression: %s \n",args2);
  //(2) compute the expression
  bool success = true;
  uint32_t result = expr(args2,&success);
  if(!success){
    return 0;
  }
  //(3) scan the memory nearby
  uint32_t addr = 0;
  for(int i=0;i<num;i++){
    addr = vaddr_read(result+i*4,4);
    printf("%08X ",addr);
  }printf("\n");
  return 0;
}

// set the watchpoints
static int cmd_w(char *args){
  WP* wp=new_wp();
  wp->expression=args;

  w_tokens[w_nr]=wp->expression;

  return 0;
}
static int cmd_d(char *args){
  assert(args!=NULL);
  WP* wp=find_wp(args);
  // delete it from the list
  free_wp(wp); 
  //delete the global watchpoint expression infomation
  for(int i=0;i<w_nr;i++){
    if(strcmp(args,w_tokens[i])==0){
      for(int j=i;j<w_nr-1;j++){
        w_tokens[j]=w_tokens[j+1];
      }
      break;
    }
  }
  --w_nr;

  return 0;
}


void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

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
 