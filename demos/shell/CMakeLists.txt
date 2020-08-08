if(LINUX OR LTO)
  return()
endif()

add_demo_no_test(shell)
add_loadable_no_test(shell echo)
add_loadable_no_test(shell ps)
target_compile_definitions(shell PRIVATE CODE_BACKING_PAGES=3)
