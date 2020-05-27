if(LINUX OR LTO)
  return()
endif()

add_demo(loadbinary)
add_loadable(loadbinary binary)
