if(COVERAGE)
  # Obviously can't get results out of our own filesystem
  return()
endif()

add_demo(filesystem)

#Use the in memory filesystem
target_sources(filesystem PRIVATE src/file_system.c)
target_compile_definitions(filesystem PRIVATE USE_FS)
