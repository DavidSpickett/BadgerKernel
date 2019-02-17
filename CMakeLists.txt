project(demo C)
cmake_minimum_required( VERSION 3.10 )

set( BUILD_PLATFORM "arm" CACHE STRING "Architecture to build for." )

if( BUILD_PLATFORM STREQUAL "arm" )
  set( PREFIX       "arm-none-eabi" )
  set( PLATFORM     "-mcpu=arm926ej-s" )
  set( PLATFORM_SRC "arm926ej_s" )
  set( RAM_START    "0x00010000" )
  set( QEMU_ARGS    "qemu-system-arm -M versatilepb" )
elseif( BUILD_PLATFORM STREQUAL "aarch64" )
  set( PREFIX       "aarch64-linux-gnu" )
  set( PLATFORM     "" )
  set( PLATFORM_SRC "aarch64_virt" )
  set( RAM_START    "0x40000000" )
  set( QEMU_ARGS    "qemu-system-aarch64 -M virt -cpu cortex-a57" )
endif()

set( CMAKE_C_COMPILER "${PREFIX}-gcc" )

set( QEMU "${QEMU_ARGS} -m 64M -nographic -semihosting -kernel build/demo" )
set( INC "-I include/" )
set( CFLAGS "-Wall -nostdlib -g -O3" )
set( CMAKE_C_FLAGS "${INC} ${PLATFORM} ${CFLAGS}" )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build" )

set_property(SOURCE src/hw/${PLATFORM_SRC}/startup.s PROPERTY LANGUAGE C)
set( SOURCES
  main.c
  src/thread.c
  src/hw/${PLATFORM_SRC}/yield.c
  src/hw/${PLATFORM_SRC}/print.c
  src/hw/${PLATFORM_SRC}/exit.c
  src/hw/${PLATFORM_SRC}/startup.s
)

add_executable ( ${PROJECT_NAME} ${SOURCES} )
target_link_libraries(demo PRIVATE "-Wl,-T,linker.ld,-defsym=ram_start=${RAM_START}" )

file( WRITE build/run.sh ${QEMU} )
add_custom_target(run COMMAND build/run.sh
  DEPENDS demo)
