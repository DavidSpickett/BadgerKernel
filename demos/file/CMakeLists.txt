# We can not perform file operations without semihosting
if(NOT SEMIHOSTING)
  return()
endif()

add_demo(file 6)
