ENTRY(worker)
SECTIONS
{
  . = code_page;
  .text : { *(.text) }
  .rodata : { *(.rodata) }
  .bss : { *(.bss) }
  ASSERT((. - code_page) <= code_page_size, "Loadable too big for code page!")
}
