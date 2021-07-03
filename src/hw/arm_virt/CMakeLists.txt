set( KERNEL_SOURCES ${KERNEL_SOURCES}
  ${CMAKE_SOURCE_DIR}/src/hw/arm_common/port.c
  ${CMAKE_SOURCE_DIR}/src/user/emutls.c
  ${CMAKE_SOURCE_DIR}/src/hw/arm_common/gic.c
  ${CMAKE_SOURCE_DIR}/src/hw/driver/virt_serial.c)

set( PREFIX        "arm-none-eabi-" )
set( PLATFORM      "-mcpu=cortex-a15" )
set( PLATFORM_SRC  "arm_virt" )
set( LINKER_SCRIPT "kernel_virt.ld" )
set( QEMU          "qemu-system-arm -M virt -cpu cortex-a15 -m 1M -nographic -semihosting -device loader,file=" )
