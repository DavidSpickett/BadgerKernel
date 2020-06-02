# To support LTO we need to attribute used a lot of stuff
if(LINUX OR LTO)
  return()
endif()

add_demo(loadpiebinary)
add_shared_loadable(loadpiebinary piebinary)
