function(add_loadable PARENT NAME)
  __add_loadable(${PARENT} ${NAME} OFF)
endfunction(add_loadable)

function(add_shared_loadable PARENT NAME)
  __add_loadable(${PARENT} ${NAME} ON)
endfunction(add_shared_loadable)

function(__add_loadable PARENT NAME PIE)
  # Enable loading in the parent kernel
  if( BP_LOWER STREQUAL "arm" )
    set( CODE_PAGE_SIZE 4096 )
  elseif( BP_LOWER STREQUAL "thumb" )
    set( CODE_PAGE_SIZE 512 )
  elseif( BP_LOWER STREQUAL "aarch64" )
    # Due to ardp alignment etc., we need more room
    set( CODE_PAGE_SIZE 5120 ) # 5k
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

  if(PIE)
    target_compile_options(${NAME} PRIVATE -fpie -shared -L. -l:${NAME})
    target_link_libraries(${NAME} PRIVATE "-Wl,--defsym=code_page_size=${CODE_PAGE_SIZE},-T,linker/loadable.ld,--build-id=none,-pie,-shared")
  else()
    # TODO: dedupe
    target_link_libraries(${NAME} PRIVATE "-Wl,--defsym=code_page_size=${CODE_PAGE_SIZE},--just-symbols=${PARENT},-T,linker/loadable.ld,--build-id=none")
  endif()

  # TODO: not ideal, we end up building them during the lit run
  add_dependencies(run_${PARENT} ${NAME})
  add_dependencies(test_${PARENT} ${NAME})
  add_dependencies(debug_${PARENT} ${NAME})
endfunction(__add_loadable)
