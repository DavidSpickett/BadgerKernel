if(COVERAGE)
  # Obviously can't get results out of our own filesystem
  return()
endif()

add_demo(filesystem)

target_compile_definitions(filesystem PRIVATE HAS_FILESYSTEM)
#Use the in memory filesystem
target_sources(filesystem PRIVATE src/file_system.c)
