set( KERNEL_SOURCES ${KERNEL_SOURCES}
  ${CMAKE_SOURCE_DIR}/src/common/print.cpp
  ${CMAKE_SOURCE_DIR}/src/common/trace.c
  ${CMAKE_SOURCE_DIR}/src/common/assert.c
  ${CMAKE_SOURCE_DIR}/src/common/errno.c
)

if(SANITIZERS)
  list(APPEND KERNEL_SOURCES
       ${CMAKE_SOURCE_DIR}/src/common/ubsan.c)
endif()

set(KERNEL_SOURCES ${KERNEL_SOURCES} PARENT_SCOPE)
