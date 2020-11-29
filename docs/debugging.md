# Debugging

## Debug Settings

AMT always builds with debug symbols (`-g`) but the higher optimisation levels can be very difficult to follow.

The recommended setting is `-O0` with LTO disabled.

## Basic Debugging

Each AMT demo has a `debug_<demo>` target.

```
$ make debug_yielding
[100%] Built target yielding
```

This will start QEMU running the kernel but stop at the first instruction and wait for a connection to QEMU's internal gdbserver. This server is running on port 1234 of the local machine.

Connect to it like so:
```
$ gdb-multiarch yielding
<...>
Reading symbols from yielding...done.
(gdb) target remote :1234
Remote debugging using :1234
_Reset ()
    at <...>/ARMMultiTasking/src/hw/aarch64_virt/startup.s:6
6         mov x1, #(0x3 << 20)
```

Remember that the kernel is already resident inside QEMU. So you'll do a continue (`c`) not a `run` like you might be used to debugging native applications.

Tip: You can use any gdb that supports the target architecture but `gdb-multiarch` combines a few and is very convenient.

## Catching UBSAN Errors

UBSAN will catch undefined behaviour at runtime and tell you about it. For example:
```
+  char* p = NULL;
+  *p = 'f';
```

```
$ make run_yielding
<...>
UBSAN: type_mismatch_v1 @ <...>/ARMMultiTasking/src/kernel/thread.c:362:6
```

Since our UBSAN is very minimal we can't show you exactly what the expected type and the type we got is here.
(in this case it's a nonnull ptr vs a null ptr)

However you can break on the handlers that print those messages.

```
(gdb) rbreak __ubsan_handle_.*
Breakpoint 1 at 0xc78c: file <...>/ARMMultiTasking/src/common/ubsan.c, line 28.
void __ubsan_handle_add_overflow(SourceInfo *, void *,
    void *);
<...>
(gdb) c
Continuing.

Breakpoint 12, __ubsan_handle_type_mismatch_v1 (
    s=0x40000ee0, a=0x0 <_Reset>, b=0xffffffffffffffd0)
    at <...>/ARMMultiTasking/src/common/ubsan.c:38
38      ubhandler(type_mismatch_v1, void* a, void* b);
(gdb) bt
#0  __ubsan_handle_type_mismatch_v1 (s=0x40000ee0,
    a=0x0 <_Reset>, b=0xffffffffffffffd0)
    at <...>ARMMultiTasking/src/common/ubsan.c:38
#1  0x00000000000033a0 in choose_next_thread ()
    at <...>/ARMMultiTasking/src/kernel/thread.c:362
#2  0x0000000000003748 in do_scheduler ()
    at <...>/ARMMultiTasking/src/kernel/thread.c:391
#3  0x0000000000008a74 in load_next_thread ()
    at <...>/ARMMultiTasking/src/hw/aarch64_virt/yield.S:153
```

Then you can inspect the parent frame to find the issue.

```
(gdb) up
#1  0x00000000000033a0 in choose_next_thread ()
    at <...>/ARMMultiTasking/src/kernel/thread.c:362
362       *p = 'f';
(gdb) p p
$1 = 0x0 <_Reset>
```

From this we can see that p is null which is not a good thing. There can still be a bit of guess work but usually it's quite obvious.

## Quitting QEMU Manually

When a demo doesn't want to exit nicely you can exit QEMU using it's monitor. Type `ctrl-a` then `x`. (release `ctrl` for the `x`)

Sometimes your terminal will get messed up due to the way a demo exits, to fix this just run `reset`.

## Stepping Through Mode Switches

If you're following a syscall from user mode to kernel and back you'll find that gdb will behave weirdly sometimes.

You can single step over/into things like supervisor calls but others like returning from interrupt it seems to lose its place and just run freely.

(this may vary with target architectures and versions of gdb)

If you have this issue then the best idea is to breakpoint on both sides of the divide. If that's returning from interrupt then step to the return point and break back in user space where you expect to hit.

## Logging Failed Syscalls

If your program is silently failing you may be calling a syscall incorrectly and not checking errno.

You can enable logging of these situations with:
```
set_kernel_config(KCFG_LOG_FAILED_ERRNO, 0);
```

With this set any subsequent syscalls that set errno (that fail) will be logged. It looks like:
```
Thread            0: Syscall get_msg failed with errno 5 (invalid arguments)!
```

Note:
* This is not a replacement for good error handling. Check errno if it's important to your code!
* Our errno usage is not complete yet, so some syscalls may not use it where you think they would. Check the source to be sure.

## General Troubleshooting Steps

There are some issues that aren't obvious to the developer. If you're scratching your head over some weird issue try these:
* Make sure the demo has the right number of threads allocated to it. (see it's `CMakeLists.txt`)
* Try different optimisation settings. If lower optimisation works then it could be that some of your C is not quite correct and is relying on compiler behaviour.
* Try a different architecture. Particularly for Thumb if it fails there but nowhere else you probably have a mode bit issue.
* For assembly code check that you have all the right clobbers. (including `memory`)
* Enable UBSAN if you haven't already.
* Try allowing more stack space per thread (see the main `CMakeLists.txt`). Stack corruption can cause a whole host of weird issues that aren't worth trying to step through. (of course you should think about *why* you need more space)
* Same applies to the kernel except this time you need to change the linker script.