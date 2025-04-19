#include "nemu.h"
#include "monitor/monitor.h"
#include "monitor/watchpoint.h"
#include "monitor/expr.h"
/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
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

  for (; n > 0; n --) {
    /* Execute one instruction, including instruction fetch,
     * instruction decode, and the actual execution. */
    // n = -1 and uint64_t a very big number!
    exec_wrapper(print_flag);
    //printf("%08x\n",cpu.eip);

#ifdef DEBUG
    /* TODO: check watchpoints here. */
  WP* curr = head;
  bool changed = false;
  while(curr!=NULL){
    bool flag;
    uint32_t new_value = expr(curr->expr,&flag);
    assert(flag);
    if(new_value!=curr->value){
      printf("The %s value change: 0x%x -> 0x%x\n",curr->expr,curr->value,new_value);
      curr->value = new_value;
      changed = true;
    }
    curr = curr->next;
  }
  if(changed){
    nemu_state = NEMU_STOP;
  }

#endif

#ifdef HAS_IOE
    extern void device_update();
    device_update();
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
