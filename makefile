#PREFIX       = arm-none-eabi
#PLATFORM     = -mcpu=arm926ej-s
#PLATFORM_SRC = arm926ej_s
#RAM_START    = 0x00010000
#QEMU_ARGS    = qemu-system-arm -M versatilepb

PREFIX       = aarch64-linux-gnu
PLATFORM     =
PLATFORM_SRC = aarch64_virt
RAM_START    = 0x40000000
QEMU_ARGS    = qemu-system-aarch64 -M virt -cpu cortex-a57

QEMU = $(QEMU_ARGS) -m 64M -nographic -semihosting -kernel build/demo.elf
INC = -I include/
CFLAGS = -Wall -nostdlib -g -O3

all: test

obj:
	mkdir -p build
	$(PREFIX)-as         $(PLATFORM)              src/hw/$(PLATFORM_SRC)/startup.s -o build/startup.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) src/hw/$(PLATFORM_SRC)/print.c   -o build/print.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) src/hw/$(PLATFORM_SRC)/exit.c    -o build/exit.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) src/hw/$(PLATFORM_SRC)/yield.c   -o build/yield.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) src/thread.c                     -o build/thread.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) main.c                           -o build/main.o

link: obj
	$(PREFIX)-ld -T linker.ld -defsym=ram_start=$(RAM_START) $(wildcard build/*.o) -o build/demo.elf

run: link
	$(QEMU)

test: link
	$(QEMU) -serial file:build/got.log > /dev/null
	diff expected.log build/got.log

clean:
	rm -rf build
