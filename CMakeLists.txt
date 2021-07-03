project(MultiTaskingDemo)
cmake_minimum_required( VERSION 3.7 )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( BUILD_PLATFORM "arm" CACHE STRING "Architecture to build for." )
set( OPT_LEVEL "3" CACHE STRING "Compiler optmisation level." )
option( SANITIZERS "Enable UBSAN" OFF )
option( LTO "Enable link time optimisation." OFF )
option( CCACHE "Enable ccache." ON )
set( SEMIHOSTING ON )
set( STACK_SIZE 2 ) # In KB
set( SERIAL_DRIVER "virt_serial.c" )

string( TOLOWER "${BUILD_PLATFORM}" BP_LOWER )

if( BP_LOWER STREQUAL "arm" )
  set( PREFIX       "arm-none-eabi-" )
  set( PLATFORM     "-mcpu=cortex-a15" )
  set( PLATFORM_SRC "arm_virt" )
  set( QEMU_CMD     "qemu-system-arm -M virt -cpu cortex-a15" )
  set( LINKER_SCRIPT "kernel_virt.ld" )
elseif( BP_LOWER STREQUAL "aarch64" )
  set( PREFIX       "aarch64-none-elf-" )
  # Don't generate NEON
  set( PLATFORM     "-mcpu=cortex-a57 -mgeneral-regs-only" )
  set( PLATFORM_SRC "aarch64_virt" )
  set( QEMU_CMD     "qemu-system-aarch64 -M virt -cpu cortex-a57" )
  # O3 LTO UBSAN trace demo needs some extra
  set( STACK_SIZE 3 )
  set( LINKER_SCRIPT "kernel_virt.ld" )
elseif( BP_LOWER STREQUAL "thumb" )
  set( PREFIX       "arm-none-eabi-" )
  set( PLATFORM     "-mthumb -mcpu=cortex-m4" )
  set( PLATFORM_SRC "thumb_lm3s6965evb" )
  set( QEMU_CMD     "qemu-system-arm -M lm3s6965evb -cpu cortex-m4" )
  set( LINKER_SCRIPT "kernel_lm3s6965evb.ld" )
else()
  message(FATAL_ERROR "Invalid platform \"${BP_LOWER}\". \
  Expected one of \"arm\", \"thumb\", \"aarch64\".")
endif()

message(STATUS "BUILD_PLATFORM is ${BP_LOWER}")
message(STATUS "OPT_LEVEL is -O${OPT_LEVEL}")
message(STATUS "SANITIZERS are ${SANITIZERS}")
message(STATUS "LTO is ${LTO}")
message(STATUS "COVERAGE is ${COVERAGE}")
message(STATUS "STACK_SIZE is ${STACK_SIZE}KB")
message(STATUS "SEMIHOSTING is ${SEMIHOSTING}")
message(STATUS "LINKER_SCRIPT is ${LINKER_SCRIPT}")
message(STATUS "SERIAL_DRIVER is ${SERIAL_DRIVER}")

if(CCACHE)
  find_program(CCACHE_FOUND ccache)
  if(NOT CCACHE_FOUND)
    message(FATAL_ERROR "ccache not found!")
  endif()
  message(STATUS "ccache is enabled")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

set( CMAKE_C_COMPILER "${PREFIX}gcc" )
set( CMAKE_CXX_COMPILER "${PREFIX}g++" )

set( CFLAGS "-DSTACK_SIZE=${STACK_SIZE} -g3 -O${OPT_LEVEL} -Wall -Werror -Wextra")

# Ignore lto type mismatch error which triggers at O0 for UBSAN handlers
# I *think*, due to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=78562#c6
if(SANITIZERS AND LTO)
  set( CFLAGS "${CFLAGS} -Wno-error=lto-type-mismatch")
endif()

# Note that the thumb board ignores -M
set( QEMU "${QEMU_CMD} -m 1M -nographic -semihosting" )
if( BP_LOWER STREQUAL "thumb" )
  # As of QEMU 2.11.1 the thumb board requires -kernel
  # This is known to be fixed in at least 5.1.0
  set( QEMU "${QEMU} -kernel ")
else()
  set( QEMU "${QEMU} -device loader,file=")
endif()

# -fno-common to disallow variables being present in multiple compliaton units
set( CFLAGS "${CFLAGS} -ffreestanding -nostdlib -fno-common" )
add_definitions( -DSRC_ROOT=\"${CMAKE_SOURCE_DIR}\" )

if(LTO)
  set( CFLAGS "${CFLAGS} -flto -ffunction-sections" )
endif()

if(SANITIZERS)
  set( CFLAGS "${CFLAGS} -fsanitize=undefined" )
endif(SANITIZERS)

if(SEMIHOSTING)
  set( CFLAGS "${CFLAGS} -DSEMIHOSTING_ENABLED" )
endif(SEMIHOSTING)

set( CMAKE_C_FLAGS "${PLATFORM} ${CFLAGS} -std=gnu11" )
set( CMAKE_CXX_FLAGS "${PLATFORM} ${CFLAGS} -fno-rtti -fno-exceptions -fno-unwind-tables -std=c++11" )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set( KERNEL_SOURCES
  ${CMAKE_SOURCE_DIR}/src/kernel/thread.c
  ${CMAKE_SOURCE_DIR}/src/kernel/thread_property.cpp
  ${CMAKE_SOURCE_DIR}/src/kernel/mutex.c
  ${CMAKE_SOURCE_DIR}/src/kernel/condition_variable.c
  ${CMAKE_SOURCE_DIR}/src/kernel/signal_handling.c
  ${CMAKE_SOURCE_DIR}/src/kernel/alloc.c
  ${CMAKE_SOURCE_DIR}/src/kernel/syscall.c
  ${CMAKE_SOURCE_DIR}/src/kernel/message.c
  ${CMAKE_SOURCE_DIR}/src/kernel/semihosting.c
  ${CMAKE_SOURCE_DIR}/src/kernel/file.c

  ${CMAKE_SOURCE_DIR}/src/hw/arm_common/port.c

  ${CMAKE_SOURCE_DIR}/src/hw/${PLATFORM_SRC}/vectors.s
  ${CMAKE_SOURCE_DIR}/src/hw/${PLATFORM_SRC}/startup.s
  ${CMAKE_SOURCE_DIR}/src/hw/${PLATFORM_SRC}/yield.S
  ${CMAKE_SOURCE_DIR}/src/hw/${PLATFORM_SRC}/port.c

  ${CMAKE_SOURCE_DIR}/src/hw/driver/${SERIAL_DRIVER}

  ${CMAKE_SOURCE_DIR}/src/common/print.cpp
  ${CMAKE_SOURCE_DIR}/src/common/trace.c
  ${CMAKE_SOURCE_DIR}/src/common/assert.c
  ${CMAKE_SOURCE_DIR}/src/common/errno.c

  # /user files here for threads within the same binary
  ${CMAKE_SOURCE_DIR}/src/user/errno.c
  ${CMAKE_SOURCE_DIR}/src/user/thread.c
  ${CMAKE_SOURCE_DIR}/src/user/file.c
  ${CMAKE_SOURCE_DIR}/src/user/util.c
  ${CMAKE_SOURCE_DIR}/src/user/alloc.c
  ${CMAKE_SOURCE_DIR}/src/user/mutex.c
  ${CMAKE_SOURCE_DIR}/src/user/condition_variable.c
  ${CMAKE_SOURCE_DIR}/src/user/timer.c
  ${CMAKE_SOURCE_DIR}/src/user/fiber.c
)

if(NOT BP_LOWER STREQUAL "aarch64")
  set( KERNEL_SOURCES ${KERNEL_SOURCES} ${CMAKE_SOURCE_DIR}/src/user/emutls.c)
endif()

if(BP_LOWER STREQUAL "thumb" )
  set( KERNEL_SOURCES ${KERNEL_SOURCES} ${CMAKE_SOURCE_DIR}/src/hw/${PLATFORM_SRC}/timer.c)
endif()

if(SANITIZERS)
  set( KERNEL_SOURCES ${KERNEL_SOURCES} ${CMAKE_SOURCE_DIR}/src/common/ubsan.c )
endif()

if(NOT BP_LOWER STREQUAL "thumb")
  set( KERNEL_SOURCES ${KERNEL_SOURCES} ${CMAKE_SOURCE_DIR}/src/hw/arm_common/gic.c )
endif()

# Convenient build target for CI
add_custom_target(make_demos)

include(cmake/AddDemo.cmake)
include(cmake/AddLoadable.cmake)

add_subdirectory(demos)
