if(LINUX)
  return()
endif()

add_demo(loadbinary)

if( BP_LOWER STREQUAL "arm" )
  set( CODE_PAGE_SIZE 256 )
elseif( BP_LOWER STREQUAL "thumb" )
  set( CODE_PAGE_SIZE 256 )
elseif( BP_LOWER STREQUAL "aarch64" )
  # Due to ardp alignment etc., we need more room
  set( CODE_PAGE_SIZE 5120 ) # 5k
else()
  message(FATAL_ERROR "Can't set CODE_PAGE_SIZE for \"${BP_LOWER}\".")
endif()

target_compile_definitions(loadbinary PRIVATE CODE_PAGE_SIZE=${CODE_PAGE_SIZE})

# Use semihosting to load binary
target_sources(loadbinary PRIVATE src/hw/arm_file.c)

include(demos/loadbinary/loadable/CMakeLists.txt)
# TODO: must be a cleaner way to do this
add_dependencies(run_loadbinary loadable)
add_dependencies(test_loadbinary loadable)
