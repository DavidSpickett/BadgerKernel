#ifndef COMMON_MACROS_H
#define COMMON_MACROS_H

// Use BK_EXPORT to mark anything that an out of kernel program
// might want to reference.

#ifdef CODE_PAGE_SIZE
// Prevent LTO from removing something if unreferenced.
#define BK_EXPORT __attribute__((used))
#else
// If elf loading is not enabled there's no harm in removing
// unused functions.
#define BK_EXPORT
#endif

#endif /* ifdef COMMON_MACROS_H */
