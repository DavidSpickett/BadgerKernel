#include "elf.h"
#include "file_system.h"
#include "print.h"
#include <stddef.h>
#include <stdint.h>

#ifndef CODE_PAGE_SIZE
#error "Loading ELFs requires CODE_PAGE_SIZE"
#endif

#define DEBUG_ELF_LOAD 0
#define DEBUG_MSG_ELF(fmt, ...) \
  do { if(DEBUG_ELF_LOAD) { printf(fmt, __VA_ARGS__); } } while(0)

typedef struct {
    unsigned char e_ident[16];
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    void*         e_entry;
    size_t        e_phoff;
    size_t        e_shoff;
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;
    uint16_t      e_phnum;
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;
} ElfHeader;

typedef struct {
#ifdef __aarch64__
    uint32_t   sh_name;
    uint32_t   sh_type;
    uint64_t   sh_flags;
    void*      sh_addr;
    size_t     sh_offset;
    uint64_t   sh_size;
    uint32_t   sh_link;
    uint32_t   sh_info;
    uint64_t   sh_addralign;
    uint64_t   sh_entsize;
#else
    uint32_t   sh_name;
    uint32_t   sh_type;
    uint32_t   sh_flags;
    void*      sh_addr;
    size_t     sh_offset;
    uint32_t   sh_size;
    uint32_t   sh_link;
    uint32_t   sh_info;
    uint32_t   sh_addralign;
    uint32_t   sh_entsize;
#endif /* ifdef __aarch64__ */
} SectionHeader;

#ifdef __aarch64__
#define EXPECTED_MACHINE 183
#else
#define EXPECTED_MACHINE 40
#endif

#define SHF_ALLOC 1<<1

#define EI_DATA_BYTE 6
#define ELFDATA2LSB 1

extern uint8_t code_page[CODE_PAGE_SIZE];

static void check_elf_hdr(const ElfHeader* header) {
  if ((header->e_ident[0] != 0x7F) ||
      (header->e_ident[1] != 'E')  ||
      (header->e_ident[2] != 'L')  ||
      (header->e_ident[3] != 'F')) {
    printf("Magic bytes don't match an ELF file\n");
    exit(1);
  }

  // Check endian before we check anything endian dependent
  if (header->e_ident[6] != ELFDATA2LSB) {
    printf("ELF must be little endian\n");
    exit(1);
  }

  if (header->e_machine != EXPECTED_MACHINE) {
    printf("e_machine does not match expected value");
    exit(1);
  }

  if (!header->e_shoff) {
    printf("ELF has no section table\n");
    exit(1);
  }
}

static bool load_section (int elf, uint16_t idx,
                          size_t section_table_offs,
                          size_t section_hdr_size,
                          void* dest) {
  off_t expected_hdr_pos = section_table_offs + (section_hdr_size*idx);
  off_t hdr_pos = lseek(elf, expected_hdr_pos, SEEK_CUR);
  if (hdr_pos != expected_hdr_pos) {
    printf("Couldn't seek to header for section %u\n", idx);
    exit(1);
  }

  SectionHeader section_hdr;
  ssize_t got = read(elf, &section_hdr, section_hdr_size);
  if (got < section_hdr_size) {
    printf("Couldn't read header for section %u\n", idx);
    exit(1);
  }

  if (!(section_hdr.sh_flags & SHF_ALLOC)) {
    DEBUG_MSG_ELF("Skipping section %u, not ALLOC\n", idx);
    return false;
  }

  void* start_code_page = (void*)code_page;
  void* end_code_page = (void*)&code_page[CODE_PAGE_SIZE];

  if (
      (section_hdr.sh_addr < start_code_page) ||
      (section_hdr.sh_addr >= end_code_page)) {
    printf("Section %u's start address not within code page\n", idx);
  }

  void* section_end = (void*)((size_t)(section_hdr.sh_addr) + section_hdr.sh_size);
  if (section_end >= end_code_page) {
    printf("Section %u extends off of the end of the code page\n", idx);
    exit(1);
  }

  off_t expected_section_content_pos = section_hdr.sh_offset;
  off_t section_content_pos = lseek(elf,
    expected_section_content_pos, SEEK_CUR);
  if (section_content_pos != expected_section_content_pos) {
    printf("Couldn't seek to content for section %u\n", idx);
    exit(1);
  }

  // Binaries are built to run at the code page
  // so remove that offset from section addresses
  void* dest_addr = dest+(section_hdr.sh_addr-start_code_page);

  // Copy from ELF file to destination page
  // (which can be different from code_page)
  ssize_t section_got = read(elf, dest_addr, section_hdr.sh_size);
  if (section_got != section_hdr.sh_size) {
    printf("Couldn't read content for section %u\n", idx);
    exit(1);
  }
  DEBUG_MSG_ELF("Loaded %u bytes from section %u\n",
    section_got, idx);

  return true;
}

void (*load_elf(const char* filename, void* dest))(void) {
  /* Validate elf and load all ALLOC sections into location
     dest. Returns a function pointer to the ELF entry point.
  */
  DEBUG_MSG_ELF("Loading \"%s\"\n", filename);
  int elf = open(filename, O_RDONLY);
  if (elf < 0) {
    printf("Couldn't open file %s\n", filename);
    exit(1);
  }

  size_t header_size = sizeof(ElfHeader);
  ElfHeader elf_hdr;
  ssize_t got = read(elf, &elf_hdr, header_size);
  if (got < header_size) {
    printf("Couldn't read complete header from %s\n",
      filename);
    exit(1);
  }

  check_elf_hdr(&elf_hdr);

  off_t new_pos = lseek(elf, elf_hdr.e_shoff, SEEK_CUR);
  if (new_pos != elf_hdr.e_shoff) {
    printf("Couldn't seek to section table\n");
    exit(1);
  }

  if (sizeof(SectionHeader) != elf_hdr.e_shentsize) {
    printf("Section header size didn't match struct size\n");
    exit(1);
  }

  bool loaded_something = false;
  for (uint16_t idx=0; idx < elf_hdr.e_shnum; ++idx) {
     loaded_something |= load_section(elf, idx,
            elf_hdr.e_shoff, elf_hdr.e_shentsize, dest);
  }

  if (!loaded_something) {
    printf("Loaded no sections from \"%s\"\n", filename);
    exit(1);
  }

  DEBUG_MSG_ELF("Loaded \"%s\"\n", filename);
  // Note that Thumb addresses already have the bit set
  return elf_hdr.e_entry;
}
