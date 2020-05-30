#include "elf.h"
#include "file_system.h"
#include "print.h"
#include "thread.h"
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#ifndef CODE_PAGE_SIZE
#error "Loading ELFs requires CODE_PAGE_SIZE"
#endif

#define DEBUG_ELF_LOAD 0
// ## will remove the preceeding comma if there are zero args
#define DEBUG_MSG_ELF(fmt, ...) \
  do { if(DEBUG_ELF_LOAD) { printf(fmt, ## __VA_ARGS__); } } while(0)
#define DEBUG_MSG_ELF_SECTION(msg) DEBUG_MSG_ELF("------ "msg" ------\n")

#define PRINT_EXIT(fmt, ...) printf("Error: "fmt, ## __VA_ARGS__); exit(1);

#define MAX_SECTION_NAME_LEN 80
// TODO: this assumption is more risky than the section names
#define MAX_SYMBOL_NAME_LEN 256

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

typedef struct {
  size_t r_offset;
  size_t r_info;
} ELFRelocation;
#ifdef __aarch64__
#define RELOC_SYM(info)  ((info) >> 32)
#define RELOC_TYPE(info) ((info) & 0xFFFFFFFF)
#else
#define RELOC_SYM(info)  ((info) >> 8)
#define RELOC_TYPE(info) ((info) & 0xFF)
#endif

typedef struct {
#ifdef __aarch64__
  uint32_t st_name;
  unsigned char	st_info;
  unsigned char	st_other;
  uint16_t st_shndx;
  size_t st_value;
  size_t st_size;
#else
  uint32_t	st_name;
  size_t	st_value;
  uint32_t	st_size;
  unsigned char	st_info;
  unsigned char	st_other;
  uint16_t	st_shndx;
#endif
} ELFSymbol;

// Used after we've looked up name
typedef struct {
  uint16_t idx;
  ELFSymbol symbol;
  char name[MAX_SYMBOL_NAME_LEN];
  unsigned char type;
  unsigned char bind;
} SymbolInfo;

#define SYM_BIND(x) ((x) >> 4)
#define SYM_TYPE(x) (x & 0xf)

#define SYM_BIND_LOCAL  0
#define SYM_BIND_GLOBAL 1
#define SYM_BIND_WEAK   2

#define SYM_TYPE_NOTYPE  0
#define SYM_TYPE_OBJECT  1
#define SYM_TYPE_FUNC    2
#define SYM_TYPE_SECTION 3
#define SYM_TYPE_FILE    4
#define SYM_TYPE_COMMON  5
#define SYM_TYPE_TLS     6

// Relocation types
#define R_ARM_REL32     3
#define R_ARM_GLOB_DAT  21
#define R_ARM_JUMP_SLOT 22
#define R_ARM_RELATIVE  23

#ifdef __aarch64__
#define EXPECTED_MACHINE 183
#else
#define EXPECTED_MACHINE 40
#endif

#define SHF_ALLOC 1<<1

#define SHN_UNDEF 0

#define EI_DATA_BYTE 6
#define ELFDATA2LSB 1

#define ET_EXEC 2
#define ET_DYN  3

#define SHT_RELA 4
#define SHT_REL  9

static const char* elf_type_tostr(uint16_t type) {
  switch (type) {
    case ET_EXEC:
      return "ET_EXEC";
    case ET_DYN:
      return "ET_DYN";
    default:
      return "(unknown)";
  }
}

static const char* reloc_type_tostr(size_t reloc_type) {
  switch(reloc_type) {
    case R_ARM_REL32:
      return "R_ARM_REL32";
    case R_ARM_GLOB_DAT:
      return "R_ARM_GLOB_DAT";
    case R_ARM_JUMP_SLOT:
      return "R_ARM_JUMP_SLOT";
    case R_ARM_RELATIVE:
      return "R_ARM_RELATIVE";
    default:
      return "(unknown)";
  }
}

static const char* sym_bind_tostr(unsigned char bind) {
  switch(bind) {
    case SYM_BIND_LOCAL:
      return "LOCAL";
    case SYM_BIND_GLOBAL:
      return "GLOBAL";
    case SYM_BIND_WEAK:
      return "WEAK";
    default:
      return "(unknown)";
  }
}

static const char* sym_type_tostr(unsigned char type) {
  switch (type) {
    case SYM_TYPE_NOTYPE:
      return "NOTYPE";
    case SYM_TYPE_OBJECT:
      return "OBJECT";
    case SYM_TYPE_FUNC:
      return "FUNC";
    case SYM_TYPE_SECTION:
      return "SECTION";
    case SYM_TYPE_FILE:
      return "FILE";
    case SYM_TYPE_COMMON:
      return "COMMON";
    case SYM_TYPE_TLS:
      return "TLS";
    default:
      return "(unknown)";
    }
}

extern uint8_t code_page[CODE_PAGE_SIZE];

static void checked_lseek(int filedes, off_t offset, int whence) {
  off_t new_pos = lseek(filedes, offset, whence);
  if (new_pos != offset) {
    PRINT_EXIT("Couldn't seek to offset %u\n", offset);
  }
}

static void get_section_name(int elf,
                             size_t name_table_offset,
                             size_t name_offset,
                             uint16_t idx,
                             char* name) {
  checked_lseek(elf, name_table_offset+name_offset, SEEK_CUR);
  ssize_t got = read(elf, name, MAX_SECTION_NAME_LEN);
  // This check is a bit weird, assuming that the name table
  // isn't at the end of the file here.
  if (got != MAX_SECTION_NAME_LEN) {
    PRINT_EXIT("Failed to read name for section %u\n", idx);
  }
}

static void check_elf_hdr(const ElfHeader* header) {
  if ((header->e_ident[0] != 0x7F) ||
      (header->e_ident[1] != 'E')  ||
      (header->e_ident[2] != 'L')  ||
      (header->e_ident[3] != 'F')) {
    PRINT_EXIT("Magic bytes don't match an ELF file\n");
  }

  // Check endian before we check anything endian dependent
  if (header->e_ident[6] != ELFDATA2LSB) {
    PRINT_EXIT("ELF must be little endian\n");
  }

  if (header->e_machine != EXPECTED_MACHINE) {
    PRINT_EXIT("e_machine does not match expected value");
  }

  if (!header->e_shoff) {
    PRINT_EXIT("ELF has no section table\n");
  }

  DEBUG_MSG_ELF("ELF type is %s (%u)\n",
    elf_type_tostr(header->e_type), header->e_type);

  switch (header->e_type) {
    case ET_EXEC:
    case ET_DYN:
      break;
    default:
      PRINT_EXIT("Unexpected ELF type %u\n", header->e_type);
  }

  if (header->e_shstrndx == SHN_UNDEF) {
    PRINT_EXIT("No section name string table\n");
  }
}

// TODO: how to do this more generally?
typedef struct {
  const char* name;
  size_t value;
} KernelSymbolInfo;
static const KernelSymbolInfo kernel_symbols[] = {
  {"config", (size_t)&config},
  {"log_event", (size_t)log_event},
  {"yield_next", (size_t)yield_next},
  {"add_named_thread", (size_t)add_named_thread},
};

static size_t get_kernel_symbol_value(const char* name) {
  size_t num_symbols = sizeof(kernel_symbols)/sizeof(KernelSymbolInfo);
  for (size_t idx=0; idx < num_symbols; ++idx) {
    // TODO: are you supposed to use the names here?
    // Maybe you can just compare indexes and name table index
    if (!strcmp(kernel_symbols[idx].name, name)) {
      DEBUG_MSG_ELF("Resolved kernel symbol \"%s\" to value 0x%x\n",
        name, kernel_symbols[idx].value);
      return kernel_symbols[idx].value;
    }
  }
  PRINT_EXIT("Couldn't find address for symbol \"%s\"\n", name);
  __builtin_unreachable();
}

static SectionHeader get_section_header(
    int elf, uint16_t idx, size_t section_table_offs,
    size_t section_hdr_size) {
  off_t hdr_pos = section_table_offs + (section_hdr_size*idx);
  checked_lseek(elf, hdr_pos, SEEK_CUR);

  SectionHeader section_hdr;
  ssize_t got = read(elf, &section_hdr, section_hdr_size);
  if (got < section_hdr_size) {
    PRINT_EXIT("Couldn't read header for section %u\n", idx);
  }
  return section_hdr;
}

static SymbolInfo get_symbol_info(int elf,
                            uint16_t table_idx, size_t sym_idx,
                            size_t section_table_offs,
                            size_t section_hdr_size,
                            size_t section_name_table_offset) {
  SectionHeader sym_table_hdr = get_section_header(
    elf, table_idx, section_table_offs, section_hdr_size);

  char sym_table_name[MAX_SECTION_NAME_LEN];
  get_section_name(elf, section_name_table_offset,
                   sym_table_hdr.sh_name,
                   table_idx, sym_table_name);
  DEBUG_MSG_ELF("Reading symbol info from \"%s\"\n", sym_table_name); //!OCLINT

  SectionHeader sym_name_table_hdr = get_section_header(
    elf, sym_table_hdr.sh_link, section_table_offs, section_hdr_size);
  char sym_name_table_name[MAX_SECTION_NAME_LEN];
  get_section_name(elf, section_name_table_offset,
                   sym_name_table_hdr.sh_name,
                   sym_table_hdr.sh_link, sym_name_table_name);
  DEBUG_MSG_ELF("Symbol names are in section \"%s\"\n", sym_name_table_name); //!OCLINT

  if (sym_table_hdr.sh_entsize != sizeof(ELFSymbol)) {
    PRINT_EXIT("Symbol table entsize doesn't match struct size (%u vs %u)\n",
      sym_table_hdr.sh_entsize, sizeof(ELFSymbol));
  }

  ELFSymbol symbol;
  size_t symbol_offset = sym_table_hdr.sh_offset + (sym_idx*sym_table_hdr.sh_entsize);
  checked_lseek(elf, symbol_offset, SEEK_CUR);
  ssize_t got = read(elf, &symbol, sym_table_hdr.sh_entsize);
  if (got < sym_table_hdr.sh_entsize) {
    PRINT_EXIT("Couldn't read symbol at index %u\n", sym_idx);
  }

  SymbolInfo sym_info;

  checked_lseek(elf, sym_name_table_hdr.sh_offset+symbol.st_name, SEEK_CUR);
  got = read(elf, sym_info.name, MAX_SYMBOL_NAME_LEN);
  if (got < MAX_SYMBOL_NAME_LEN) {
    // TODO: read until null terminator function?
    // TODO: or just give in and us the heap
    PRINT_EXIT("Failed to read symbol name for index %u\n", sym_idx);
  }

  sym_info.idx = sym_idx;
  sym_info.bind = SYM_BIND(symbol.st_info);
  sym_info.type = SYM_TYPE(symbol.st_info);
  sym_info.symbol = symbol;

  DEBUG_MSG_ELF("|Symbol info\n");
  DEBUG_MSG_ELF("|------------|\n");
  DEBUG_MSG_ELF("|      Index |  %u\n",      sym_info.idx);
  DEBUG_MSG_ELF("|       Name |  \"%s\"\n",  sym_info.name);
  DEBUG_MSG_ELF("|Name offset |  %u\n",      sym_info.symbol.st_name);
  DEBUG_MSG_ELF("|      Value |  0x%x\n",    sym_info.symbol.st_value);
  DEBUG_MSG_ELF("|       Size |  %u\n",      sym_info.symbol.st_size);
  DEBUG_MSG_ELF("|       Type |  %s (%u)\n", sym_type_tostr(sym_info.type), sym_info.type);
  DEBUG_MSG_ELF("|       Bind |  %s (%u)\n", sym_bind_tostr(sym_info.bind), sym_info.bind);

  return sym_info;
}

__attribute__((
  annotate("oclint:suppress[high cyclomatic complexity]"),
  annotate("oclint:suppress[high ncss method]"),
  annotate("oclint:suppress[high npath complexity]"),
  annotate("oclint:suppress[long method]")))
static void resolve_relocs(int elf, uint16_t idx,
                         size_t section_table_offs,
                         size_t section_hdr_size,
                         size_t name_table_offset,
                         bool is_shared) {
  SectionHeader section_hdr = get_section_header(
      elf, idx, section_table_offs, section_hdr_size);

  char name[MAX_SECTION_NAME_LEN];
  get_section_name(elf, name_table_offset, section_hdr.sh_name,
                   idx, name);

  if (section_hdr.sh_type == SHT_REL) {
    // So far Arm uses SHT_REL
    DEBUG_MSG_ELF(">>>>>>>> Section \"%s\" (%u) has relocations of type SHT_REL (%u)\n",
      name, idx, SHT_REL);
  } else if(section_hdr.sh_type == SHT_RELA) {
    PRINT_EXIT("Section %s has unsupported relocation type SHT_RELA! (%u)\n", name, SHT_RELA);
  } else {
    DEBUG_MSG_ELF("No relocations in section \"%s\" (%u)\n", name, idx); //!OCLINT
    return;
  }

  DEBUG_MSG_ELF("\"%s\" is linked to symbol table at section index %u\n", //!OCLINT
    name, section_hdr.sh_link);

  size_t section_end = section_hdr.sh_offset + section_hdr.sh_size;
  size_t reloc_size = sizeof(ELFRelocation);
  for (size_t offs=section_hdr.sh_offset, reloc_idx=0;
       offs < section_end;
       offs+=reloc_size,++reloc_idx) {
    checked_lseek(elf, offs, SEEK_CUR);
    ELFRelocation reloc;
    ssize_t got = read(elf, &reloc, reloc_size);
    if (got != reloc_size) {
      PRINT_EXIT("Failed to read reloc %u in section %s\n", reloc_idx, name);
    }

    size_t reloc_type = RELOC_TYPE(reloc.r_info);
    size_t reloc_sym = RELOC_SYM(reloc.r_info);
    DEBUG_MSG_ELF(">>>> Processing relocation %u\n", reloc_idx);
    DEBUG_MSG_ELF("| Relocation Info\n");
    DEBUG_MSG_ELF("|--------------|\n");
    DEBUG_MSG_ELF("| Type         | %s (%u)\n", reloc_type_tostr(reloc_type), reloc_type);
    DEBUG_MSG_ELF("| Symbol Index | %u\n", reloc_sym);
    DEBUG_MSG_ELF("| Offset       | 0x%x\n", reloc.r_offset);

    SymbolInfo sym_info = get_symbol_info(elf, section_hdr.sh_link, reloc_sym,
                    section_table_offs,
                    section_hdr_size,
                    name_table_offset);

    if (!strcmp(sym_info.name, "")) {
      // Relocations start with one to empty symbol, ignore it
      // TODO: is this the dynamic linker callback address?
      DEBUG_MSG_ELF("Note: Skipping relocation\n", sym_info.name);
      continue;
    }

    size_t symbol_value;

    // Relocations can be against symbols in the kernel, or the binary itself
    if (sym_info.symbol.st_shndx != SHN_UNDEF) {
      // Must be something in some section of this file
      // Which needs to be offset by code_page to get the final value
      symbol_value = sym_info.symbol.st_value + (size_t)code_page;
      DEBUG_MSG_ELF("Resolved symbol \"%s\" to value 0x%x\n",
        sym_info.name, symbol_value);
    } else {
      // Should be in the kernel (hard error if we don't find it)
      symbol_value = get_kernel_symbol_value(sym_info.name);
    }

#ifdef __thumb__
    // Going to ignore Thumb on Arm, assume matching kernel and program
    if (sym_info.type == SYM_TYPE_FUNC) {
      symbol_value |= 1;
    }
#endif

    // This is where we put the answer to the relocation's question
    size_t* reloc_result_location = (size_t*)(code_page + reloc.r_offset);

    // See Arm ELF spec for more inf
    switch(reloc_type) {
      case R_ARM_JUMP_SLOT:
      case R_ARM_GLOB_DAT:
        // (S + A) | T
        *reloc_result_location = symbol_value;
        break;
      case R_ARM_REL32:
        // ((S + A) | T) - P
        // Where P is where we are going to write the relocation result
        *reloc_result_location = symbol_value - (size_t)reloc_result_location;
        break;
      default:
        PRINT_EXIT("Unhandled relocation type %u (%s)\n",
          reloc_type, reloc_type_tostr(reloc_type));
    }

    DEBUG_MSG_ELF("Set final relocation value to 0x%x\n", *reloc_result_location);
  }

  DEBUG_MSG_ELF(">>>>>>>> Finished processing relocations for section \"%s\" (%u)\n",
    name, idx);
}

__attribute__((
  annotate("oclint:suppress[high cyclomatic complexity]"),
  annotate("oclint:suppress[high ncss method]"),
  annotate("oclint:suppress[high npath complexity]"),
  annotate("oclint:suppress[long method]")))
static bool load_section(int elf, uint16_t idx,
                         size_t section_table_offs,
                         size_t section_hdr_size,
                         size_t name_table_offset,
                         void* dest,
                         bool is_shared) {
  SectionHeader section_hdr = get_section_header(
      elf, idx, section_table_offs, section_hdr_size);

  checked_lseek(elf, name_table_offset+section_hdr.sh_name,
    SEEK_CUR);
  ssize_t max_section_name_len = 80;
  char name[max_section_name_len];
  ssize_t got = read(elf, name, max_section_name_len);
  if (got != max_section_name_len) {
    PRINT_EXIT("Failed to read name for section %u\n", idx);
  }

  if (!(section_hdr.sh_flags & SHF_ALLOC)) { //!OCLINT
    DEBUG_MSG_ELF("Skipping non ALLOC section \"%s\" (%u)\n", name, idx); //!OCLINT
    return false;
  }

  void* start_code_page = (void*)code_page;
  void* end_code_page = (void*)&code_page[CODE_PAGE_SIZE];

  void* section_start = section_hdr.sh_addr;
  if (is_shared) {
    // Shared executables have a base address of zero
    // but will be run from the code page as normal
    section_start = (void*)((size_t)section_start + (size_t)start_code_page);
  }

  if (
      (section_start < start_code_page) ||
      (section_start >= end_code_page)) {
    PRINT_EXIT("Section \"%s\" (%u) start address not within code page\n", name, idx);
  }

  void* section_end = (void*)((size_t)(section_start) + section_hdr.sh_size);
  if (section_end >= end_code_page) {
    PRINT_EXIT("Section \"%s\" (%u) extends off of the end of the code page\n", name, idx);
  }

  checked_lseek(elf, section_hdr.sh_offset, SEEK_CUR);

  // Binaries are built to run at the code page
  // so remove that offset from section addresses
  // (PIE program addresses were offset by code_page above so safe to offset as well)
  void* dest_addr = dest+(section_start-start_code_page);

  // Copy from ELF file to destination page
  // (which can be different from code_page)
  ssize_t section_got = read(elf, dest_addr, section_hdr.sh_size);
  if (section_got != section_hdr.sh_size) {
    PRINT_EXIT("Couldn't read content for section \"%s\" (%u)\n", name, idx);
  }
  DEBUG_MSG_ELF("Loaded %u bytes from section \"%s\" (%u)\n", //!OCLINT
    section_got, name, idx);

  return true;
}

__attribute__((
  annotate("oclint:suppress[high cyclomatic complexity]"),
  annotate("oclint:suppress[high ncss method]"),
  annotate("oclint:suppress[high npath complexity]")))
void (*load_elf(const char* filename, void* dest))(void) {
  /* Validate elf and load all ALLOC sections into location
     dest. Returns a function pointer to the ELF entry point.
  */
  DEBUG_MSG_ELF_SECTION("Opening and validating file");
  DEBUG_MSG_ELF("Processing \"%s\"\n", filename); //!OCLINT
  int elf = open(filename, O_RDONLY);
  if (elf < 0) {
    PRINT_EXIT("Couldn't open file %s\n", filename);
  }

  size_t header_size = sizeof(ElfHeader);
  ElfHeader elf_hdr;
  ssize_t got = read(elf, &elf_hdr, header_size);
  if (got < header_size) {
    PRINT_EXIT("Couldn't read complete header from %s\n",
      filename);
  }

  check_elf_hdr(&elf_hdr);

  if (sizeof(SectionHeader) != elf_hdr.e_shentsize) {
    PRINT_EXIT("Section header size didn't match struct size\n");
  }

  SectionHeader name_table_hdr = get_section_header(
    elf, elf_hdr.e_shstrndx, elf_hdr.e_shoff,
    elf_hdr.e_shentsize);

  DEBUG_MSG_ELF_SECTION("Loading sections into memory");
  bool loaded_something = false;
  bool is_shared = elf_hdr.e_type == ET_DYN;
  for (uint16_t idx=0; idx < elf_hdr.e_shnum; ++idx) {
     loaded_something |= load_section(
            elf, idx,
            elf_hdr.e_shoff,
            elf_hdr.e_shentsize,
            name_table_hdr.sh_offset,
            dest,
            is_shared);
  }
  if (!loaded_something) {
    PRINT_EXIT("Loaded no sections from \"%s\"\n", filename);
  }

  DEBUG_MSG_ELF_SECTION("Resolving relocations");
  for (uint16_t idx=0; idx < elf_hdr.e_shnum; ++idx) {
    resolve_relocs(elf, idx, elf_hdr.e_shoff,
      elf_hdr.e_shentsize, name_table_hdr.sh_offset,
      is_shared);
  }
  DEBUG_MSG_ELF_SECTION("Finished resolving relocations");

  DEBUG_MSG_ELF("\"%s\" was loaded into ", filename);
  if (dest == code_page) {
    DEBUG_MSG_ELF("code_page (0x%x)\n", code_page);
  } else {
    DEBUG_MSG_ELF("0x%x\n", dest);
  }

  void (*entry_point)(void) = elf_hdr.e_entry;
  if (is_shared) {
    // Entry is relative to zero so offset from code page
    entry_point = (void (*)(void))((size_t)entry_point + (size_t)code_page);
  }
  // Note that Thumb addresses already have the bit set

  DEBUG_MSG_ELF("Entry point will be set to 0x%x\n", entry_point);

  DEBUG_MSG_ELF_SECTION("Finished loading ELF");
  return entry_point;
}
