if(LINUX OR LTO)
  return()
endif()

add_demo(loadbinaries)
add_loadable(loadbinaries task)
add_loadable(loadbinaries task2)
target_compile_definitions(loadbinaries PRIVATE CODE_BACKING_PAGES=3)
