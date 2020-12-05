properties = (
    # Number, Name, #define value, type
    (0, "ID", "ID", "int type"),
    (1, "Name", "NAME", "char type"),
    (2, "Child", "CHILD", "int type"),
    (3, "State", "STATE", "ThreadState type"),
    (4, "Permissions", "PERMISSIONS", "uint16_t type"),
    (5, "Registers", "REGISTERS", "RegisterContext type"),
    (6, "ErrnoPtr", "ERRNO_PTR", "int* type"),
    (7, "PendingSignals", "PENDING_SIGNALS", "uint32_t type"),
    (8, "SignalHandler", "SIGNAL_HANDLER", "void (*type)(uint32_t)"),
)
