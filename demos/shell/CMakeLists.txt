if(LINUX OR LTO)
  return()
endif()

add_demo(shell)
add_loadable(shell test_app)
target_compile_definitions(shell PRIVATE CODE_BACKING_PAGES=3)
