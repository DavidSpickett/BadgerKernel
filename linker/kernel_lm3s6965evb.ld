ENTRY(_Reset)

SECTIONS {
 .vectors . : { *vectors.s.o(.text) }
 .startup . : { *startup.s.o(.text) }

 .text : {
   *(.text*)
   *(.rodata*)
   . = ALIGN(4); /* align to pointer size */
   _init_array = .;
   *(.init_array*)
   _einit_array = .;
   _etext = .;
 }

 . = 0x20000000;
 ram_start = . ;

 .data : AT ( ADDR (.text) + SIZEOF (.text) ) {
   _data = .;
   *(.data)
   *(.thread_vars)
   _edata = .;
 }

 . = ALIGN(8); /* align to 2x pointer size */
 . = . + 0xC00; /* 3kB of stack for kernel */
 stack_top = .;

 .bss (NOLOAD) : {
   _bstart = .;
   /* thread structs go first */
   *(.thread_structs)
   /* then the important zero init globals */
   *(.thread_vars_bss)
   *(.bss COMMON)
   *(.code_page*)
   _bend = .;
 }

 ram_end = .;
 ASSERT((ram_end <= (ram_start + 0x10000)), "Too much data for RAM!")

 /DISCARD/ : { *(.ARM.exidx* ) } /* Discard unwinding info */
}
