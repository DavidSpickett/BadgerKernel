# To support LTO we need to attribute used a lot of stuff
# We can not load binaries without semihosting
if(LTO OR NOT SEMIHOSTING)
  return()
endif()

add_demo(loadpiebinary 2)
add_shared_loadable(loadpiebinary piebinary)

target_compile_definitions(loadpiebinary_kernel PRIVATE "STARTUP_PROG=\"piebinary\"")
