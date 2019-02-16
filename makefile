PREFIX = arm-none-eabi
PLATFORM = -mcpu=arm926ej-s
CFLAGS = -Wall
INC = -I include/
QEMU = qemu-system-arm -M versatilepb -m 128M -nographic -semihosting -kernel build/demo.bin

all: test

obj:
	mkdir -p build
	$(PREFIX)-as         $(PLATFORM)              src/startup.s             -o build/startup.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) src/thread.c              -o build/thread.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) src/hw/arm926ej_s/print.c -o build/print.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) src/hw/arm926ej_s/exit.c  -o build/exit.o
	$(PREFIX)-gcc $(INC) $(PLATFORM) -c $(CFLAGS) main.c                    -o build/main.o

link: obj
	$(PREFIX)-ld -T linker.ld $(wildcard build/*.o) -o build/demo.elf

objcopy: link
	$(PREFIX)-objcopy -O binary build/demo.elf build/demo.bin

run: objcopy
	$(QEMU)

test: objcopy
	@$(QEMU) > build/got.log
	diff expected.log build/got.log

clean:
	rm -rf build
