project(MultiTaskingDemo C)
cmake_minimum_required( VERSION 3.7 )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set( BUILD_PLATFORM "arm" CACHE STRING "Architecture to build for." )
set( OPT_LEVEL "3" CACHE STRING "Compiler optmisation level." )
set( COMPILER "gcc" CACHE STRING "Name of compiler to use." )
option( SANITIZERS "Enable UBSAN/ASAN (ASAN Linux only)." OFF )
option( COVERAGE "Generate coverage data. (Linux only)" OFF )
option( LTO "Enable link time optimisation." OFF )
set( STACK_SIZE   1 ) # In KB
set( RAM_SIZE 0x100000 ) # 1MB in bytes

string( TOLOWER "${BUILD_PLATFORM}" BP_LOWER )

# Match to catch clang/clang-XYZ
if (COMPILER MATCHES "clang")
  set( CLANG ON)
endif()

if (BP_LOWER STREQUAL "linux")
  set( LINUX ON )
elseif( BP_LOWER STREQUAL "arm" )
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
  set( STACK_SIZE 2 )
  set( RAM_START    "0x40000000" )
  set( UART_BASE    "0x09000000" )
  set( QEMU_CMD     "qemu-system-aarch64 -M virt -cpu cortex-a57" )
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
  Expected one of \"linux\", \"arm\", \"thumb\", \"aarch64\".")
endif()

message(STATUS "BUILD_PLATFORM is ${BP_LOWER}")
message(STATUS "OPT_LEVEL is -O${OPT_LEVEL}")
message(STATUS "SANITIZERS are ${SANITIZERS}")
message(STATUS "LTO is ${LTO}")
message(STATUS "COVERAGE is ${COVERAGE}")
message(STATUS "COMPILER is ${COMPILER}")
if (NOT LINUX)
  message(STATUS "STACK_SIZE is ${STACK_SIZE}KB")
endif()

# Note that anything but GCC isn't going to work for bare metal
set( CMAKE_C_COMPILER "${PREFIX}${COMPILER}" )

# Ignore attr error so we can use oclint annotations
set( CFLAGS "-DSTACK_SIZE=${STACK_SIZE} -g3 -O${OPT_LEVEL} -Wall -Werror -Wno-error=attributes -std=gnu11")

if (LINUX)
  set( CFLAGS "${CFLAGS} -pthread" )
  if (CLANG)
    set( CFLAGS "${CFLAGS} -femulated-tls" )
  endif()
else()
  # Note that the thumb board ignores -M
  set( QEMU "${QEMU_CMD} -m 1M -nographic -semihosting -kernel" )
  set( CFLAGS "${CFLAGS} -ffreestanding -nostdlib -DUART_BASE=${UART_BASE}" )
endif()

if(LTO)
  set( CFLAGS "${CFLAGS} -flto -ffunction-sections" )
endif()

if(SANITIZERS)
  set( CFLAGS "${CFLAGS} -fsanitize=undefined" )
  if (LINUX)
    # Failing is the default on bare metal, but it doesn't recognise the option
    set( CFLAGS "${CFLAGS} -fno-sanitize-recover=all -fsanitize=address")
  endif()
endif(SANITIZERS)

if(COVERAGE)
  if(LINUX)
    set( CFLAGS "${CFLAGS} --coverage " )
  else()
    message(FATAL_ERROR "Coverage only available on Linux.")
  endif()
endif()

set( CMAKE_C_FLAGS "${PLATFORM} ${CFLAGS}" )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set( KERNEL_SOURCES
  src/thread.c
  src/mutex.c
  src/condition_variable.c
)

if(LINUX)
  set( KERNEL_SOURCES ${KERNEL_SOURCES}
    src/hw/linux/main.c
  )
  if(CLANG)
    set( KERNEL_SOURCES ${KERNEL_SOURCES} src/emutls.c)
  endif()
else()
  set( KERNEL_SOURCES ${KERNEL_SOURCES}
    src/hw/${PLATFORM_SRC}/vectors.s
    src/hw/${PLATFORM_SRC}/startup.s
    src/hw/${PLATFORM_SRC}/yield.S
    src/hw/arm_util.c
    src/print.c
    src/alloc.c
  )

  if(NOT BP_LOWER STREQUAL "aarch64")
    set( KERNEL_SOURCES ${KERNEL_SOURCES} src/emutls.c)
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
endif()

add_library(kernel OBJECT ${KERNEL_SOURCES})

# Convenent build target for CI
add_custom_target(make_demos)

include(AddDemo.cmake)

add_demo(yielding)
add_demo(exyielding)
add_demo(message)
add_demo(exit)
add_demo(spawn)
add_demo(args)
add_demo(mutex)
add_demo(conditionvariables)
add_demo(cancel)
add_demo(printthreadname)
add_demo(file)
add_demo(filesystem)
add_demo(generated)

if(NOT BP_LOWER STREQUAL "aarch64" OR (LINUX AND CLANG))
  add_demo(threadlocalstorage)
endif()

if(NOT LINUX)
  add_demo(stackcheck)
  add_demo(selfyield)
  add_demo(alloc)
  add_demo(timer)
endif()
