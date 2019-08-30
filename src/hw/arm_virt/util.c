void exit(int status) {
  asm volatile (
      "ldr r0, =0x18\n\t"    // angel_SWIreason_ReportException
      "ldr r1, =0x20026\n\t" // ADP_Stopped_ApplicationExit
      "svc 0x00123456\n\t"   // make semihosting call
  );
}
