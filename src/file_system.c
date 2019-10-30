#include <string.h>
#include <stdint.h>
#include "util.h"
#include "alloc.h"
#include "file_system.h"

static void* checked_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
      printf("malloc failed!\n");
      exit(1);
    }
    return ptr;
}

static void* checked_realloc(void* ptr, size_t size) {
  void* new_ptr = realloc(ptr, size);
  if (!new_ptr) {
    printf("realloc failed!\n");
    exit(1);
  }
  return new_ptr;
} 

typedef struct FileNode {
  char* name;
  // Start of list of children
  struct FileNode* first_child;
  // Next file on the same dir level as this
  struct FileNode* next_peer;
  bool is_file;

  // File stuff
  uint8_t* content;
  size_t size;
  int descriptor;
} FileNode;

FileNode root;
int num_file_descriptors= 0;
FileNode** file_descriptors = NULL;

void init_node(FileNode* node, char* name) {
  node->name = name;
  node->first_child = NULL;
  node->next_peer = NULL;
  node->is_file = false;
  node->content = NULL;
  node->size = 0;
  node->descriptor = -1;
}

int new_file_descriptor(FileNode* node) {
  // First search for an empty slot
  for (int idx = 0; idx < num_file_descriptors; ++idx) {
    if (file_descriptors[idx] == NULL) {
      file_descriptors[idx] = node;
      return idx;
    }
  }

  // Otherwise we need a new one
  int idx = num_file_descriptors;
  num_file_descriptors++;
  file_descriptors = checked_realloc(file_descriptors,
    sizeof(FileNode*)*num_file_descriptors);
  file_descriptors[idx] = node;
  node->descriptor = idx;
  return idx;
}

FileNode* add_child_node(FileNode* node, const char* name) {
  FileNode* child = checked_malloc(sizeof(FileNode));
  // New copy in case this came from a split path
  char* name_copy = checked_malloc(strlen(name)+1);
  strcpy(name_copy, name);
  init_node(child, name_copy);

  // Find end of child list
  FileNode* current = node->first_child;
  FileNode* prev = NULL;

  while (current != NULL) {
    prev = current;
    current = current->next_peer;
  }

  if (prev) {
    // Insert at end
    prev->next_peer = child;
  } else {
    // start new list
    node->first_child = child;
  }

  return child;
}

void init_file_system(void) {
  init_node(&root, "/");
  file_descriptors = NULL;
  num_file_descriptors = 0;
}

void replace_seperator(char* path) {
  /* Replace all slashes with null bytes,
     effectivley giving us a list of string
     parts of the path. */
  while (*path) {
    if (*path == '/') {
      *path = '\0';
    }
    path++;
  }
}

FileNode* find_child(FileNode* node, const char* name) {
  FileNode* next_child = node->first_child;

  while (next_child) {
    if (strcmp(name, next_child->name) == 0) {
      return next_child;
    }
    next_child = next_child->next_peer;
  }

  return NULL;
}

bool remove_child(FileNode* parent, FileNode* child) {
  FileNode* current_child = parent->first_child;
  FileNode* prev = NULL;

  while (current_child) {
    if (current_child == child) {
      if (prev) {
        // We're not the head of the list
        prev->next_peer = current_child->next_peer;
      } else {
        // We're the start of the list
        parent->first_child = current_child->next_peer;
      }

      free(current_child->name);
      if (current_child->is_file) {
        free(current_child->content);
      }
      free(current_child);

      return true;
    }
    prev = current_child;
    current_child = current_child->next_peer;
  }

  return false;
}

void remove_all_children_of(FileNode* parent) {
  FileNode* current_child = parent->first_child;

  while (current_child) {
    remove_all_children_of(current_child);
    // Copies to prevent use after free
    FileNode* next_child = current_child->next_peer;
    // (also frees current_child)
    remove_child(parent, current_child);
    current_child = next_child;
  }
}

void destroy_file_system(void) {
  /* This is only the array of pointers to nodes,
     we don't want to free the nodes themselves here. */
  free(file_descriptors);
  // That's what this will do
  remove_all_children_of(&root);
}

FileNode* find_parent_of(FileNode* current, const FileNode* node) {
  FileNode* current_child = current->first_child;

  while (current_child) {
    if (current_child == node) {
      return current;
    } else if (current_child->first_child) {
      // Search all children of this node
      FileNode* got = find_parent_of(current_child, node);
      if (got) {
        return got;
      }
    }
    current_child = current_child->next_peer;
  }

  return NULL;
}

static FileNode* get_node_from_split_path(const char* path_part,
                                          const char* path_end,
                                          int oflag) {
  FileNode* current = &root;

  while (path_part != path_end) {
    bool last_path_part = (path_part + strlen(path_part)) == path_end;
    FileNode* node = find_child(current, path_part);

    if (node) {
      if (last_path_part) {
        // Found the target
        return node;
      } else {
        // Go down one dir
        current = node;
      }
    } else {
      switch (oflag) {
        case O_RDONLY:
          // invalid path
          return NULL;
        case O_WRONLY:
        {
          // Can't have a subdir of a file
          if (current->is_file) {
            return NULL;
          }

          // Otherwise add a new dir
          FileNode* new_node = add_child_node(current, path_part);

          if (last_path_part) {
            return new_node;
          } else {
            // Continue making dir structure
            current = new_node;
          }
          break;
        }
        default:
          __builtin_unreachable();
      }
    }

    // +1 to skip next null terminator
    path_part += strlen(path_part) + 1;
  } 
   
  return NULL;
}

FileNode* get_node(const char* path, int oflag) {
  // Bit of a cop-out so we can get root
  if (!strcmp(root.name, path)) {
    return &root;
  }

  const char* _path = path+1;
  size_t path_len   = strlen(_path);
  char* split_path  = checked_malloc(path_len+1);
  char* path_end    = split_path + path_len;

  strcpy(split_path, _path);
  replace_seperator(split_path);

  FileNode* got = get_node_from_split_path(split_path, path_end, oflag);
  free(split_path);

  return got;
}

int open(const char* path, int oflag, ...) {
  FileNode* file = get_node(path, oflag);
  if (!file) {
    return -1;
  }

  // Actually make it a file
  file->is_file = true;
  return new_file_descriptor(file);
}

void remove_file_descriptor(FileNode* file) {
  if (file->descriptor != -1) {
    file_descriptors[file->descriptor] = NULL;
    file->descriptor = -1;
  }
}

bool check_fd(int fd) {
  if ((fd < 0) ||
      (fd >= num_file_descriptors) ||
      !file_descriptors[fd]) {
    return false;
  }
  return true;
}

int close(int fildes) {
  if (!check_fd(fildes)) {
    return -1;
  }

  remove_file_descriptor(file_descriptors[fildes]);
  return 0;
}

ssize_t write(int fd, const void *buf, size_t count) {
  if (!check_fd(fd)) {
    return 0;
  } 

  FileNode* file = file_descriptors[fd];
  file->content = checked_realloc(file->content,
                          file->size+count);
  memcpy(file->content+file->size, buf, count); 

  return count;
}

ssize_t read(int fd, void *buf, size_t count) {
  if (!check_fd(fd)) {
    return 0;
  }

  FileNode* file = file_descriptors[fd];
  memcpy(buf, file->content, count);

  return count;
}

int remove(const char *path) {
  FileNode* file = get_node(path, O_RDONLY);
  if (!file) {
    return -1;
  }

  // Close FD (if any)
  remove_file_descriptor(file);

  // recurses down the tree
  remove_all_children_of(file);

  // Delete from parent node
  FileNode* parent = find_parent_of(&root, file);
  remove_child(parent, file);

  return 0;
}

FileInfo* ls_path(const char* path) {
  FileNode* file = get_node(path, O_RDONLY);
  if (!file) {
    return NULL;
  }

  FileInfo* ret = NULL;
  FileInfo* prev = NULL;
  FileInfo* current = NULL;
  FileNode* child = file->first_child;

  while(child) {
    prev = current;
    current = checked_malloc(sizeof(FileInfo));
    if (!ret) {
      ret = current;
    }
    if (prev) {
      prev->next = current;
    }

    current->name = child->name;
    current->is_file = child->is_file;
    // Needs to be init in case there is no next peer
    current->next = NULL;

    child = child->next_peer;
  }

  return ret;
}

void free_ls_result(FileInfo* head) {
  if (!head) {
    return;
  }
  
  while (head) {
    FileInfo* next = head->next;
    free(head);
    head = next;
  }  
}
