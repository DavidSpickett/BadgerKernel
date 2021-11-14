#include "common/assert.h"
#include "common/print.h"
#include "user/file.h"
#include "user/thread.h"
#include "user/util.h"
#include <string.h>

#define MAX_CMD_LINE_PARTS 20
typedef struct {
  size_t num_parts;
  const char* parts[MAX_CMD_LINE_PARTS];
} ProcessedCmdLine;

static void AddCmdLinePart(ProcessedCmdLine* cmdline, const char* part) {
  cmdline->parts[cmdline->num_parts] = part;
  cmdline->num_parts++;
  assert(cmdline->num_parts <= MAX_CMD_LINE_PARTS);
}

static ProcessedCmdLine split_cmd_line(char* cmd_line) {
  /* Take a long string with whitespace in and make it into
     effectivley a list of strings by null terminating where
     spaces are. */
  ProcessedCmdLine parts = {.num_parts = 0};

  if (!cmd_line || (*cmd_line == '\0')) {
    return parts;
  }

  // If there is data and there is at least one character
  char* curr = cmd_line;
  const char* start_part = cmd_line;
  for (; *curr; ++curr) {
    if (*curr == ' ') {
      // If there is non space chars to actually save
      if (curr != start_part) {
        AddCmdLinePart(&parts, start_part);
      }
      // Which means we'll fill all spaces with null terminators
      *curr = '\0';
      // Start new part
      start_part = curr + 1;
    }
  }

  // Catch leftover part
  if (start_part < curr) {
    AddCmdLinePart(&parts, start_part);
  }

  return parts;
}

/* Note that these all look like main() */
static void help(int argc, char* argv[]);
static void quit(int argc, char* argv[]);
static void run(int argc, char* argv[]);

typedef struct {
  const char* name;
  void (*fn)(int, char*[]);
  const char* help_text;
} BuiltinCommand;
BuiltinCommand builtins[] = {
    {"help", help, "help <builtin name>"},
    {"quit", quit, "Quit the shell"},
    {"run", run, "run <program name>"},
};
const size_t num_builtins = sizeof(builtins) / sizeof(BuiltinCommand);

// Semihosting has no way to 'ls' a dir
// so maintain a manual list
const char* programs[] = {"echo", "ps", "ls"};
const size_t num_programs = sizeof(programs) / sizeof(const char*);

char run_progname[256];
static void run(int argc, char* argv[]) {
  if (argc < 2) {
    printf("run expects at least 1 argument, the program name\n");
    return;
  }

  // Pass on rest of arguments, +/- 1 to skip "run"
  // The program will still expect argv[0] to be its name
  const ThreadArgs args = make_args(argc - 1, argv + 1, 0, 0);

  int tid = INVALID_THREAD;
  // This is mostly for fun, it allows you to fill up all the
  // available threads by doing "run run run ... <something>".
  if (!strcmp(argv[1], "run")) {
    tid = add_named_thread_with_args(run, argv[1], &args);
  } else {
    tid = add_thread_from_file_with_args(argv[1], &args);
  }

  if (tid == INVALID_THREAD) {
    printf("Couldn't load \"%s\"\n", argv[1]);
  } else {
    // Set it as our child so we come back here when done
    set_child(tid);
    yield_to(tid);
  }
}

static void help(int argc, char* argv[]) {
  if (argc > 2) {
    printf("help expects at most 1 command name\n");
    return;
  }

  if (argc == 2) {
    // Builtin help
    for (size_t i = 0; i < num_builtins; ++i) {
      if (!strcmp(argv[1], builtins[i].name)) {
        printf("%s\n", builtins[i].help_text);
        return;
      }
    }
    printf("Unknown builtin \"%s\"\n", argv[1]);
  } else {
    printf("Builtins:\n");
    for (size_t i = 0; i < num_builtins; ++i) {
      printf("%s ", builtins[i].name);
    }
    printf("\nPrograms:\n");
    for (size_t i = 0; i < num_programs; ++i) {
      printf("%s ", programs[i]);
    }
    printf("\n");
  }
}

static void quit(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  exit(0);
}

static void do_command(char* cmd) {
  ProcessedCmdLine parts = split_cmd_line(cmd);
  // Don't run blank/all whitespace lines
  if (!parts.num_parts) {
    return;
  }

  int tid = INVALID_THREAD;
  ThreadArgs args = make_args(parts.num_parts, &parts.parts, 0, 0);

  // Builtins take priority
  for (size_t i = 0; i < num_builtins; ++i) {
    if (!strcmp(parts.parts[0], builtins[i].name)) {
      tid = add_named_thread_with_args(builtins[i].fn, builtins[i].name, &args);
      break;
    }
  }

  // Then programs
  for (size_t i = 0; i < num_programs; ++i) {
    if (strcmp(parts.parts[0], programs[i]) == 0) {
      tid = add_thread_from_file_with_args(programs[i], &args);
      break;
    }
  }

  if (tid != INVALID_THREAD) {
    // So that we return here
    set_child(tid);
    yield_to(tid);
  } else {
    printf("Unknown command \"%s\"\n", cmd);
  }
}

#define PRINT_PROMPT      printf("$ ");
#define MAX_CMD_LINE      256 // Bold assumptions
#define INPUT_BUFFER_SIZE 16

typedef struct {
  // The command line we're building
  char cmd_line[MAX_CMD_LINE];
  // Cursor/edit position in that command line
  size_t pos;
  // Latest chars read from stdin
  char in[INPUT_BUFFER_SIZE];
  // Pointer to the input character we're processing
  char* curr;
} CmdLineState;

static void handle_return(CmdLineState* state) {
  state->cmd_line[state->pos] = '\0';
  state->pos = 0;
  putchar('\n');
  do_command(state->cmd_line);
  PRINT_PROMPT
  state->pos = 0;
}

static void handle_end_of_text(CmdLineState* state) {
  putchar('\n');
  PRINT_PROMPT
  state->pos = 0;
}

static void handle_escape_sequence(CmdLineState* state) {
  // TODO: actual cursor pos. This just skips them so we don't crash
  if (*(state->curr + 1) == '[') {
    switch (*(state->curr + 2)) {
      case 'A': // Up
      case 'B': // Down
      case 'C': // Right / forward
      case 'D': // Left / back
        state->curr += 2;
        break;
      default:
        assert(0); // Shouldn't get here
        __builtin_unreachable();
    }
  } else {
    assert(0 && "Unhandled escape sequence!\n");
  }
}

static void handle_backspace(CmdLineState* state) {
  if (state->pos) {
    state->pos--;
    printf("\b \b");
  }
}

static void handle_char(CmdLineState* state) {
  if (state->pos < MAX_CMD_LINE) {
    state->cmd_line[state->pos] = *state->in;
    ++state->pos;
    putchar(*state->in);
  }
}

static void command_loop(int input) {
  CmdLineState state = {.pos = 0};

  PRINT_PROMPT
  while (1) {
    // -1 for null terminator space
    ssize_t got = read(input, &state.in, INPUT_BUFFER_SIZE - 1);

    // in qemu 2, got = 1 + size of read seems to mean nothing read
    // qemu version >=4 returns 0 when nothing is read
    if (!got || got == INPUT_BUFFER_SIZE) {
      continue;
    }

    // Terminate after new data so we don't reprocess old data
    state.in[got] = '\0';
    for (state.curr = state.in; *state.curr != '\0'; ++state.curr) {
      switch (*state.in) {
        case '\r': // Enter
          handle_return(&state);
          break;
        case 0x03: // Ctrl-C
          handle_end_of_text(&state);
          break;
        case 0x1B:
          handle_escape_sequence(&state);
          break;
        case 0x7F:
          handle_backspace(&state);
          break;
        case 0x04: // Ctrl-D
          quit(0, NULL);
          break;
        default:
          handle_char(&state);
          break;
      }
    }
  }
}

void run_shell() {
  printf("--------------------\n");
  printf("----- BK Shell -----\n");
  printf("--------------------\n");

  int input = open(":tt", O_RDONLY);
  assert((input >= 0) && "Could not open stdin!");
  // TODO: use stdout also?
  command_loop(input);
}

void setup(void) {
  set_kernel_config(0, KCFG_LOG_SCHEDULER | KCFG_LOG_THREADS);
  set_thread_name(CURRENT_THREAD, "shell");
  run_shell();
}
