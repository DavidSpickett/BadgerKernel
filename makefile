prefix = arm-none-eabi
platform = -mcpu=arm926ej-s
cflags = -Wall
inc = -I include/
qemu = qemu-system-arm -M versatilepb -m 128M -nographic -semihosting -kernel build/demo.bin

all: test

obj:
	mkdir -p build
	$(prefix)-as         $(platform)              src/startup.s             -o build/startup.o
	$(prefix)-gcc $(inc) $(platform) -c $(clfags) src/thread.c              -o build/thread.o
	$(prefix)-gcc $(inc) $(platform) -c $(clfags) src/hw/arm926ej_s/print.c -o build/print.o
	$(prefix)-gcc $(inc) $(platform) -c $(clfags) src/hw/arm926ej_s/exit.c  -o build/exit.o
	$(prefix)-gcc $(inc) $(platform) -c $(clfags) main.c                    -o build/main.o

link: obj
	$(prefix)-ld -T linker.ld $(wildcard build/*.o) -o build/demo.elf

objcopy: link
	$(prefix)-objcopy -O binary build/demo.elf build/demo.bin

run: objcopy
	$(qemu)

test: objcopy
	@$(qemu) > build/got.log
	diff expected.log build/got.log

clean:
	rm -rf build
