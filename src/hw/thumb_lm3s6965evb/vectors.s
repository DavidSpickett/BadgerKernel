.4byte monitor_stack_top
.4byte _Reset
.4byte default
.4byte default
.4byte default
.4byte default
.4byte default
.4byte default
.4byte default
.4byte default
.4byte default
.extern __platform_yield
.4byte __platform_yield
.4byte default
.4byte default
.4byte default
.4byte __platform_yield
# skipping vectors we aren't using...

default:
  b .
