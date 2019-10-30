#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#ifdef linux
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#else
#include <stddef.h>
#include <sys/types.h>
#include <stdbool.h>

// These are semihosting values, not posix
#define O_RDONLY 0
#define O_WRONLY 6

// Only needed for Linux to make a new file
#define O_CREAT  0
// For Linux to make new file readable
#define S_IRUSR  0

int open(const char* path, int oflag, ...);
ssize_t read(int fildes, void* buf, size_t nbyte);
ssize_t write(int fildes, const void* buf, size_t nbyte);
int remove(const char* path);
int close(int filedes);
void exit(int status);
#endif /* ifdef linux */

#ifdef USE_FS
void init_file_system(void);
void destroy_file_system(void);
void walk(const char* path, char** out);

typedef struct FileInfo {
  const char* name;
  bool is_file;
  struct FileInfo* next;
} FileInfo;
FileInfo* ls_path(const char* path);
void free_ls_result(FileInfo* head);
#endif /* ifdef USE_FS */

#endif /* ifdef FILE_SYSTEM_H */
