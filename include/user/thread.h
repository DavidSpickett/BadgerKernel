#ifndef USER_THREAD_H
#define USER_THREAD_H

Ok we need a common include, or move kernel
header into it's own folder and include kernel/thread.h
to disambigate the names.

#include "thread.h" // TODO: should this really be here?

int add_named_thread(void (*worker)(void), const char* name);
int add_thread(void (*worker)(void));
#if CODE_PAGE_SIZE
int add_thread_from_file(const char* filename);
#endif
int add_named_thread(void (*worker)(void), const char* name);
int add_named_thread_with_args(void (*worker)(), const char* name,
                               ThreadArgs args);

#endif /* ifdef USER_THREAD_H */
