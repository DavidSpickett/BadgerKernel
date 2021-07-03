set( KERNEL_SOURCES ${KERNEL_SOURCES}
  # /user files here for threads within the same binary
  ${CMAKE_SOURCE_DIR}/src/user/errno.c
  ${CMAKE_SOURCE_DIR}/src/user/thread.c
  ${CMAKE_SOURCE_DIR}/src/user/file.c
  ${CMAKE_SOURCE_DIR}/src/user/util.c
  ${CMAKE_SOURCE_DIR}/src/user/alloc.c
  ${CMAKE_SOURCE_DIR}/src/user/mutex.c
  ${CMAKE_SOURCE_DIR}/src/user/condition_variable.c
  ${CMAKE_SOURCE_DIR}/src/user/timer.c
  ${CMAKE_SOURCE_DIR}/src/user/fiber.c
  PARENT_SCOPE
)
