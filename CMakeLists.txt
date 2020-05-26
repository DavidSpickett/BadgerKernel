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
message(STATUS "BUILD_PLATFORM is ${BP_LOWER}")
message(STATUS "OPT_LEVEL is -O${OPT_LEVEL}")
message(STATUS "SANITIZERS are ${SANITIZERS}")
message(STATUS "LTO is ${LTO}")
message(STATUS "COVERAGE is ${COVERAGE}")
message(STATUS "COMPILER is ${COMPILER}")

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
endif()

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

set( BASE_SOURCES
  src/thread.c
  src/mutex.c
  src/condition_variable.c
)

if(LINUX)
  set( BASE_SOURCES ${BASE_SOURCES}
    src/hw/linux/main.c
  )
  if(CLANG)
    set( BASE_SOURCES ${BASE_SOURCES} src/emutls.c)
  endif()
else()
  set( BASE_SOURCES ${BASE_SOURCES}
    src/hw/${PLATFORM_SRC}/vectors.s
    src/hw/${PLATFORM_SRC}/startup.s
    src/hw/${PLATFORM_SRC}/yield.S
    src/hw/arm_util.c
    src/print.c
    src/alloc.c
  )

  if(NOT BP_LOWER STREQUAL "aarch64")
    set( BASE_SOURCES ${BASE_SOURCES} src/emutls.c)
  endif()

  if(BP_LOWER STREQUAL "thumb" )
    set( BASE_SOURCES ${BASE_SOURCES} src/hw/${PLATFORM_SRC}/timer.c)
  else()
    set( BASE_SOURCES ${BASE_SOURCES} src/hw/arm_timer.c)
  endif()

  set_property(SOURCE src/hw/${PLATFORM_SRC}/startup.s PROPERTY LANGUAGE C)
  set_property(SOURCE src/hw/${PLATFORM_SRC}/yield.S   PROPERTY LANGUAGE C)
  set_property(SOURCE src/hw/${PLATFORM_SRC}/vectors.s PROPERTY LANGUAGE C)

  if(SANITIZERS)
    set( BASE_SOURCES ${BASE_SOURCES} src/ubsan.c )
  endif()

  if(NOT BP_LOWER STREQUAL "thumb")
    set( BASE_SOURCES ${BASE_SOURCES} src/hw/arm_gic.c )
  endif()
endif()

add_library(support OBJECT ${BASE_SOURCES})

# Seperate build/test targets for CI
add_custom_target(make_demos)
add_custom_target(test_demos)

function(add_demo_app NAME)
        add_executable ( ${NAME} demos/${NAME}.c $<TARGET_OBJECTS:support> )

        if(NOT LINUX)
          target_link_libraries(${NAME} PRIVATE "-Wl,-T,linker.ld,-defsym=ram_start=${RAM_START},-defsym=ram_size=${RAM_SIZE},-lgcc,-lc,-N,--build-id=none")
        endif()

        if (NAME STREQUAL "filesystem")
          # Use in memory file system
          target_sources(${NAME} PRIVATE src/file_system.c)
          target_compile_definitions(${NAME} PRIVATE USE_FS=1)
        else()
          # Otherwise use semihosting or standard Posix
          if(NOT LINUX)
            target_sources(${NAME} PRIVATE src/hw/arm_file.c)
          endif()
        endif()

        add_custom_command(TARGET ${NAME} PRE_BUILD
          COMMAND eval "${CMAKE_C_COMPILER} --version | head -n 1"
          VERBATIM)
        add_dependencies(make_demos ${NAME})

        add_custom_target(run_${NAME})
        add_dependencies(run_${NAME} ${NAME})

        if(LINUX)
          add_custom_command(TARGET run_${NAME} POST_BUILD
            COMMAND eval "./${NAME}"
            VERBATIM)
        else()
           add_custom_command(TARGET run_${NAME} POST_BUILD
             COMMAND eval "${QEMU} ${NAME}"
            VERBATIM)
        endif()

        add_custom_target(debug_${NAME})
        add_dependencies(debug_${NAME} ${NAME})

        if(LINUX)
          add_custom_command(TARGET debug_${NAME} POST_BUILD
            COMMAND eval "gdb ./${NAME}"
            VERBATIM)
        else()
          add_custom_command(TARGET debug_${NAME} POST_BUILD
            COMMAND eval "${QEMU} ${NAME} -s -S"
            VERBATIM)
        endif()

        # This could be done with add_test, but then we wouldn't see the failure output.
        add_custom_target(test_${NAME} ALL)
        add_dependencies(test_${NAME} ${NAME})

        if(LINUX)
          add_custom_command(TARGET test_${NAME} POST_BUILD
            COMMAND eval "./${NAME} > ${NAME}_got.log"
            COMMAND diff demos/${NAME}_expected.log ${NAME}_got.log
            VERBATIM)
        else()
          add_custom_command(TARGET test_${NAME} POST_BUILD
            # Why eval? Well, I've spent way too much time trying to get this argument substitution to work.
            # There's a few ways to fail here.
            # If we exit(1) from Qemu, show the serial output and exit(1) again
            # (e.g. we get a UBSAN failure)
            # If Qemu runs successfully then diff the serial output and expected output
            COMMAND eval "${QEMU} ${NAME} -serial file:${NAME}_got.log > /dev/null 2>&1 || (cat ${NAME}_got.log && exit 1)"
            COMMAND diff demos/${NAME}_expected.log ${NAME}_got.log
            VERBATIM)
        endif()

        add_dependencies(test_demos test_${NAME})

endfunction(add_demo_app)

add_demo_app(yielding)
add_demo_app(exyielding)
add_demo_app(message)
add_demo_app(exit)
add_demo_app(spawn)
add_demo_app(args)
add_demo_app(mutex)
add_demo_app(conditionvariables)
add_demo_app(cancel)
add_demo_app(printthreadname)
add_demo_app(file)
add_demo_app(filesystem)

# For the random demo generator
add_demo_app(generated)

if(NOT BP_LOWER STREQUAL "aarch64" OR (LINUX AND CLANG))
  add_demo_app(threadlocalstorage)
endif()

if(NOT LINUX)
  add_demo_app(stackcheck)
  add_demo_app(selfyield)
  add_demo_app(alloc)
  add_demo_app(timer)
endif()
