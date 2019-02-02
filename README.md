A simple demo showing tasks cooperativley sharing time. 

Mostly C, the assembly parts are in yield.inc. Current build target is 32 bit Arm, so the assembly is for that. It also relies on the size of a pointer being 32 bit.

## Building

Install the toolchain from:

https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads

Install qemu with Arm support, which for MacPorts will be:
```
sudo port install qemu +target_arm
```

Then:
```
make
make run
```
(note that qemu will spam a lot of output so it's better to use a seperate window for running it)

## Example

The included setup creates two threads. The first prints every time it is run and the other prints every 3 times it runs. It looks something like this: (repeated output removed)

```
Thread <HIDDEN>: resuming
Thread <HIDDEN>: thread yielded
Thread <HIDDEN>: scheduling new thread
Thread <HIDDEN>: yielding
Thread        0: resuming
Thread        0: working
<...>
Thread        1: resuming
Thread        1: yielding
<...>
Thread        0: working
<...>
Thread        1: resuming
Thread        1: yielding
<...>
Thread        0: working
<...>
Thread        1: resuming
Thread        1: working
Thread        1: yielding
```

References:

https://balau82.wordpress.com/2010/02/28/hello-world-for-bare-metal-arm-using-qemu/
