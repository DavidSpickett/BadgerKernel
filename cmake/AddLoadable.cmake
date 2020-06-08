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
    set( CODE_PAGE_SIZE 7168 )
  elseif( BP_LOWER STREQUAL "aarch64" )
    # Due to ardp alignment etc., we need more room
    set( CODE_PAGE_SIZE 11264 )
  else()
    message(FATAL_ERROR "Can't set CODE_PAGE_SIZE for \"${BP_LOWER}\".")
  endif()

  target_compile_definitions(${PARENT} PRIVATE CODE_PAGE_SIZE=${CODE_PAGE_SIZE})
  # Use semihosting to load binary
  target_sources(${PARENT} PRIVATE src/hw/arm_file.c)
  # Need elf parser
  target_sources(${PARENT} PRIVATE src/elf.c)

  add_executable(${NAME} demos/${PARENT}/${NAME}.c)
  add_dependencies(${NAME} ${PARENT})

  set(LINK_CMD "-Wl,--defsym=code_page_size=${CODE_PAGE_SIZE},--build-id=none,")
  if(PIE)
    target_compile_options(${NAME} PRIVATE -fpie -shared -L. -l:${NAME})
    target_link_libraries(${NAME} PRIVATE "${LINK_CMD}-T,linker/pie_loadable.ld,-pie,-shared")
    if(SANITIZERS)
      target_sources(${NAME} PRIVATE src/ubsan.c)
    endif()
  else()
    target_link_libraries(${NAME} PRIVATE "${LINK_CMD}-T,linker/loadable.ld,--just-symbols=${PARENT}")
  endif()

  # TODO: not ideal, we end up building them during the lit run
  add_dependencies(run_${PARENT} ${NAME})
  add_dependencies(test_${PARENT} ${NAME})
  add_dependencies(debug_${PARENT} ${NAME})
endfunction(__add_loadable)
