# Backtraces only work consistently on Arm
# They can work at -01 too but let's not take the risk.
if(BP_LOWER STREQUAL "arm" AND
   OPT_LEVEL STREQUAL "0")
  add_demo_expect_test(backtrace 2)
  target_compile_options(backtrace_kernel PRIVATE -mapcs-frame)
endif()
