# LTO needs us to attribute used a whole bunch of stuff
if(LINUX OR LTO)
  return()
endif()

add_demo(loadbinary)
add_loadable(loadbinary binary)
