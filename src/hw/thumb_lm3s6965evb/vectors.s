.extern __platform_yield

.4byte stack_top
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
.4byte handle_exception
.4byte default
.4byte default
.4byte default
.4byte handle_exception
# skipping vectors we aren't using...

default:
  b .
