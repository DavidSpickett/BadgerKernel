ENTRY(worker)
SECTIONS
{
  code_start = .;
  .text : { *(.text) }
  .rodata : { *(.rodata) }
  .bss : { *(.bss) }
  ASSERT((. - code_start) <= code_page_size, "PIE Loadable too big for code page!")
}
