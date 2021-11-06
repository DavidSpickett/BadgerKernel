# We can not load binary files without semihosting
if(LTO OR NOT SEMIHOSTING)
  return()
endif()

# UBSAN adds around 4k of .data to print.cpp
# Which breaks the 64k RAM budget on thumb
if(BP_LOWER STREQUAL "thumb" AND SANITIZERS)
  if(OPT_LEVEL STREQUAL "3" OR OPT_LEVEL STREQUAL "0")
    return()
  endif()
endif()

add_demo(loadbinaries 5)
add_loadable(loadbinaries task)
add_loadable(loadbinaries task2)
target_compile_definitions(loadbinaries PRIVATE CODE_BACKING_PAGES=3)
