#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};
#define KEYDOWN_MASK 0x8000

int _read_key();
unsigned long _uptime();

extern int curr_game;

ssize_t events_read(void *buf, size_t len) {
  char read_str[128];
  int key = _read_key();
  if(key == 13){
    curr_game = (curr_game == 0 ? 2:0);
  }
  if(key!=0){
    if((key&KEYDOWN_MASK)>>15){
      sprintf(read_str,"kd %s\n",keyname[key&0x7fff]);
    }else{
      sprintf(read_str,"ku %s\n",keyname[key&0x7fff]);
    }
  }else{
    sprintf(read_str,"t %d\n",_uptime());
  }
  int real_len = (strlen(read_str))>len?len:strlen(read_str);
  memcpy(buf,read_str,real_len);
  
  return real_len;
}

static char dispinfo[128] __attribute__((used));

ssize_t dispinfo_read(void *buf, off_t offset, size_t len) {
  int size_disp = strlen(dispinfo);
  int res_len = size_disp-offset;
  int read_len = (len>res_len)?res_len:len;
  memcpy(buf, dispinfo+offset, read_len);
  return read_len;
}


void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h);

ssize_t fb_write(const void *buf, off_t offset, size_t len) {
  int vga_w = _screen.width;
  int vga_h = _screen.height;
  assert(offset%4==0);
  assert(len%4==0);
  int pixels_num = len/4;
  int i;
  for(i=0;i<pixels_num;i++){
    if(offset+i*4>=(vga_h*vga_w*4)){
      break;
    }
    int y = (offset+i*4)/(4*vga_w);
    int x = (offset+i*4)%(4*vga_w)/4;
    _draw_rect((uint32_t*)buf+i,x,y,1,1);
  }
  return i*4;
}

void init_device() {
  _ioe_init();
  int vga_w = _screen.width;
  int vga_h = _screen.height;
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", vga_w, vga_h);
  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
}
