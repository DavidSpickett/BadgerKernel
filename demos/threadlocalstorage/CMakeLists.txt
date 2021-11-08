if(BP_LOWER STREQUAL "aarch64" OR BP_LOWER STREQUAL "raspi4")
  # AArch64 toolchain was built for hardware TLS
  return()
endif()

add_demo(threadlocalstorage 2)
