#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

int sprintf(char *str, const char *format, ...);

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

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
    return strlen((char*)buf);
  }else{
    //time event
    sprintf((char*)buf,"t %u\n",_uptime());
    return strlen(buf);
  }
  return 0;
}

static char dispinfo[128] __attribute__((used));


void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf,dispinfo+offset,len);
  return;
}

void fb_write(const void *buf, off_t offset, size_t len) {
  /*
  y*width +x = offset;
  (y+h)*width +(x+w) = (offset + len);
  juxing!
   */
  int x=0,y=0;
  offset /=sizeof(int);
  len /=sizeof(int);
  x = offset %_screen.width;
  y = offset /_screen.width;
  _draw_rect((const uint32_t*)buf,x,y,len,1);
  // w = (offset+len) %_screen.width-x;
  // h = (offset+len) /_screen.width-y;

}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d", _screen.width, _screen.height);
}
