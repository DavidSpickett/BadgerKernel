ENTRY(worker)
SECTIONS
{
  . = code_page;
  .text : {
    /* If we actually loaded ELF this wouldn't be needed
       We'd just read the entry point from the ELF */
    *(.worker)
    *(.text)
  }
  .rodata : { *(.rodata) }
  .bss : { *(.bss) }
  ASSERT((. - code_page) <= code_page_size, "Loadable too big for code page!")
}
