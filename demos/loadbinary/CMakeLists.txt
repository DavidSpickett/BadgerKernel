if(LINUX)
  return()
endif()

add_demo(loadbinary)

# Use semihosting to load binary
target_sources(loadbinary PRIVATE src/hw/arm_file.c)
target_compile_definitions(loadbinary PRIVATE HAS_FILESYSTEM)
