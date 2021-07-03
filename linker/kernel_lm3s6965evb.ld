ENTRY(_Reset)
SECTIONS
{
 .vectors . : { *vectors.s.o(.text) }
 .startup . : { *startup.s.o(.text) }
 .text : {
    *(.text*)
    *(.rodata*)
    /* align to pointer size (8 for AArch64) */
    . = ALIGN(8);
    _init_array = .;
    *(.init_array*)
    _einit_array = .;
    _etext = . ;
 }
 . = 0x20000000;
 ram_start = . ;
 .data :
  AT ( ADDR (.text) + SIZEOF (.text) )
  {
    _data = . ;
    *(.data)
    *(.thread_vars)
    _edata = . ; }
 /* align 16 for AArch64 */
 . = ALIGN(16);
 . = . + 0xC00; /* 3kB of stack for kernel */
 stack_top = .;
 .bss (NOLOAD) : {
    _bstart = . ;
    /* Thread structs go first! */
    *(.thread_structs)
    /* then the important zero init globals */
    *(.thread_vars_bss)
    *(.bss COMMON)
    /* Putting this first minimises waste on Arm/Thumb due to alignment below */
    *(.code_page_backing)
    /* AArch64 PIE requires that programs start on 4k alignemnt for adrp usage.
       We don't need to align for function starts, we get the proper entry addr from the elf.
    */
    . = ALIGN(0x1000);
    /* on the end in case of weird stuff writing beyond */
    *(.code_page)
    _bend = . ;
  }
  ram_end = .;
  ASSERT((ram_end <= (ram_start + 0x10000)), "Too much data for RAM!")
  /* Don't need unwinding info on Arm */
  /DISCARD/ : { *(.ARM.exidx* ) }
}
