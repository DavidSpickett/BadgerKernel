# We can not load binaries without semihosting
if(NOT SEMIHOSTING)
  return()
endif()

# TODO: we need to properly handle the relocation to yield for thumb
if(LTO AND BP_LOWER STREQUAL "thumb")
  return()
endif()

add_demo(loadpiebinary 2)
add_shared_loadable(loadpiebinary piebinary)

target_compile_definitions(loadpiebinary_kernel PRIVATE "STARTUP_PROG=\"piebinary\"")
