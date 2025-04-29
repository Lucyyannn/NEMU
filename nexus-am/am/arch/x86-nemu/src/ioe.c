#include <am.h>
#include <x86.h>
#include <stdio.h>

#define RTC_PORT 0x48   // Note that this is not standard
#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64


static unsigned long boot_time;

// inb inl  outb outl
void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  unsigned long curr_time = inl(RTC_PORT);

  return (curr_time-boot_time);
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  int dy;
  for(dy = 0; dy < h; dy++){
    memcpy(&fb[(y+dy)*_screen.width+x],pixels+ dy*w, sizeof(uint32_t)*w);
  //  memcpy(fb+x+(y+dy)*_screen.width , pixels + dy*w, sizeof(uint32_t)*w);
  }
  return;
}

void _draw_sync() {
  
}

int _read_key() {
  if(inb(I8042_STATUS_PORT)==1){
    return inl(I8042_DATA_PORT);
  }
  return _KEY_NONE;
}
