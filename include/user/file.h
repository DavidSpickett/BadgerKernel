#ifndef USER_FILE_H
#define USER_FILE_H

#include "file_system.h"

int open(const char* path, int oflag, ...);
ssize_t read(int filedes, void* buf, size_t nbyte);
ssize_t write(int filedes, const void* buf, size_t nbyte);
off_t lseek(int fd, off_t offset, int whence);
int remove(const char* path);
int close(int filedes);
void exit(int status);

#ifdef HAS_FILESYSTEM
// TODO?

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
#endif /* ifdef HAS_FILESYSTEM */

#endif /* ifdef USER_FILM_H */
