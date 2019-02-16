void qemu_exit() {
  register int reg0 asm("r0") = 0x18; // angel_SWIreason_ReportException
  register int reg1 asm("r1") = 0x20026; // ADP_Stopped_ApplicationExit
  (void)reg0; (void)reg1;
  asm("svc 0x00123456");  // make semihosting call
}
