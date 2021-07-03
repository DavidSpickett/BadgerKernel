ENTRY(_Reset)

SECTIONS {
 .vectors . : { *vectors.s.o(.text) }
 .startup . : { *startup.s.o(.text) }

 .text : {
   *(.text*)
   *(.rodata*)
    . = ALIGN(8); /* align to pointer size (8 for AArch64) */
   _init_array = .;
   *(.init_array*)
   _einit_array = .;
   _etext = .;
 }

 . = 0x40000000;
 ram_start = .;

 .data : AT ( ADDR (.text) + SIZEOF (.text) )
 {
   _data = .;
   *(.data)
   *(.thread_vars)
   _edata = .;
 }

 . = ALIGN(16); /* align to 2x pointer size, 16 for AArch64 */
 . = . + 0xC00; /* 3kB of stack for kernel */
 stack_top = .;

 .bss (NOLOAD) : {
   _bstart = . ;
   /* Thread structs go first */
   *(.thread_structs)
   /* then the important zero init globals */
   *(.thread_vars_bss)
   *(.bss COMMON)
   /* Putting this first minimises waste on Arm due to alignment below */
   *(.code_page_backing)
   /* AArch64 PIE requires that programs start on 4k alignment for adrp use */
   . = ALIGN(0x1000);
   *(.code_page)
   _bend = . ;
 }

 ram_end = .;
 ASSERT((ram_end <= (ram_start + 0x100000)), "Too much data for RAM!")

 /DISCARD/ : { *(.ARM.exidx* ) } /* Discard unwinding info */
}
