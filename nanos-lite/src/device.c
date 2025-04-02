#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

char *strncpy(char *dest, const char *src, size_t n);
int sprintf(char *str, const char *format, ...);
//void *malloc(size_t size);

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  return 0;
}

static char dispinfo[128] __attribute__((used));


void dispinfo_read(void *buf, off_t offset, size_t len) {
  //buf = (char*)malloc(len+1);
  strncpy((char*)buf,dispinfo+offset,len);
  // if(*(dispinfo+offset+len-1)!='\0'){
  //   *(buf+len)='\0';
  // }
  return;
}

void fb_write(const void *buf, off_t offset, size_t len) {
  /*
  y*width +x = offset;
  (y+h)*width +(x+w) = (offset + len);
  juxing!
   */
  int x=0,y=0,w=0,h=0;
  offset /=sizeof(int);
  len /=sizeof(int);
  x = offset %_screen.width;
  y = offset /_screen.width;
  w = (offset+len) %_screen.width-x;
  h = (offset+len) /_screen.width-y;
  assert(w>=0 &&h>=0);
  _draw_rect((const uint32_t*)buf,x,y,w,h);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
}
