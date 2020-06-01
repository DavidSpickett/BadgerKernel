#include "thread.h"
#include "file_system.h"
#include "print.h"
#include <string.h>

static char* first_whitespace(char* str) {
  char* rest = str;
  while(*rest != ' ') {
    ++rest;
  }
  return rest;
}

static void help(const char* args);
static void quit(const char* args);
static void  run(const char* args);
static void echo(const char* args);

typedef struct {
  const char* name;
  void (*fn)();
} BuiltinCommand;
BuiltinCommand builtins[] = {
  {"help", help},
  {"quit", quit},
  {"run", run},
  {"echo", echo},
};

static void echo(const char* args) {
  printf("%s", args);
}

static void run(const char* args) {
  int tid = add_thread_from_file(args);
  yield_to(tid);
}

static void help(const char* args) {
  (void)args;
  size_t num_commands = sizeof(builtins)/sizeof(BuiltinCommand);
  printf("Available commands are:\n");
  for (size_t i=0; i<num_commands; ++i) {
    printf("%s ", builtins[i].name);
  }
}

static void quit(const char* cmd) {
  (void)cmd;
  exit(0);
}

static void do_command(char* cmd) {
  char* rest = first_whitespace(cmd);
  // Split cmd and arguments
  *rest = '\0';
  size_t num_commands = sizeof(builtins)/sizeof(BuiltinCommand);
  int tid = -1;
  ThreadArgs args = make_args(rest+1, 0, 0, 0);
  for (size_t i=0; i<num_commands; ++i) {
    if (!strcmp(cmd, builtins[i].name)) {
      tid = add_named_thread_with_args(builtins[i].fn, "", args);
    }
  }

  if (tid != -1) {
    yield_to(tid);
  } else {
    printf("Unknown command \"%s\"", cmd);
  }
}

#define PRINT_PROMPT printf("\n$ ");

#define MAX_CMD_LINE 256 // Bold assumptions
static void command_loop(int input) {
  char cmd_line[MAX_CMD_LINE];
  size_t cmd_line_pos = 0;
  PRINT_PROMPT

  char in[2];
  in[1] = '\0';
  while(1) {
    ssize_t got = read(input, &in, 1);

    // Will read nullptr if no data
    if (got && in[0]) {
      switch (in[0]) {
        case '\r': // Enter
          cmd_line[cmd_line_pos] = '\0';
          cmd_line_pos = 0;
          printf("\n");
          do_command(cmd_line);
          PRINT_PROMPT
          break;
        case 0x7F: // Backspace
          if (cmd_line_pos) {
            cmd_line_pos--;
            printf("\b \b");
          }
          break;
        default:
          if (cmd_line_pos < MAX_CMD_LINE) {
            cmd_line[cmd_line_pos] = in[0];
            ++cmd_line_pos;
            printf("%s", in);
          }
          break;
      }
    }
  }
}

static void run_shell() {
  printf("---------------------\n");
  printf("----- AMT Shell -----\n");
  printf("---------------------");

  int input = open(":tt", O_RDONLY);
  if (input < 0) {
    printf("Couldn't open stdin!\n");
  }
  command_loop(input);
}

void setup(void) {
  config.log_scheduler = false;
  config.log_threads = false;
  add_named_thread(run_shell, "shell");
}
