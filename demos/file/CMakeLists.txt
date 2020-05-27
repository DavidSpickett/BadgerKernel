add_demo(file)

if(NOT LINUX)
  # Use semihosting
  target_sources(file PRIVATE src/hw/arm_file.c)
endif()
