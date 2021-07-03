set( KERNEL_SOURCES ${KERNEL_SOURCES}
  ${CMAKE_SOURCE_DIR}/src/kernel/thread.c
  ${CMAKE_SOURCE_DIR}/src/kernel/thread_property.cpp
  ${CMAKE_SOURCE_DIR}/src/kernel/mutex.c
  ${CMAKE_SOURCE_DIR}/src/kernel/condition_variable.c
  ${CMAKE_SOURCE_DIR}/src/kernel/signal_handling.c
  ${CMAKE_SOURCE_DIR}/src/kernel/alloc.c
  ${CMAKE_SOURCE_DIR}/src/kernel/syscall.c
  ${CMAKE_SOURCE_DIR}/src/kernel/message.c
  ${CMAKE_SOURCE_DIR}/src/kernel/semihosting.c
  ${CMAKE_SOURCE_DIR}/src/kernel/file.c
  PARENT_SCOPE
)
