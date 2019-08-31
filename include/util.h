#ifndef UTIL_H
#define UTIL_H

#ifdef linux
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#else
#include <stddef.h>

#define O_RDONLY 0

int open(char* path, int oflag);
size_t read(int filedes, void* buf, size_t nbyte);
int close(int filedes);
void exit(int status);

#endif

#endif /* ifdef UTIL_H */
