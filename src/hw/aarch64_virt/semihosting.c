void qemu_exit(void) {
  asm volatile (
    "mov x1, #0x26\n\t"        // 0x20026 == ADP_Stopped_ApplicationExit
    "movk x1, #2, lsl #16\n\t"
    "str x1, [sp,#0]\n\t"
    "mov x0, #0\n\t"           // Exit status code 0
    "str x0, [sp,#8]\n\t"
    "mov x1, sp\n\t"           // address of parameter block (unused here)
    "mov w0, #0x18\n\t"        // SYS_EXIT
    "svc 0x3333\n\t"           // ask monitor to do semihosting call
  );
}

void qemu_print(const char* msg) {
  asm volatile (
    "mov x1, %[msg]\n\t"       // address of message
    "mov w0, #0x04\n\t"        // SYS_WRITE0
    "svc 0x3333\n\t"           // ask monitor to do semihosting call
  :
  : [msg]"r"(msg)
  );
}
