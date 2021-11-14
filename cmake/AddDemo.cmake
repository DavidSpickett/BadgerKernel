function(add_demo NAME MAX_THREADS)
  __add_demo(${NAME} "log" ${MAX_THREADS})
endfunction()

function(add_demo_expect_test NAME MAX_THREADS)
  __add_demo(${NAME} "expect" ${MAX_THREADS})
endfunction()

function(add_demo_no_test NAME MAX_THREADS)
  __add_demo(${NAME} "none" ${MAX_THREADS})
endfunction()

function(__add_demo NAME TEST_TYPE MAX_THREADS)
  # TODO: source file properties are only visible to the current CMakeLists.txt
  # Really, we shouldn't be using this hack. Instead using the normal compiler detection
  # mechanisms.
  set( ASM_SOURCES ${KERNEL_SOURCES} )
  list( FILTER ASM_SOURCES INCLUDE REGEX ".*\.(s|S)$" )
  set_source_files_properties(${ASM_SOURCES} PROPERTIES LANGUAGE C)

  set(KERNEL_EXECUTABLE "${NAME}_kernel")

  add_executable(${KERNEL_EXECUTABLE} ${CMAKE_SOURCE_DIR}/demos/${NAME}/${NAME}.c ${KERNEL_SOURCES} )
  # Making <demo name> a custom target means we can add loadables to it later
  # which will get built by "make <demo name>"
  add_custom_target(${NAME} DEPENDS ${KERNEL_EXECUTABLE})
  add_dependencies(demos ${NAME})

  target_compile_definitions(${KERNEL_EXECUTABLE} PRIVATE MAX_THREADS=${MAX_THREADS})

  set(LINKER_SCRIPT_PATH "${CMAKE_SOURCE_DIR}/linker/${LINKER_SCRIPT}")
  set_target_properties(${KERNEL_EXECUTABLE} PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT_PATH}")
  target_link_libraries(${KERNEL_EXECUTABLE} PRIVATE "-Wl,--script,${LINKER_SCRIPT_PATH},-lgcc,--omagic,--build-id=none")

  if (NOT BP_LOWER STREQUAL "raspi4")
    add_custom_target(run_${NAME})
    add_dependencies(run_${NAME} ${NAME})

    add_custom_command(TARGET run_${NAME} POST_BUILD
      COMMAND eval "${QEMU}${KERNEL_EXECUTABLE}"
     VERBATIM USES_TERMINAL)

    add_custom_target(debug_${NAME})
    add_dependencies(debug_${NAME} ${NAME})

    add_custom_command(TARGET debug_${NAME} POST_BUILD
      COMMAND eval "${QEMU}${KERNEL_EXECUTABLE} -s -S"
      VERBATIM USES_TERMINAL)
  endif()

  if(NOT TEST_TYPE STREQUAL "none" AND NOT BP_LOWER STREQUAL "raspi4")
    add_custom_target(test_${NAME} ALL)
    add_dependencies(test_${NAME} ${NAME})

    # This uses TEST_TYPE to decide what to do
    configure_file("${CMAKE_SOURCE_DIR}/scripts/test_demo.sh" "${CMAKE_BINARY_DIR}/demos/${NAME}/test.sh")
    add_custom_command(TARGET test_${NAME} POST_BUILD
      COMMAND "${CMAKE_BINARY_DIR}/demos/${NAME}/test.sh"
      VERBATIM)
  endif(NOT TEST_TYPE STREQUAL "none" AND NOT BP_LOWER STREQUAL "raspi4")
endfunction(__add_demo)
