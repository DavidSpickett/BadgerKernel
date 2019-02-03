prefix = arm-none-eabi
platform = -mcpu=arm926ej-s

all:
	$(prefix)-as $(platform) -g startup.s -o startup.o
	$(prefix)-gcc -c $(platform) -g main.c -o main.o -Wall
	$(prefix)-gcc -c $(platform) -g thread.c -o thread.o -Wall
	$(prefix)-ld -T linker.ld main.o thread.o startup.o -o main.elf
	$(prefix)-objcopy -O binary main.elf main.bin
	$(prefix)-objdump -h main.elf > sections.txt
	$(prefix)-objdump -d main.elf > disasm.txt
run:
	qemu-system-arm -M versatilepb -m 128M -nographic -kernel main.bin -gdb tcp::1234
clean:
	rm startup.o main.o main.elf main.bin sections.txt disasm.txt thread.o serial.o
