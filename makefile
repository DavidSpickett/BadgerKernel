PREFIX = arm-none-eabi
PLATFORM = -mcpu=arm926ej-s
PLATFORM_SRC = arm926ej_s
CFLAGS = -Wall -nostdlib
INC = -I include/
QEMU = qemu-system-arm -M versatilepb -m 64M -nographic -semihosting -kernel build/demo.elf

all: test

obj:
	mkdir -p build
	$(PREFIX)-as         $(PLATFORM)              src/hw/$(PLATFORM_SRC)/startup.s  -o build/startup.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) src/hw/$(PLATFORM_SRC)/print.c -o build/print.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) src/hw/$(PLATFORM_SRC)/exit.c  -o build/exit.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) src/thread.c                   -o build/thread.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) main.c                         -o build/main.o

link: obj
	$(PREFIX)-ld -T linker.ld $(wildcard build/*.o) -o build/demo.elf

run: link
	$(QEMU)

test: link
	@$(QEMU) > build/got.log
	diff expected.log build/got.log

clean:
	rm -rf build
