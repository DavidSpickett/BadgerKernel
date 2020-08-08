if(LINUX OR LTO)
  return()
endif()

add_demo(shell)
add_loadable(shell echo)
add_loadable(shell ps)
target_compile_definitions(shell PRIVATE CODE_BACKING_PAGES=3)
