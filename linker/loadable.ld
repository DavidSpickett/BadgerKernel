ENTRY(worker)
SECTIONS
{
  /*. = code_page; TODO: put this back for non PIE */
  code_page = .; /* to pass the check below */
  .text : { *(.text) }
  .rodata : { *(.rodata) }
  .bss : { *(.bss) }
  ASSERT((. - code_page) <= code_page_size, "Loadable too big for code page!")
}
