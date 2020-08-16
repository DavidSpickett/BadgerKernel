if(BP_LOWER STREQUAL "aarch64")
  # AArch64 toolchain was built for hardware TLS
  return()
endif()

add_demo(threadlocalstorage 2)
