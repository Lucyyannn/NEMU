#include <am.h>
#include <x86.h>

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
  int i;
  for (i = 0; i < _screen.width * _screen.height; i++) {
    fb[i] = *(pixels+i);
  }

  // int dy;
  // for (dy = 0; dy < h; ++dy)
  //   memcpy(fb + (y + dy)*_screen.width + x, pixels + w * dy, sizeof(uint32_t) * w);
}

void _draw_sync() {
}

int _read_key() {
  int ret = _KEY_NONE;
  if(inb(I8042_STATUS_PORT)==1){
    ret = inl(I8042_DATA_PORT);
  }

  return ret;
}
