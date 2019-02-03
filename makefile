prefix = arm-none-eabi
platform = -mcpu=arm926ej-s
inc = -I include/

all:
	$(prefix)-as $(platform) -g src/startup.s -o startup.o
	$(prefix)-gcc $(inc) -c $(platform) -g main.c -o main.o -Wall
	$(prefix)-gcc $(inc) -c $(platform) -g src/thread.c -o thread.o -Wall
	$(prefix)-gcc $(inc) -c $(platform) -g src/hw/arm926ej_s/print.c -o print.o -Wall
	$(prefix)-ld -T linker.ld main.o thread.o startup.o print.o -o main.elf
	$(prefix)-objcopy -O binary main.elf main.bin
	$(prefix)-objdump -h main.elf > sections.txt
	$(prefix)-objdump -d main.elf > disasm.txt
run:
	qemu-system-arm -M versatilepb -m 128M -nographic -kernel main.bin -gdb tcp::1234
clean:
	rm startup.o main.o main.elf main.bin sections.txt disasm.txt thread.o print.o
