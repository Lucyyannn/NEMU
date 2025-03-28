#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

uintptr_t loader(_Protect *as, const char *filename) {
  void *buffer;
  size_t len = get_ramdisk_size();
  ramdisk_read(buffer,0,len);
  memcpy(DEFAULT_ENTRY,buffer,len);
  return (uintptr_t)DEFAULT_ENTRY;
}
