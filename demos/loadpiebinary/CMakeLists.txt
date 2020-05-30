# UBSAN requires that we add all the handlers to the kernel symbol list
# To support LTO we need to attribute used a lot of stuff
if(LINUX OR LTO OR SANITIZERS)
  return()
endif()

add_demo(loadpiebinary)
add_shared_loadable(loadpiebinary piebinary)
