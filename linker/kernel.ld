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
 . = . + 0x800; /* 1kB of stack for kernel */
 stack_top = .;
 .bss : {
    /* Thread structs go first! */
    *(.thread_structs)
    /* then the important globals */
    *(.thread_vars)
    *(.bss COMMON)
    /* 8 byte fn alignment seems ok for everyone */
    . = ALIGN(8);
    /* on the end in case of weird stuff writing beyond */
    *(.code_page)
    *(.code_page_backing)
  }
  ram_end = .;
  ASSERT((ram_end <= (ram_start + ram_size)), "Too much data for RAM!")
}
