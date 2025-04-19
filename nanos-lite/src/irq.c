#include "common.h"
_RegSet* do_syscall(_RegSet *r);
_RegSet* do_trap(_RegSet *r);
_RegSet* do_time(_RegSet *r);
_RegSet* schedule(_RegSet *prev);

static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case 1:
      return do_time(r);
    case 7:
      return do_trap(r);
    case 8:
      return do_syscall(r);
    default: panic("Unhandled event ID = %d", e.event);
  }
  return NULL;
}


_RegSet* do_time(_RegSet *r){
  //Log("OS has received a TIME IRQ!");
  return schedule(r);
};

_RegSet* do_trap(_RegSet *r){
  Log("OS has received a trap!");
  return schedule(r);
};

void init_irq(void) {
  _asye_init(do_event);
}
