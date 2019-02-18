project(MultiTaskingDemo C)
cmake_minimum_required( VERSION 3.9 )

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

set( QEMU "${QEMU_ARGS} -m 64M -nographic -semihosting -kernel" )
set( INC "-I include/" )
set( CFLAGS "-Wall -nostdlib -g -O3 -std=gnu11" )
set( CMAKE_C_FLAGS "${INC} ${PLATFORM} ${CFLAGS}" )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build" )

set_property(SOURCE src/hw/${PLATFORM_SRC}/startup.s PROPERTY LANGUAGE C)
set( BASE_SOURCES
  src/thread.c
  src/hw/${PLATFORM_SRC}/yield.c
  src/hw/${PLATFORM_SRC}/print.c
  src/hw/${PLATFORM_SRC}/semihosting.c
  src/hw/${PLATFORM_SRC}/startup.s
)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "build")

function(add_demo_app NAME)
        add_executable ( ${NAME} ${BASE_SOURCES} demos/${NAME}.c)
        target_link_libraries(${NAME} PRIVATE "-Wl,-T,linker.ld,-defsym=ram_start=${RAM_START}" )

        # For some reason I can't run the command line directly
        file( WRITE build/run_${NAME}.sh "${QEMU} build/${NAME}" )
        add_custom_target(run_${NAME} 
          COMMAND /bin/sh build/run_${NAME}.sh
          DEPENDS ${NAME})

        # This could be done with add_test, but then we wouldn't see the failure output.
        file( WRITE build/test_${NAME}.sh "${QEMU} build/${NAME} -serial file:build/${NAME}_got.log > \
          /dev/null 2>&1 && diff demos/${NAME}_expected.log build/${NAME}_got.log" )
        add_custom_target(test_${NAME} ALL
          COMMAND /bin/sh build/test_${NAME}.sh
          DEPENDS ${NAME})
endfunction(add_demo_app)

add_demo_app(yielding)
