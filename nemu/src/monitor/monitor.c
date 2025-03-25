#include "nemu.h"
#include <unistd.h>

#define ENTRY_START 0x100000

void init_difftest();
void init_regex();
void init_wp_pool();
void init_device();

void reg_test();
void init_qemu_reg();
bool gdb_memcpy_to_qemu(uint32_t, void *, int);

FILE *log_fp = NULL;
static char *log_file = NULL;
static char *img_file = NULL;
static int is_batch_mode = false;

static inline void init_log() {
#ifdef DEBUG
  if (log_file == NULL) return;
  log_fp = fopen(log_file, "w");
  Assert(log_fp, "Can not open '%s'", log_file);
#endif
}

static inline void welcome() {
  printf("Welcome to NEMU!\n");
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("For help, type \"help\"\n");
}

static inline int load_default_img() {
  // const uint8_t img []  = {
  //   0xb8, 0x34, 0x12, 0x00, 0x00,        // 100000:  movl  $0x1234,%eax
  //   0xb9, 0x27, 0x00, 0x10, 0x00,        // 100005:  movl  $0x100027,%ecx
  //   0x89, 0x01,                          // 10000a:  movl  %eax,(%ecx)
  //   0x66, 0xc7, 0x41, 0x04, 0x01, 0x00,  // 10000c:  movw  $0x1,0x4(%ecx)
  //   0xbb, 0x02, 0x00, 0x00, 0x00,        // 100012:  movl  $0x2,%ebx
  //   0x66, 0xc7, 0x84, 0x99, 0x00, 0xe0,  // 100017:  movw  $0x1,-0x2000(%ecx,%ebx,4)
  //   0xff, 0xff, 0x01, 0x00,
  //   0xb8, 0x00, 0x00, 0x00, 0x00,        // 100021:  movl  $0x0,%eax
  //   0xd6,                                // 100026:  nemu_trap
  // };
  const uint8_t img []  = {
0xbd, 0x00, 0x00, 0x00, 0x00,        // 100000: movl $0x0,%ebp
0xbc, 0x00, 0x7c, 0x00, 0x00,        // 100005: movl $0x7c00,%esp
0xe8, 0x0d, 0x00, 0x00, 0x00,        // 10000a: call 10001c <_trm_init>
0x90,                                // 10000f: nop

0x55,                                // 100010: push %ebp
0x89, 0xe5,                          // 100011: mov %esp,%ebp
0x8b, 0x45, 0x08,                    // 100013: mov 0x8(%ebp),%eax
0xd6,                                // 100016: (bad) nemu_trap
0xeb, 0xfe,                          // 100017: jmp 100017 <_halt+0x7>
0x66, 0x90,                          // 100019: xchg %ax,%ax
0x90,                                // 10001b: nop

0x55,                                // 10001c: push %ebp
0x89, 0xe5,                          // 10001d: mov %esp,%ebp
0x83, 0xec, 0x08,                    // 10001f: sub $0x8,%esp
0xe8, 0x2d, 0x00, 0x00, 0x00,        // 100022: call 100054 <main>
0xd6,                                // 100027: (bad) nemu_trap
0xeb, 0xfe,                          // 100028: jmp 100028 <_trm_init+0xc>
0x66, 0x90,                          // 10002a: xchg %ax,%ax

0x55,                                // 10002c: push %ebp
0x89, 0xe5,                          // 10002d: mov %esp,%ebp
0x8b, 0x45, 0x08,                    // 10002f: mov 0x8(%ebp),%eax
0x85, 0xc0,                          // 100032: test %eax,%eax
0x74, 0x02,                          // 100034: je 100038 <nemu_assert+0xc>
0x5d,                                // 100036: pop %ebp
0xc3,                                // 100037: ret
0xc7, 0x45, 0x08, 0x01, 0x00, 0x00, 0x00, // 100038: movl $0x1,0x8(%ebp)
0x5d,                                // 10003f: pop %ebp
0xe9, 0xcb, 0xff, 0xff, 0xff,        // 100040: jmp 100010 <_halt>

0x8d, 0x76, 0x00,                    // 100045: lea 0x0(%esi),%esi

0x55,                                // 100048: push %ebp
0x89, 0xe5,                          // 100049: mov %esp,%ebp
0x81, 0x7d, 0x08, 0xf4, 0x01, 0x00, 0x00, // 10004b: cmpl $0x1f4,0x8(%ebp)
0x7e, 0x08,                          // 100052: jle 10005c <if_else+0x14>
0xb8, 0x96, 0x00, 0x00, 0x00,        // 100054: mov $0x96,%eax
0x5d,                                // 100059: pop %ebp
0xc3,                                // 10005a: ret
0x90,                                // 10005b: nop

0x81, 0x7d, 0x08, 0x2c, 0x01, 0x00, 0x00, // 10005c: cmpl $0x12c,0x8(%ebp)
0x7e, 0x07,                          // 100063: jle 10006c <if_else+0x24>
0xb8, 0x64, 0x00, 0x00, 0x00,        // 100065: mov $0x64,%eax
0x5d,                                // 10006a: pop %ebp
0xc3,                                // 10006b: ret

0x83, 0x7d, 0x08, 0x64,              // 10006c: cmpl $0x64,0x8(%ebp)
0x7e, 0x0a,                          // 100070: jle 10007c <if_else+0x34>
0xb8, 0x4b, 0x00, 0x00, 0x00,        // 100072: mov $0x4b,%eax
0x5d,                                // 100077: pop %ebp
0xc3,                                // 100078: ret
0x8d, 0x76, 0x00,                    // 100079: lea 0x0(%esi),%esi

0x83, 0x7d, 0x08, 0x32,              // 10007c: cmpl $0x32,0x8(%ebp)
0x7e, 0x0a,                          // 100080: jle 10008c <if_else+0x44>
0xb8, 0x32, 0x00, 0x00, 0x00,        // 100082: mov $0x32,%eax
0x5d,                                // 100087: pop %ebp
0xc3,                                // 100088: ret
0x8d, 0x76, 0x00,                    // 100089: lea 0x0(%esi),%esi

0x31, 0xc0,                          // 10008c: xor %eax,%eax
0x5d,                                // 10008e: pop %ebp
0xc3,                                // 10008f: ret

0x8d, 0x4c, 0x24, 0x04,              // 100090: lea 0x4(%esp),%ecx
0x83, 0xe4, 0xf0,                    // 100094: and $0xfffffff0,%esp
0xff, 0x71, 0xfc,                    // 100097: pushl -0x4(%ecx)
0x55,                                // 10009a: push %ebp
0x89, 0xe5,                          // 10009b: mov %esp,%ebp
0x53,                                // 10009d: push %ebx
0x51,                                // 10009e: push %ecx
0x31, 0xdb,                          // 10009f: xor %ebx,%ebx
0xeb, 0x35,                          // 1000a1: jmp 1000d8 <main+0x48>

0x90,                                // 1000a3: nop
0x3d, 0x2c, 0x01, 0x00, 0x00,        // 1000a4: cmp $0x12c,%eax
0x7f, 0x41,                          // 1000a9: jg 1000ec <main+0x5c>
0x83, 0xf8, 0x64,                    // 1000ab: cmp $0x64,%eax
0x7e, 0x44,                          // 1000ae: jle 1000f4 <main+0x64>
0xb8, 0x4b, 0x00, 0x00, 0x00,        // 1000b0: mov $0x4b,%eax
0x8d, 0x76, 0x00,                    // 1000b5: lea 0x0(%esi),%esi
0x83, 0xec, 0x0c,                    // 1000b8: sub $0xc,%esp
0x39, 0x83, 0x20, 0x01, 0x10, 0x00,  // 1000bb: cmp %eax,0x100120(%ebx)
0x0f, 0x94, 0xc0,                    // 1000c1: sete %al
0x0f, 0xb6, 0xc0,                    // 1000c4: movzbl %al,%eax
0x50,                                // 1000c7: push %eax
0xe8, 0x5f, 0xff, 0xff, 0xff,        // 1000c8: call 10002c <nemu_assert>
0x83, 0xc3, 0x04,                    // 1000cd: add $0x4,%ebx
0x83, 0xc4, 0x10,                    // 1000d0: add $0x10,%esp
0x83, 0xfb, 0x38,                    // 1000d3: cmp $0x38,%ebx
0x74, 0x28,                          // 1000d6: je 100100 <main+0x70>

0x8b, 0x83, 0x60, 0x01, 0x10, 0x00,  // 1000d8: mov 0x100160(%ebx),%eax
0x3d, 0xf4, 0x01, 0x00, 0x00,        // 1000de: cmp $0x1f4,%eax
0x7e, 0xbf,                          // 1000e3: jle 1000a4 <main+0x14>
0xb8, 0x96, 0x00, 0x00, 0x00,        // 1000e5: mov $0x96,%eax
0xeb, 0xcc,                          // 1000ea: jmp 1000b8 <main+0x28>
0xb8, 0x64, 0x00, 0x00, 0x00,        // 1000ec: mov $0x64,%eax
0xeb, 0xc5,                          // 1000f1: jmp 1000b8 <main+0x28>

0x83, 0xf8, 0x32,                    // 1000f4: cmp $0x32,%eax
0x7e, 0x1d,                          // 1000f7: jle 100116 <main+0x86>
0xb8, 0x32, 0x00, 0x00, 0x00,        // 1000f9: mov $0x32,%eax
0xeb, 0xb8,                          // 1000fe: jmp 1000b8 <main+0x28>

0x83, 0xec, 0x0c,                    // 100100: sub $0xc,%esp
0x6a, 0x01,                          // 100103: push $0x1
0xe8, 0x22, 0xff, 0xff, 0xff,        // 100105: call 10002c <nemu_assert>
0x31, 0xc0,                          // 10010a: xor %eax,%eax
0x8d, 0x65, 0xf8,                    // 10010c: lea -0x8(%ebp),%esp
0x59,                                // 10010f: pop %ecx
0x5b,                                // 100110: pop %ebx
0x5d,                                // 100111: pop %ebp
0x8d, 0x61, 0xfc,                    // 100112: lea -0x4(%ecx),%esp
0xc3,                                // 100115: ret

0x31, 0xc0,                          // 100116: xor %eax,%eax
0xeb, 0x9e,                          // 100118: jmp 1000b8 <main+0x28>
                              // 100026:  nemu_trap
  };


  Log("No image is given. Use the default build-in image.");

  memcpy(guest_to_host(ENTRY_START), img, sizeof(img));

  return sizeof(img);
}

static inline void load_img() {
  long size;
  if (img_file == NULL) {
    size = load_default_img();
  }
  else {
    int ret;

    FILE *fp = fopen(img_file, "rb");
    Assert(fp, "Can not open '%s'", img_file);

    Log("The image is %s", img_file);

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);

    fseek(fp, 0, SEEK_SET);
    ret = fread(guest_to_host(ENTRY_START), size, 1, fp);
    assert(ret == 1);

    fclose(fp);
  }

#ifdef DIFF_TEST
  gdb_memcpy_to_qemu(ENTRY_START, guest_to_host(ENTRY_START), size);
#endif
}

static inline void restart() {
  /* Set the initial instruction pointer. */
  cpu.eip = ENTRY_START;
  //cpu.eflags=0x2;
  cpu.ZF=0;
  cpu.SF=0;
  cpu.OF=0;
  cpu.CF=0;
#ifdef DIFF_TEST
  init_qemu_reg();
#endif
}

static inline void parse_args(int argc, char *argv[]) {
  int o;
  while ( (o = getopt(argc, argv, "-bl:")) != -1) {
    switch (o) {
      case 'b': is_batch_mode = true; break;
      case 'l': log_file = optarg; break;
      case 1:
                if (img_file != NULL) Log("too much argument '%s', ignored", optarg);
                else img_file = optarg;
                break;
      default:
                panic("Usage: %s [-b] [-l log_file] [img_file]", argv[0]);
    }
  }
}

int init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Open the log file. */
  init_log();

  /* Test the implementation of the `CPU_state' structure. */
  reg_test();//test the regs

#ifdef DIFF_TEST
  /* Fork a child process to perform differential testing. */
  init_difftest();
#endif

  /* Load the image to memory. */
  load_img();

  /* Initialize this virtual computer system. */
  restart();

  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();

  /* Initialize devices. */
  init_device();

  /* Display welcome message. */
  welcome();

  return is_batch_mode;
}
