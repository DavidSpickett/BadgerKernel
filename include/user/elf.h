#ifndef USER_ELF_H
#define USER_ELF_H

#include "user/thread.h"

// Thread worker function that loads an ELF specified by filename.
// Then restarts itself running the loaded program with the given
// args and permissions changes.
//
// This is only used by the kernel in response to an add_thread
// syscall.
void load_elf(void* dest, uint16_t remove_permissions, const ThreadArgs* args,
              const char* filename);

#endif /* ifdef USER_ELF_H */
