project(MultiTaskingDemo C)
cmake_minimum_required( VERSION 3.9 )

set( BUILD_PLATFORM "arm" CACHE STRING "Architecture to build for." )
set( OPT_LEVEL "3" CACHE STRING "Compiler optmisation level." )
option( UBSAN "Enable very basic undefined behaviour sanitiser." OFF )

message(STATUS "BUILD_PLATFORM is ${BUILD_PLATFORM}")
message(STATUS "OPT_LEVEL is -O${OPT_LEVEL}")
message(STATUS "UBSAN is ${UBSAN}")

if( BUILD_PLATFORM STREQUAL "arm" )
  set( PREFIX       "arm-none-eabi" )
  set( PLATFORM     "-mcpu=arm926ej-s" )
  set( PLATFORM_SRC "arm926ej_s" )
  set( RAM_START    "0x00010000" )
  set( STACK_OFFSET "0" )
  set( UART_BASE    "0x101f1000" )
  set( QEMU_ARGS    "qemu-system-arm -M versatilepb" )
elseif( BUILD_PLATFORM STREQUAL "aarch64" )
  set( PREFIX       "aarch64-linux-gnu" )
  set( PLATFORM     "" )
  set( PLATFORM_SRC "aarch64_virt" )
  set( RAM_START    "0x40000000" )
  set( STACK_OFFSET "0" )
  set( UART_BASE    "0x09000000" )
  set( QEMU_ARGS    "qemu-system-aarch64 -M virt -cpu cortex-a57" )
elseif( BUILD_PLATFORM STREQUAL "thumb" )
  set( PREFIX       "arm-none-eabi" )
  set( PLATFORM     "-mthumb -mcpu=cortex-m4" )
  set( PLATFORM_SRC "thumb_lm3s6965evb" )
  set( RAM_START    "0x00000000" )
  set( STACK_OFFSET "0x20000000" )
  set( UART_BASE    "0x4000C000" )
  set( QEMU_ARGS    "qemu-system-arm -M lm3s6965evb -cpu cortex-m4" )
endif()

set( CMAKE_C_COMPILER "${PREFIX}-gcc" )

set( QEMU "${QEMU_ARGS} -m 64M -nographic -semihosting -kernel" )

set( CFLAGS "-Wall -nostdlib -g -O${OPT_LEVEL} -std=gnu11 -DUART_BASE=${UART_BASE}" )
if(UBSAN)
  set( CFLAGS "${CFLAGS} -fsanitize=undefined" )
endif(UBSAN)
set( CMAKE_C_FLAGS "${PLATFORM} ${CFLAGS}" )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
set_property(SOURCE src/hw/${PLATFORM_SRC}/startup.s PROPERTY LANGUAGE C)
set( BASE_SOURCES
  src/thread.c
  src/util.c
  src/print.c
  src/hw/${PLATFORM_SRC}/yield.c
  src/hw/${PLATFORM_SRC}/semihosting.c
  src/hw/${PLATFORM_SRC}/startup.s
)

if(UBSAN)
  set( BASE_SOURCES ${BASE_SOURCES} src/ubsan.c )
endif(UBSAN)

add_library(support OBJECT ${BASE_SOURCES})

function(add_demo_app NAME)
        add_executable ( ${NAME} demos/${NAME}.c $<TARGET_OBJECTS:support> )
        target_link_libraries(${NAME} PRIVATE "-Wl,-T,linker.ld,-defsym=ram_start=${RAM_START},-defsym=stack_offset=${STACK_OFFSET}")

        add_custom_command(TARGET ${NAME} PRE_BUILD
          COMMAND eval "${PREFIX}-gcc --version | head -n 1"
          VERBATIM)

        add_custom_target(run_${NAME})
        add_dependencies(run_${NAME} ${NAME})
        add_custom_command(TARGET run_${NAME} POST_BUILD
          COMMAND eval "${QEMU} ${NAME}"
          VERBATIM)

        add_custom_target(debug_${NAME})
        add_dependencies(debug_${NAME} ${NAME})
        add_custom_command(TARGET debug_${NAME} POST_BUILD
          COMMAND eval "${QEMU} ${NAME} -s -S"
          VERBATIM)

        # This could be done with add_test, but then we wouldn't see the failure output.
        add_custom_target(test_${NAME} ALL)
        add_dependencies(test_${NAME} ${NAME})
        add_custom_command(TARGET test_${NAME} POST_BUILD
          # Why eval? Well, I've spent way too much time trying to get this argument substitution to work.
          COMMAND eval "${QEMU} ${NAME} -serial file:${NAME}_got.log > /dev/null 2>&1"
          COMMAND diff demos/${NAME}_expected.log ${NAME}_got.log
          VERBATIM)

endfunction(add_demo_app)

add_demo_app(yielding)
add_demo_app(message)
add_demo_app(exit)
