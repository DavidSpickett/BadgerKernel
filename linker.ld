ENTRY(_Reset)
SECTIONS
{
 . = ram_start;
 .startup . : { *startup.s.o(.text) }
 .text : { *(.text) }
 .data : { *(.data) }
 /* Cortex M needs to offset stack up into RAM.
    Everyone else can leave this as 0 as they run
    completely from RAM. */
 . = . + stack_offset;
 . = ALIGN(8);
 . = . + 0x1000; /* 4kB of stack memory */
 stack_top = .;
 /* This is here so that it's read/write on Cortex M */
 .bss : {
    /* Thread structs go first! */
    *(.thread_structs)
    *(.thread_vars)
    . = ALIGN(16);
    *(.monitor_stack)
    *(.bss COMMON)
  }
}
