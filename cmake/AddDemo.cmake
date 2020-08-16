function(add_demo NAME MAX_THREADS)
  __add_demo(${NAME} ON, ${MAX_THREADS})
endfunction()

function(add_demo_no_test NAME MAX_THREADS)
  __add_demo(${NAME} OFF ${MAX_THREADS})
endfunction()

function(__add_demo NAME TEST MAX_THREADS)
  add_executable ( ${NAME} demos/${NAME}/${NAME}.c ${KERNEL_SOURCES} )
  target_compile_definitions(${NAME} PRIVATE MAX_THREADS=${MAX_THREADS})

  target_link_libraries(${NAME} PRIVATE "-Wl,-T,linker/kernel.ld,-defsym=ram_start=${RAM_START},-defsym=ram_size=${RAM_SIZE},-lgcc,-lc,-N,--build-id=none")

  add_custom_command(TARGET ${NAME} PRE_BUILD
    COMMAND eval "${CMAKE_C_COMPILER} --version | head -n 1"
    VERBATIM)
  add_dependencies(make_demos ${NAME})

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

  if(TEST)
    # This could be done with add_test, but then we wouldn't see the failure output.
    add_custom_target(test_${NAME} ALL)
    add_dependencies(test_${NAME} ${NAME})

    add_custom_command(TARGET test_${NAME} POST_BUILD
      # Why eval? Well, I've spent way too much time trying to get this argument substitution to work.
      # There's a few ways to fail here.
      # If we exit(1) from Qemu, show the serial output and exit(1) again
      # (e.g. we get a UBSAN failure)
      # If Qemu runs successfully then diff the serial output and expected output
      COMMAND eval "${QEMU} ${NAME} -serial file:${NAME}_got.log > /dev/null 2>&1 || (cat ${NAME}_got.log && exit 1)"
      COMMAND diff demos/${NAME}/expected.log ${NAME}_got.log
      VERBATIM)
  endif(TEST)
endfunction(__add_demo)
