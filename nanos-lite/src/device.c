#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

int sprintf(char *str, const char *format, ...);

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

extern int current_game;
size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  //key event
  if(key!=_KEY_NONE){
    bool down = false;
    if (key & 0x8000) {
      down = true;
      key ^= 0x8000;// delete the down_mask, now key==index
    }
    char* event = down? "kd" :"ku";
    sprintf((char*)buf,"%s %s\n",event,keyname[key]);
    if(down && (key==_KEY_F12)){
      current_game=(current_game==0)?2:0;
    }
    return strlen((char*)buf);
  }
  //time event
  sprintf((char*)buf,"t %u\n",_uptime());
  return strlen(buf);

}

static char dispinfo[128] __attribute__((used));


void dispinfo_read(void *buf, off_t offset, size_t len) {
  // int real_len = strlen(dispinfo)-offset;
  // assert(real_len >0);
  // real_len = (len<real_len)?len:real_len;
  memcpy(buf,dispinfo+offset,len);
  return;
}

void fb_write(const void *buf, off_t offset, size_t len) {
  int x=0,y=0;
  offset /=sizeof(uint32_t);
  len /=sizeof(uint32_t);
  x = offset %_screen.width;
  y = offset /_screen.width;
  assert(len<_screen.width);
  _draw_rect((const uint32_t*)buf,x,y,len,1);

}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d", _screen.width, _screen.height);
}
