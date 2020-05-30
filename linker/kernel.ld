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
    /* Putting this first mimises waste on Arm/Thumb due to alignment below */
    *(.code_page_backing)
    /* AArch64 PIE requires that programs start on 4k alignemnt for adrp usage.
       We don't need to align for function starts, we get the proper entry addr from the elf.
    */
    . = ALIGN(0x1000);
    /* on the end in case of weird stuff writing beyond */
    *(.code_page)
  }
  ram_end = .;
  ASSERT((ram_end <= (ram_start + ram_size)), "Too much data for RAM!")
}
