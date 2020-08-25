# Backtraces only work consistently on Arm
# They can work at -01 too but let's not take the risk.
if(BP_LOWER STREQUAL "arm" AND
   OPT_LEVEL STREQUAL "0")
  add_demo(backtrace 2)
  target_compile_options(backtrace PRIVATE -mapcs-frame)
endif()
