add_executable(loadable demos/loadbinary/loadable/loadable.c)
add_dependencies(loadable loadbinary)
target_compile_definitions(loadbinary PRIVATE CODE_PAGE_SIZE=${CODE_PAGE_SIZE})
target_link_libraries(loadable PRIVATE "-Wl,--defsym=code_page_size=${CODE_PAGE_SIZE},--just-symbols=loadbinary,-T,linker/loadable.ld,--build-id=none")
add_custom_command(TARGET loadable POST_BUILD
  COMMAND eval "${PREFIX}objcopy -O binary loadable loadable.bin")
