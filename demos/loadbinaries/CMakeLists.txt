if(LTO)
  return()
endif()

# UBSAN adds around 4k of .data to print.cpp
# Which breaks the 64k RAM budget on thumb
if(BP_LOWER STREQUAL "thumb" AND
   OPT_LEVEL STREQUAL "3" AND
   SANITIZERS)
  return()
endif()

add_demo(loadbinaries 5)
add_loadable(loadbinaries task)
add_loadable(loadbinaries task2)
target_compile_definitions(loadbinaries PRIVATE CODE_BACKING_PAGES=3)
