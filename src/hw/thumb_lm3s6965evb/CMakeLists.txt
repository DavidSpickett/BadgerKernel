set( KERNEL_SOURCES ${KERNEL_SOURCES}
  ${CMAKE_SOURCE_DIR}/src/hw/arm_common/port.c
  ${CMAKE_SOURCE_DIR}/src/user/emutls.c
  ${CMAKE_SOURCE_DIR}/src/hw/thumb_lm3s6965evb/timer.c
  ${CMAKE_SOURCE_DIR}/src/hw/thumb_lm3s6965evb/port.c
  ${CMAKE_SOURCE_DIR}/src/hw/driver/virt_serial.c)

set( PREFIX        "arm-none-eabi-" )
set( PLATFORM      "-mthumb -mcpu=cortex-m4" )
set( PLATFORM_SRC  "thumb_lm3s6965evb" )
set( LINKER_SCRIPT "kernel_lm3s6965evb.ld" )
# As of QEMU 2.11.1 the thumb board requires -kernel
# This is known to be fixed in at least 5.1.0
# (note that the thumb board ignores -M)
set( QEMU "qemu-system-arm -M lm3s6965evb -cpu cortex-m4 -m 1M -nographic -semihosting -kernel " )
