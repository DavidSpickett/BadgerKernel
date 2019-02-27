void qemu_exit() {
  asm volatile (
      "ldr r0, =0x18\n\t"    // angel_SWIreason_ReportException
      "ldr r1, =0x20026\n\t" // ADP_Stopped_ApplicationExit
      "svc 0xAB\n\t"         // have monitor make semihosting call
  );
}

void qemu_print(const char* msg) {
  asm volatile (
      "mov r1, %[msg]\n\t" // pointer to msg
      "ldr r0, =0x04\n\t"  // SYS_WRITE0
      "svc 0xAB\n\t"       // have monitor make semihosting call
    :
    : [msg]"r"(msg)
  );
}

// GDB helper to get current Cortex-M privilege level
// Yes, 1 means unprivileged. I know, weird right?
enum plevel { privileged, unprivileged };
enum plevel pl() {
  enum plevel level;
  asm volatile(
      "mrs r5, control\n\t"
      "mov r6, #1\n\t"
      "and r5, r5, r6\n\t"
      "mov %0, r5\n\t"
  : "=r"(level)
  :
  : "r5", "r6"
  );
  return level;
}
