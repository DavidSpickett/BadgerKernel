# LTO needs us to attribute used a whole bunch of stuff
# We can not load binaries without semihosting
if(LTO OR NOT SEMIHOSTING)
  return()
endif()

add_demo(loadbinary 3)
add_loadable(loadbinary binary)
