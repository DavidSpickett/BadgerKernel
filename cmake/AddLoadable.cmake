function(add_loadable PARENT NAME)
  add_executable(${NAME} demos/${PARENT}/${NAME}.c)
  add_dependencies(${NAME} ${PARENT})
  target_link_libraries(${NAME} PRIVATE "-Wl,--defsym=code_page_size=${CODE_PAGE_SIZE},--just-symbols=loadbinary,-T,linker/loadable.ld,--build-id=none")
  add_custom_command(TARGET ${NAME} POST_BUILD
    COMMAND eval "${PREFIX}objcopy -O binary ${NAME} ${NAME}.bin")
  # TODO: not ideal, we end up building them during the lit run
  add_dependencies(run_${PARENT} ${NAME})
  add_dependencies(test_${PARENT} ${NAME})
endfunction(add_loadable)
