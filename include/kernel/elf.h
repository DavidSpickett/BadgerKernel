#ifndef KERNEL_ELF_H
#define KERNEL_ELF_H

void (*load_elf(const char* filename, void* dest))(void);

#endif /* ifdef KERNEL_ELF_H */
