syscalls = (
    # Name
    # Whether the call has a return value
    # Whether the call invalidates the user side thread info
    ("add_thread",          True,  False),
    ("get_thread_property", True,  False),
    # TODO: only certain properties invalid user thread info
    ("set_thread_property", True,  True),
    # User gets kernel_config from user_thread_info
    ("set_kernel_config",   False, True),
    ("yield",               True,  False),
    ("restart",             False, True),
    ("get_msg",             True,  False),
    ("send_msg",            True,  False),
    ("thread_wait",         False, False),
    ("thread_wake",         True,  False),
    ("thread_cancel",       True,  False),
    ("mutex",               True,  False),
    # One of the ops does return void but we return
    # true from kernel for that anyway
    ("condition_variable",  True,  False),
    ("open",                True,  False),
    ("read",                True,  False),
    ("write",               True,  False),
    ("lseek",               True,  False),
    ("remove",              True,  False),
    ("close",               True,  False),
    ("exit",                False, False),
    ("malloc",              True,  False),
    ("realloc",             True,  False),
    ("free",                False, False),
    ("list_dir",            True,  False),
)
