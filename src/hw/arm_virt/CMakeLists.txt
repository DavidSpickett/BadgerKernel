set( KERNEL_SOURCES ${KERNEL_SOURCES}
  ${CMAKE_SOURCE_DIR}/src/hw/arm_common/port.c
  ${CMAKE_SOURCE_DIR}/src/user/emutls.c
  ${CMAKE_SOURCE_DIR}/src/hw/arm_common/gic.c
  ${CMAKE_SOURCE_DIR}/src/hw/driver/virt_serial.c
  ${CMAKE_SOURCE_DIR}/src/hw/arm_virt/port.c
  ${CMAKE_SOURCE_DIR}/src/hw/arm_virt/startup.s
  ${CMAKE_SOURCE_DIR}/src/hw/arm_virt/vectors.s
  ${CMAKE_SOURCE_DIR}/src/hw/arm_virt/yield.S
)

set( PREFIX        "arm-none-eabi-" )
set( PLATFORM      "-mcpu=cortex-a15" )
set( LINKER_SCRIPT "kernel_virt.ld" )
set( QEMU          "qemu-system-arm -M virt -cpu cortex-a15 -m 1M -nographic -semihosting -device loader,file=" )
