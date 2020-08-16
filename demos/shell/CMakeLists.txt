if(LTO)
  return()
endif()

add_demo_no_test(shell 3)
add_loadable_no_test(shell echo)
add_loadable_no_test(shell ps)
add_loadable_no_test(shell ls)
target_compile_definitions(shell PRIVATE CODE_BACKING_PAGES=3)
