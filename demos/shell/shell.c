#include "user/thread.h"
#include "thread.h"
#include "user/file.h"
#include "user/util.h"
#include "util.h"
#include "print.h"
#include <string.h>

#define MAX_CMD_LINE_PARTS 20
typedef struct {
  int num_parts;
  const char* parts[MAX_CMD_LINE_PARTS];
} ProcessedCmdLine;
static ProcessedCmdLine split_cmd_line(char* cmd_line) {
  /* Take a long string with whitespace in and make it into
     effectivley a list of strings by null terminating where
     spaces are. */
  ProcessedCmdLine parts;
  parts.num_parts = 0;

  // If there is data and there is at least one character
  if (cmd_line && (*cmd_line != '\0')) {
    char* curr = cmd_line;
    const char* start_part = cmd_line;
    for ( ; *curr; ++curr) {
      if (*curr == ' ') {
        // If there is non space chars to actually save
        if (curr != start_part) {
          // Save current part
          parts.parts[parts.num_parts] = start_part;
          parts.num_parts++;

          if (parts.num_parts >= MAX_CMD_LINE_PARTS) {
            printf("Too many parts to command line!\n");
            exit(1);
          }
        }
        // Which means we'll fill all spaces with null terminators
        *curr = '\0';
        // Start new part
        start_part = curr+1;
      }
    }
    // Catch leftover part
    if (start_part < curr) {
      // TODO: dedupe
      parts.parts[parts.num_parts] = start_part;
      parts.num_parts++;

      if (parts.num_parts >= MAX_CMD_LINE_PARTS) {
        printf("Too many parts to command line!\n");
        exit(1);
      }
    }
  }
  return parts;
}

/* Note that these all look like main() */
static void help(int argc, char* argv[]);
static void quit(int argc, char* argv[]);
static void  run(int argc, char* argv[]);

typedef struct {
  const char* name;
  void (*fn)(int, char*[]);
  const char* help_text;
} BuiltinCommand;
BuiltinCommand builtins[] = {
  {"help", help, "help <builtin name>"},
  {"quit", quit, "Quit the shell"},
  {"run",  run,  "run <program name>"},
};
const size_t num_builtins = sizeof(builtins)/sizeof(BuiltinCommand);

// Semihosting has no way to 'ls' a dir
// so maintain a manual list
const char* programs[] = {"echo", "ps"};
const size_t num_programs = sizeof(programs)/sizeof(const char*);

//TODO: thread names are pointers so some time after
// run() finishes the pointer to the name on the stack is invalid
// So after you run a few more commands you'll see that the name
// of the program's thread has gone. (which is lucky because it
// could point to some non string)
// Again, this workaround breaks if we have background processes
char run_progname[256];
static void run(int argc, char* argv[]) {
  if (argc < 2) {
    printf("run expects at least 1 argument, the program name\n");
    return;
  }
  strcpy(run_progname, argv[1]);

  // TODO: bodge since load_elf hard errors
  int test = open(run_progname, O_RDONLY);
  if (test < 0) {
    printf("Couldn't find application \"%s\"\n", run_progname);
    return;
  }
  close(test);

  // Pass on rest of arguments, +/- 1 to skip "run"
  // The program will still expect argv[0] to be its name
  const ThreadArgs args = make_args(argc-1, argv+1, 0, 0);
  int tid = add_thread_from_file_with_args(run_progname, &args);
  // Set it as our child so we come back here when done
  set_child(tid);
}

static void help(int argc, char* argv[]) {
  if (argc > 2) {
    printf("help expects at most 1 command name\n");
    return;
  }

  if (argc == 2) {
    // Builtin help
    for (size_t i=0; i<num_builtins; ++i) {
      if (!strcmp(argv[1], builtins[i].name)) {
        printf("%s\n", builtins[i].help_text);
        return;
      }
    }
    printf("Unknown builtin \"%s\"\n", argv[1]);
  } else {
    printf("Builtins:\n");
    for (size_t i=0; i<num_builtins; ++i) {
      printf("%s ", builtins[i].name);
    }
    printf("\nPrograms:\n");
    for (size_t i=0; i<num_programs; ++i) {
      printf("%s ", programs[i]);
    }
    printf("\n");
  }
}

static void quit(int argc, char* argv[]) {
  (void)argc; (void)argv;
  exit(0);
}

static void do_command(char* cmd) {
  ProcessedCmdLine parts = split_cmd_line(cmd);
  // Don't run blank/all whitespace lines
  if (!parts.num_parts) {
    return;
  }

  int tid = -1;
  ThreadArgs args = make_args(parts.num_parts, &parts.parts, 0, 0);

  // Builtins take priority
  for (size_t i=0; i<num_builtins; ++i) {
    if (!strcmp(parts.parts[0], builtins[i].name)) {
      tid = add_named_thread_with_args(builtins[i].fn, builtins[i].name, &args);
    }
  }

  // Then programs
  for (size_t i=0; i<num_programs; ++i) {
    if (!strcmp(parts.parts[0], programs[i])) {
      tid = add_thread_from_file_with_args(programs[i], &args);
    }
  }

  if (tid != -1) {
    // So that we return here
    set_child(tid);
    yield_to(tid);
  } else {
    printf("Unknown command \"%s\"\n", cmd);
  }
}

#define PRINT_PROMPT printf("$ ");

#define MAX_CMD_LINE 256 // Bold assumptions
#define INPUT_BUFFER_SIZE 16
static void command_loop(int input) {
  char cmd_line[MAX_CMD_LINE];
  size_t cmd_line_pos = 0;
  PRINT_PROMPT

  char in[INPUT_BUFFER_SIZE];
  while(1) {
    // -1 for null terminator space
    ssize_t got = read(input, &in, INPUT_BUFFER_SIZE-1);

    assert(in[INPUT_BUFFER_SIZE-1] == '\0');

    if (got && (got != INPUT_BUFFER_SIZE)) {
      for (const char* curr = in; *curr != '\0'; ++curr) {
        switch (*curr) {
          case '\r': // Enter
            cmd_line[cmd_line_pos] = '\0';
            cmd_line_pos = 0;
            printf("\n");
            do_command(cmd_line);
            PRINT_PROMPT
            break;
          case 0x03: // End of text ( Ctrl-C )
            cmd_line_pos = 0;
            printf("\n");
            PRINT_PROMPT
            break;
          case 0x1B: // Escape char
            if (*(curr+1) == '[') {
              switch (*(curr+2)) {
                case 'A': // Up
                case 'B': // Down
                  curr += 2; // Ignore
                  break;
                case 'C': // Right / forward
                  // TODO: cursor pos
                  curr += 2;
                  break;
                case 'D': // Left / back
                  curr += 2;
                  break;
              }
            } else {
              printf("Unhandled escape sequence!\n");
              exit(1);
            }
            break;
          case 0x7F: // Backspace
            if (cmd_line_pos) {
              cmd_line_pos--;
              printf("\b \b");
            }
            break;
          case 0x04: // End of transmission (Ctrl-D)
            quit(0, NULL);
            break;
          default:
            if (cmd_line_pos < MAX_CMD_LINE) {
              cmd_line[cmd_line_pos] = *curr;
              ++cmd_line_pos;
              // TODO: directly putchar
              char out[2];
              out[0] = *curr;
              out[1] = '\0';
              printf("%s", out);
            }
            break;
        }
      }
    }
  }
}

void run_shell() {
  printf("---------------------\n");
  printf("----- AMT Shell -----\n");
  printf("---------------------\n");

  int input = open(":tt", O_RDONLY);
  if (input < 0) {
    printf("Couldn't open stdin!\n");
  }
  command_loop(input);
}

void setup(void) {
  KernelConfig cfg = { .log_scheduler=false,
                       .log_threads=false,
                       .destroy_on_stack_err=false};
  k_set_kernel_config(&cfg);
  k_add_named_thread(run_shell, "shell");
}
