# We can not perform file operations without semihosting
if(NOT SEMIHOSTING)
  return()
endif()

if(BP_LOWER STREQUAL "thumb" AND SANITIZERS)
  if (OPT_LEVEL STREQUAL "0" OR OPT_LEVEL STREQUAL "3")
    return()
  endif()
endif()

add_demo_no_test(shell 4)
add_loadable(shell echo)
add_loadable(shell ps)
add_loadable(shell ls)
target_compile_definitions(shell_kernel PRIVATE CODE_BACKING_PAGES=3)
