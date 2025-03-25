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
const uint8_t img[] = {
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
    0x8b, 0x45, 0x0c,                    // 10004b: mov 0xc(%ebp),%eax
    0x03, 0x45, 0x08,                    // 10004e: add 0x8(%ebp),%eax
    0x5d,                                // 100051: pop %ebp
    0xc3,                                // 100052: ret
    0x90,                                // 100053: nop

    0x8d, 0x4c, 0x24, 0x04,              // 100054: lea 0x4(%esp),%ecx
    0x83, 0xe4, 0xf0,                    // 100058: and $0xfffffff0,%esp
    0xff, 0x71, 0xfc,                    // 10005b: pushl -0x4(%ecx)
    0x55,                                // 10005e: push %ebp
    0x89, 0xe5,                          // 10005f: mov %esp,%ebp
    0x57,                                // 100061: push %edi
    0x56,                                // 100062: push %esi
    0x53,                                // 100063: push %ebx
    0x51,                                // 100064: push %ecx
    0x83, 0xec, 0x08,                    // 100065: sub $0x8,%esp
    0x31, 0xff,                          // 100068: xor %edi,%edi
    0x66, 0x90,                          // 10006a: xchg %ax,%ax
    0x8d, 0x34, 0x3f,                    // 10006c: lea (%edi,%edi,1),%esi
    0x31, 0xdb,                          // 10006f: xor %ebx,%ebx
    0x8d, 0x76, 0x00,                    // 100071: lea 0x0(%esi),%esi
    0x83, 0xec, 0x0c,                    // 100074: sub $0xc,%esp
    0x8b, 0x87, 0xe0, 0x01, 0x10, 0x00,  // 100077: mov 0x1001e0(%edi),%eax
    0x03, 0x83, 0xe0, 0x01, 0x10, 0x00,  // 10007d: add 0x1001e0(%ebx),%eax
    0x3b, 0x84, 0xb3, 0xe0, 0x00, 0x10, 0x00, // 100083: cmp 0x1000e0(%ebx,%esi,4),%eax
    0x0f, 0x94, 0xc0,                    // 10008a: sete %al
    0x0f, 0xb6, 0xc0,                    // 10008d: movzbl %al,%eax
    0x50,                                // 100090: push %eax
    0xe8, 0x96, 0xff, 0xff, 0xff,        // 100091: call 10002c <nemu_assert>
    0x83, 0xc3, 0x04,                    // 100096: add $0x4,%ebx
    0x83, 0xc4, 0x10,                    // 100099: add $0x10,%esp
    0x83, 0xfb, 0x20,                    // 10009c: cmp $0x20,%ebx
    0x75, 0xd3,                          // 10009f: jne 100074 <main+0x20>
    0x83, 0xec, 0x0c,                    // 1000a1: sub $0xc,%esp
    0x6a, 0x01,                          // 1000a4: push $0x1
    0xe8, 0x81, 0xff, 0xff, 0xff,        // 1000a6: call 10002c <nemu_assert>
    0x83, 0xc7, 0x04,                    // 1000ab: add $0x4,%edi
    0x83, 0xc4, 0x10,                    // 1000ae: add $0x10,%esp
    0x83, 0xff, 0x20,                    // 1000b1: cmp $0x20,%edi
    0x75, 0xb6,                          // 1000b4: jne 10006c <main+0x18>
    0x83, 0xec, 0x0c,                    // 1000b6: sub $0xc,%esp
    0x6a, 0x01,                          // 1000b9: push $0x1
    0xe8, 0x6c, 0xff, 0xff, 0xff,        // 1000bb: call 10002c <nemu_assert>
    0x31, 0xc0,                          // 1000c0: xor %eax,%eax
    0x8d, 0x65, 0xf0,                    // 1000c2: lea -0x10(%ebp),%esp
    0x59,                                // 1000c5: pop %ecx
    0x5b,                                // 1000c6: pop %ebx
    0x5e,                                // 1000c7: pop %esi
    0x5f,                                // 1000c8: pop %edi
    0x5d,                                // 1000c9: pop %ebp
    0x8d, 0x61, 0xfc,                    // 1000ca: lea -0x4(%ecx),%esp
    0xc3,                                // 1000cd: ret
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
  cpu.eflags=0x2;
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
