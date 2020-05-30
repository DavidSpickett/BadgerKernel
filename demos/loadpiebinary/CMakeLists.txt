if(LINUX OR LTO)
  return()
endif()

add_demo(loadpiebinary)
add_shared_loadable(loadpiebinary piebinary)
