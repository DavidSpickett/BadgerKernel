# We can not load binaries without semihosting
if(NOT SEMIHOSTING)
  return()
endif()

add_demo(loadbinary 3)
add_loadable(loadbinary binary)
