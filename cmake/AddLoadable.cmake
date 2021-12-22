function(add_loadable PARENT NAME)
  __add_loadable(${PARENT} ${NAME} OFF)
endfunction(add_loadable)

function(add_shared_loadable PARENT NAME)
  __add_loadable(${PARENT} ${NAME} ON)
endfunction(add_shared_loadable)

function(__add_loadable PARENT NAME PIE)
  # Enable loading in the parent kernel
  if( BP_LOWER STREQUAL "arm" )
    set( CODE_PAGE_SIZE 9216 )
  elseif( BP_LOWER STREQUAL "thumb" )
    set( CODE_PAGE_SIZE 8192 )
  elseif( BP_LOWER STREQUAL "aarch64" )
    # Due to ardp alignment etc., we need more room
    set( CODE_PAGE_SIZE 12288 )
  elseif( BP_LOWER STREQUAL "raspi4" )
    set( CODE_PAGE_SIZE 11264 )
  else()
    message(FATAL_ERROR "Can't set CODE_PAGE_SIZE for \"${BP_LOWER}\".")
  endif()

  target_compile_definitions(${PARENT}_kernel PRIVATE CODE_PAGE_SIZE=${CODE_PAGE_SIZE})
  # Need elf parser
  target_sources(${PARENT}_kernel PRIVATE ${CMAKE_SOURCE_DIR}/src/user/elf.c)

  add_executable(${NAME} ${CMAKE_SOURCE_DIR}/demos/${PARENT}/${NAME}.c)
  # We depend on the kernel so we can use its symbols
  add_dependencies(${NAME} ${PARENT}_kernel)

  set(LINK_CMD "-Wl,--defsym=code_page_size=${CODE_PAGE_SIZE},--build-id=none,")
  if(PIE)
    target_compile_options(${NAME} PRIVATE -fpie -shared -fPIC)
    set(LINKER_SCRIPT_PATH "${CMAKE_SOURCE_DIR}/linker/pie_loadable.ld")
    target_link_libraries(${NAME} PRIVATE "${LINK_CMD}-T,${LINKER_SCRIPT_PATH},-pie,-shared")
    if(SANITIZERS)
      target_sources(${NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/common/ubsan.c)
    endif()
  else()
    set(LINKER_SCRIPT_PATH "${CMAKE_SOURCE_DIR}/linker/loadable.ld")
    target_link_libraries(${NAME} PRIVATE "${LINK_CMD}-T,${LINKER_SCRIPT_PATH},--just-symbols=${CMAKE_BINARY_DIR}/demos/${PARENT}/${PARENT}_kernel")
  endif()
  set_target_properties(${NAME} PROPERTIES LINK_DEPENDS "${LINKER_SCRIPT_PATH}")

  # The custom target "demo" depends on "demo_kernel" and its loadables.
  add_dependencies(${PARENT} ${NAME})
endfunction(__add_loadable)
