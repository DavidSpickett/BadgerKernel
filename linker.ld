ENTRY(_Reset)
SECTIONS
{
 .vectors . : { *vectors.s.o(.text) }
 . = ram_start;
 .startup . : { *startup.s.o(.text) }
 .text : { *(.text) }
 .data : { *(.data) }
 /* align 16 for AArch64 */
 . = ALIGN(16);
 . = . + 0x400; /* 1kB of stack memory for entry */
 stack_top = .;
 /* This is here so that it's read/write on Cortex M */
 .bss : {
    /* Thread structs go first! */
    *(.thread_structs)
    /* including the scheduler thread */
    *(.scheduler_thread)
    /* then the important globals */
    *(.thread_vars)
    /* align 16 for AArch64 */
    . = ALIGN(16);
    *(.monitor_stack)
    *(.bss COMMON)
  }
}
