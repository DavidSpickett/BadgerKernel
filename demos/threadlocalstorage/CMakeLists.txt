if(BP_LOWER STREQUAL "aarch64" OR (LINUX AND NOT CLANG))
  # AArch64 toolchain was built for hardware TLS
  # Clang has an option to enable software TLS
  return()
endif()

# TODO: why not run this on linux anyway?

add_demo(threadlocalstorage)
