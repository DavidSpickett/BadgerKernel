# We don't have enough RAM on the Thumb board
if(LTO OR (BP_LOWER STREQUAL "thumb"))
  return()
endif()

add_demo_no_test(shell)
add_loadable_no_test(shell echo)
add_loadable_no_test(shell ps)
add_loadable_no_test(shell ls)
target_compile_definitions(shell PRIVATE CODE_BACKING_PAGES=3)
