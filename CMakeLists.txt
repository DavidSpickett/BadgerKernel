project(MultiTaskingDemo C)
cmake_minimum_required( VERSION 3.7 )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( BUILD_PLATFORM "arm" CACHE STRING "Architecture to build for." )
set( OPT_LEVEL "3" CACHE STRING "Compiler optmisation level." )
option( SANITIZERS "Enable UBSAN/ASAN (ASAN Linux only)." OFF )
option( LTO "Enable link time optimisation." OFF )
option( CCACHE "Enable ccache." ON )
set( STACK_SIZE 2 ) # In KB
set( RAM_SIZE 0x100000 ) # 1MB in bytes

string( TOLOWER "${BUILD_PLATFORM}" BP_LOWER )

if( BP_LOWER STREQUAL "arm" )
  set( PREFIX       "arm-none-eabi-" )
  set( PLATFORM     "-mcpu=cortex-a15" )
  set( PLATFORM_SRC "arm_virt" )
  set( RAM_START    "0x40000000" )
  set( UART_BASE    "0x09000000" )
  set( QEMU_CMD     "qemu-system-arm -M virt -cpu cortex-a15" )
elseif( BP_LOWER STREQUAL "aarch64" )
  set( PREFIX       "aarch64-elf-" )
  # Don't generate NEON
  set( PLATFORM     "-mcpu=cortex-a57 -mgeneral-regs-only" )
  set( PLATFORM_SRC "aarch64_virt" )
  set( RAM_START    "0x40000000" )
  set( UART_BASE    "0x09000000" )
  set( QEMU_CMD     "qemu-system-aarch64 -M virt -cpu cortex-a57" )
  # O3 LTO UBSAN trace demo needs some extra
  set( STACK_SIZE 3 )
elseif( BP_LOWER STREQUAL "thumb" )
  set( PREFIX       "arm-none-eabi-" )
  set( PLATFORM     "-mthumb -mcpu=cortex-m4" )
  set( PLATFORM_SRC "thumb_lm3s6965evb" )
  set( RAM_START    "0x20000000" )
  # 64k of RAM
  set( RAM_SIZE     0x10000)
  set( UART_BASE    "0x4000C000" )
  set( QEMU_CMD     "qemu-system-arm -M lm3s6965evb -cpu cortex-m4" )
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

if(CCACHE)
  find_program(CCACHE_FOUND ccache)
  if(NOT CCACHE_FOUND)
    message(FATAL_ERROR "ccache not found!")
  endif()
  message(STATUS "ccache is enabled")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

set( CMAKE_C_COMPILER "${PREFIX}gcc" )

# Ignore attr error so that we can use "naked" which
# is ignored on AArch64
set( CFLAGS "-DSTACK_SIZE=${STACK_SIZE} -g3 -O${OPT_LEVEL} -Wall -Werror -Wno-error=attributes -std=gnu11")

# Ignore lto type mismatch error which triggers at O0 for UBSAN handlers
# I *think*, due to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=78562#c6
if(SANITIZERS AND LTO)
  set( CFLAGS "${CFLAGS} -Wno-error=lto-type-mismatch")
endif()

# Note that the thumb board ignores -M
set( QEMU "${QEMU_CMD} -m 1M -nographic -semihosting -kernel" )
set( CFLAGS "${CFLAGS} -ffreestanding -nostdlib -DUART_BASE=${UART_BASE}" )

if(LTO)
  set( CFLAGS "${CFLAGS} -flto -ffunction-sections" )
endif()

if(SANITIZERS)
  set( CFLAGS "${CFLAGS} -fsanitize=undefined" )
endif(SANITIZERS)

set( CMAKE_C_FLAGS "${PLATFORM} ${CFLAGS}" )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set( KERNEL_SOURCES
  src/kernel/thread.c
  src/kernel/mutex.c
  src/kernel/condition_variable.c
  src/kernel/signal_handling.c
  src/kernel/alloc.c

  src/hw/arm_file.c

  src/hw/${PLATFORM_SRC}/vectors.s
  src/hw/${PLATFORM_SRC}/startup.s
  src/hw/${PLATFORM_SRC}/yield.S
  src/hw/arm_util.c

  src/print.c
  src/trace.c

  # /user files here for threads within the same binary
  src/user/errno.c
  src/user/thread.c
  src/user/file.c
  src/user/util.c
  src/user/alloc.c
  src/user/mutex.c
  src/user/condition_variable.c
  src/user/syscall.c
)

if(NOT BP_LOWER STREQUAL "aarch64")
  set( KERNEL_SOURCES ${KERNEL_SOURCES} src/user/emutls.c)
endif()

if(BP_LOWER STREQUAL "thumb" )
  set( KERNEL_SOURCES ${KERNEL_SOURCES} src/hw/${PLATFORM_SRC}/timer.c)
else()
  set( KERNEL_SOURCES ${KERNEL_SOURCES} src/hw/arm_timer.c)
endif()

set_property(SOURCE src/hw/${PLATFORM_SRC}/startup.s PROPERTY LANGUAGE C)
set_property(SOURCE src/hw/${PLATFORM_SRC}/yield.S   PROPERTY LANGUAGE C)
set_property(SOURCE src/hw/${PLATFORM_SRC}/vectors.s PROPERTY LANGUAGE C)

if(SANITIZERS)
  set( KERNEL_SOURCES ${KERNEL_SOURCES} src/ubsan.c )
endif()

if(NOT BP_LOWER STREQUAL "thumb")
  set( KERNEL_SOURCES ${KERNEL_SOURCES} src/hw/arm_gic.c )
endif()

# Convenient build target for CI
add_custom_target(make_demos)

include(cmake/AddDemo.cmake)
include(cmake/AddLoadable.cmake)

include(demos/yielding/CMakeLists.txt)
include(demos/exyielding/CMakeLists.txt)
include(demos/message/CMakeLists.txt)
include(demos/exit/CMakeLists.txt)
include(demos/spawn/CMakeLists.txt)
include(demos/args/CMakeLists.txt)
include(demos/mutexes/CMakeLists.txt)
include(demos/conditionvariables/CMakeLists.txt)
include(demos/cancel/CMakeLists.txt)
include(demos/printthreadname/CMakeLists.txt)
include(demos/file/CMakeLists.txt)
include(demos/generated/CMakeLists.txt)
include(demos/threadlocalstorage/CMakeLists.txt)
include(demos/stackcheck/CMakeLists.txt)
include(demos/selfyield/CMakeLists.txt)
include(demos/alloc/CMakeLists.txt)
include(demos/timer/CMakeLists.txt)
include(demos/loadbinary/CMakeLists.txt)
include(demos/loadbinaries/CMakeLists.txt)
include(demos/loadpiebinary/CMakeLists.txt)
include(demos/parentchild/CMakeLists.txt)
include(demos/permissions/CMakeLists.txt)
include(demos/trace/CMakeLists.txt)
include(demos/signalhandling/CMakeLists.txt)
include(demos/backtrace/CMakeLists.txt)
include(demos/shell/CMakeLists.txt)
