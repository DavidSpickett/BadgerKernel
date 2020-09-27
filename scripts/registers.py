import cog


class Register(object):
    def __init__(self, platform_name, generic_name=None, comment=None):
        self.platform_name = platform_name
        self.generic_name = generic_name
        self.comment = comment


contexts = {
    "aarch64": [
        Register("x29"),
        Register("x30", comment="aka lr"),
        Register("x27"),
        Register("x28"),
        Register("x25"),
        Register("x26"),
        Register("x23"),
        Register("x24"),
        Register("x21"),
        Register("x22"),
        Register("x19"),
        Register("x20", generic_name="syscall_num"),
        Register("x17"),
        Register("x18"),
        Register("x15"),
        Register("x16"),
        Register("x13"),
        Register("x14"),
        Register("x11"),
        Register("x12"),
        Register("x9"),
        Register("x10"),
        Register("x7"),
        Register("x8"),
        Register("x5"),
        Register("x6"),
        Register("x3", generic_name="arg3"),
        Register("x4"),
        Register("pc"),
        Register("x2", generic_name="arg2"),
        Register("fpsr"),
        Register("spsr_el1"),
        Register("x0", generic_name="arg0"),
        Register("x1", generic_name="arg1"),
    ],
    "arm": [
        Register("r0", generic_name="arg0"),
        Register("r1", generic_name="arg1"),
        Register("r2", generic_name="arg2"),
        Register("r3", generic_name="arg3"),
        Register("r4"),
        Register("r5"),
        Register("r6"),
        Register("r7"),
        Register("r8", generic_name="syscall_num"),
        Register("r9"),
        Register("r10"),
        Register("r11"),
        Register("r12"),
        Register("lr"),
        Register("pc", comment="aka the exception mode lr"),
        Register("cpsr"),
    ],
    "thumb": [
        Register("r4"),
        Register("r5"),
        Register("r6"),
        Register("r7"),
        Register("r8", generic_name="syscall_num"),
        Register("r9"),
        Register("r10"),
        Register("r11"),
        Register("r0", generic_name="arg0"),
        Register("r1", generic_name="arg1"),
        Register("r2", generic_name="arg2"),
        Register("r3", generic_name="arg3"),
        Register("r12"),
        Register("lr"),
        Register("pc"),
        Register("xpsr"),
    ],
}


def generate_context(platform):
    registers = contexts[platform]
    cog.outl("typedef struct {")
    for reg in registers:
        if reg.generic_name:
            cog.out("  union {")
            if reg.comment:
                cog.outl(" // {}".format(reg.comment))
            else:
                cog.outl("")
            cog.outl("    size_t {};".format(reg.platform_name))
            cog.outl("    size_t {};".format(reg.generic_name))
            cog.outl("  };")
        else:
            cog.out("  size_t {};".format(reg.platform_name))
            if reg.comment:
                cog.outl(" // {}".format(reg.comment))
            else:
                cog.outl("")
    cog.outl("} __attribute__((packed)) RegisterContext;")
