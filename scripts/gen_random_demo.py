from textwrap import dedent
from random import choice, randint
from copy import copy


# This can be > MAX_THREADS in the program, we are fuzzing after all
MAX_THREAD_ID = 11
# Max number of initially added threads (they could spawn more themselves)
MAX_INITIAL_THREADS = 10
# Number of levels of thread calling add_thread we can have
ADD_THREAD_DEPTH = 3
# Number of things a given thread can do
MAX_THREAD_ACTIONS = 10
# Allow threads to enable/disable timer interrupts
ENABLE_TIMER = False


class Action(object):
    def __init__(self, name):
        self.name = name

    def emit(self):
        return "{}();".format(self.name)


class ThreadIDAction(Action):
    def emit(self):
        return "{}({});".format(self.name, randint(0, MAX_THREAD_ID))


class AddThreadAction(Action):
    def __init__(self):
        self.name = "add_thread"

        # This is not a proper thread ID, just a way to
        # name a unique worker func
        thread_postfix = 99

        # Total side effects hack
        added_ids = []

    def emit(self):
        # Use classname so we don't get multiple instances of the postfix
        AddThreadAction.thread_postfix += 1
        self.added_ids.append(AddThreadAction.thread_postfix)
        return "{}(thread_work_{});".format(
            self.name, AddThreadAction.thread_postfix)


def gen_worker(thread_num, depth):
    thread_actions = [
      Action("yield"),
      Action("yield_next"),
      ThreadIDAction("yield_to"),
      ThreadIDAction("thread_cancel"),
    ]

    # Timer adds a whole host of issues right now
    if ENABLE_TIMER:
        thread_actions.extend([
         Action("disable_timer"),
         Action("enable_timer"),
        ])

    # Prevent infinite recursion by only adding this for a few steps
    if depth < ADD_THREAD_DEPTH:
        thread_actions.append(AddThreadAction())

    num_actions = randint(0, MAX_THREAD_ACTIONS)
    work_actions = [choice(thread_actions) for n in range(num_actions)]
    work_actions = "\n  ".join([a.emit() for a in work_actions])

    return dedent('''\
      void thread_work_{}(void) {{
        {}
      }}
    ''').format(thread_num, work_actions)


def gen_workers(num_threads):
    workers = [gen_worker(n, 0) for n in range(num_threads)]

    # Recurse to find added threads
    added_thread_ids = copy(AddThreadAction.added_ids)
    depth = 1
    while AddThreadAction.added_ids:
        AddThreadAction.added_ids = []
        # Added thread work funcs must be defined *before* add_thread is called
        workers = [gen_worker(n, depth) for n in added_thread_ids] + workers
        added_thread_ids = copy(AddThreadAction.added_ids)
        depth += 1

    return "\n".join(workers)


def gen_add_threads(num_threads):
    return '\n  '.join(
      ["add_thread(thread_work_{});".format(n) for n in range(num_threads)]
      )


def gen_demo(num_threads):
    return dedent('''\
      void setup(void) {{
        {}
      }}''').format(gen_add_threads(num_threads))


def gen_file(num_threads):
    return "\n".join([
      "/*****************************/",
      "/**** AUTO GENERATED TEST ****/",
      "/*****************************/",
      "",
      "#include \"thread.h\"",
      "#include \"timer.h\"",
      "",
      gen_workers(num_threads),
      gen_demo(num_threads),
      ])


if __name__ == "__main__":
    num_threads = randint(0, MAX_INITIAL_THREADS)
    print(gen_file(num_threads))
