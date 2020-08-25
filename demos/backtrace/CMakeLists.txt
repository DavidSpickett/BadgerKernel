if(BP_LOWER STREQUAL "thumb")
  return()
endif()

add_demo(backtrace 2)
target_compile_options(backtrace PRIVATE -mapcs-frame)
