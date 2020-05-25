ENTRY(_Reset)
SECTIONS
{
 .vectors . : { *vectors.s.o(.text) }
 .startup . : { *startup.s.o(.text) }
 .text : { *(.text) }
 . = ram_start;
 .data : { *(.data) }
 /* align 16 for AArch64 */
 . = ALIGN(16);
 . = . + 0x400; /* 1kB of stack for kernel */
 stack_top = .;
 .bss : {
    /* Thread structs go first! */
    *(.thread_structs)
    /* then the important globals */
    *(.thread_vars)
    /* Thumb fns will be aligned to a 4 byte start address */
    . = ALIGN(4);
    *(.loaded_code)
    *(.bss COMMON)
  }
  ram_end = .;
  ASSERT((ram_end <= (ram_start + ram_size)), "Too much data for RAM!")
}
