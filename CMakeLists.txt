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

string( TOLOWER "${BUILD_PLATFORM}" BP_LOWER )

if(CCACHE)
  find_program(CCACHE_FOUND ccache)
  if(NOT CCACHE_FOUND)
    message(FATAL_ERROR "ccache not found!")
  endif()
  message(STATUS "ccache is enabled")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

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

if( BP_LOWER STREQUAL "arm" )
  include(src/hw/arm_virt/CMakeLists.txt)
elseif( BP_LOWER STREQUAL "aarch64" )
  include(src/hw/aarch64_virt/CMakeLists.txt)
elseif( BP_LOWER STREQUAL "thumb" )
  include(src/hw/thumb_lm3s6965evb/CMakeLists.txt)
else()
  message(FATAL_ERROR "Invalid platform \"${BP_LOWER}\". \
  Expected one of \"arm\", \"thumb\", \"aarch64\".")
endif()

set(KERNEL_SOURCES ${KERNEL_SOURCES}
  # TODO: this file should be optional
  ${CMAKE_SOURCE_DIR}/src/hw/${PLATFORM_SRC}/vectors.s
  ${CMAKE_SOURCE_DIR}/src/hw/${PLATFORM_SRC}/startup.s
  ${CMAKE_SOURCE_DIR}/src/hw/${PLATFORM_SRC}/yield.S
  ${CMAKE_SOURCE_DIR}/src/hw/${PLATFORM_SRC}/port.c
)

set( CMAKE_C_COMPILER "${PREFIX}gcc" )
set( CMAKE_CXX_COMPILER "${PREFIX}g++" )

set( CFLAGS "-DSTACK_SIZE=${STACK_SIZE} -g3 -O${OPT_LEVEL} -Wall -Werror -Wextra")

# Ignore lto type mismatch error which triggers at O0 for UBSAN handlers
# I *think*, due to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=78562#c6
if(SANITIZERS AND LTO)
  set( CFLAGS "${CFLAGS} -Wno-error=lto-type-mismatch")
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

message(STATUS "BUILD_PLATFORM is ${BP_LOWER}")
message(STATUS "OPT_LEVEL is -O${OPT_LEVEL}")
message(STATUS "SANITIZERS are ${SANITIZERS}")
message(STATUS "LTO is ${LTO}")
message(STATUS "COVERAGE is ${COVERAGE}")
message(STATUS "STACK_SIZE is ${STACK_SIZE}KB")
message(STATUS "SEMIHOSTING is ${SEMIHOSTING}")
message(STATUS "LINKER_SCRIPT is ${LINKER_SCRIPT}")

# Convenient build target for CI
add_custom_target(make_demos)

include(cmake/AddDemo.cmake)
include(cmake/AddLoadable.cmake)

add_subdirectory(demos)
