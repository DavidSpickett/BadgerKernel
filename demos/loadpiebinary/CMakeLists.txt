# To support LTO we need to attribute used a lot of stuff
if(LTO)
  return()
endif()

add_demo(loadpiebinary 6)
add_shared_loadable(loadpiebinary piebinary)

target_compile_definitions(loadpiebinary PRIVATE "STARTUP_PROG=\"piebinary\"")
