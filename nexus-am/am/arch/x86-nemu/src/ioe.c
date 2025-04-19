#include <am.h>
#include <x86.h>
#define RTC_PORT 0x48   // Note that this is not standard
#define I8042_DATA_PORT 0x60
#define I8042_STATUS_PORT 0x64
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  unsigned long now_time = inl(RTC_PORT);
  return now_time-boot_time;
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};

int get_width(){
  return _screen.width;
}

int get_height(){
  return _screen.height;
}

extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  
  int i,t;
  i=y*_screen.width+x;
  for (t=1;t<=h;t++) {
    for(int j=0; j<w;j++){
      fb[i+j]=*pixels;
    }
    i+=_screen.width;
  }
}

void _draw_sync() {
}

int _read_key() {
  int key=0;
  int flag = inb(I8042_STATUS_PORT);
  if(flag){
    key = inl(I8042_DATA_PORT);
  }
  return key;
}
