void qemu_exit(void) {
  asm volatile (
      "ldr r0, =0x18\n\t"    // angel_SWIreason_ReportException
      "ldr r1, =0x20026\n\t" // ADP_Stopped_ApplicationExit
      "svc 0x00123456\n\t"   // make semihosting call
  );
}

void qemu_print(const char* msg) {
  asm volatile (
      "mov r1, %[msg]\n\t"   // pointer to msg
      "ldr r0, =0x04\n\t"    // SYS_WRITE0
      "svc 0x00123456\n\t"   // make semihosting call
    :
    : [msg]"r"(msg)
  );
}
