#include <stdlib.h>
#include "nemu.h"
#include "monitor/monitor.h"
#include "monitor/expr.h"

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
 /*
 nemu_state: END/RUNNING/STOP
  */
#define MAX_INSTR_TO_PRINT 10

int nemu_state = NEMU_STOP;

void exec_wrapper(bool);

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  if (nemu_state == NEMU_END) {
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  }
  nemu_state = NEMU_RUNNING;

  bool print_flag = n < MAX_INSTR_TO_PRINT;

  // for debug: watchpoints
  bool success;
  uint32_t *values=(uint32_t *)malloc(w_nr * sizeof(uint32_t));
  for(int i=0;i<w_nr;i++){
    values[i]=expr(w_tokens[i],&success);
  }

  // run for each step
  for (; n > 0; n --) {
    /* Execute one instruction, including instruction fetch,
     * instruction decode, and the actual execution. */
    exec_wrapper(print_flag);

#ifdef DEBUG
    /* check watchpoints here. */
    for(int i=0;i<w_nr;i++){
      uint32_t result=expr(w_tokens[i],&success);
      if(result!=values[i]){
        nemu_state = NEMU_STOP;
        printf("You have triggered the watchpoint: %s.",w_tokens[i]);
        printf("origion value: %d , current value: %d .\n",values[i],result);
      }
    }

#endif

#ifdef HAS_IOE
    extern void device_update();
    device_update();
#endif

    if (nemu_state != NEMU_RUNNING) { 
      free(values);
      values=NULL;
      return; 
    }
  }


  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
