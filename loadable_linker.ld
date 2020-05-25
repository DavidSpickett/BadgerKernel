ENTRY(worker)
SECTIONS
{
  . = loaded_code;
  .text : {
    /* If we actually loaded ELF this wouldn't be needed
       We'd just read the entry point from the ELF */
    *(.worker)
    *(.text)
  }
  .rodata : { *(.rodata) }
  .bss : { *(.bss) }
  /* TODO: assert total size here */
}
