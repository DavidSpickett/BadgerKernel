all:
	arm-none-eabi-as -mcpu=arm926ej-s -g startup.s -o startup.o
	arm-none-eabi-gcc -c -mcpu=arm926ej-s -g main.c -o main.o -Wall
	arm-none-eabi-gcc -c -mcpu=arm926ej-s -g thread.c -o thread.o -Wall
	arm-none-eabi-ld -T linker.ld main.o thread.o startup.o -o main.elf
	arm-none-eabi-objcopy -O binary main.elf main.bin
	arm-none-eabi-objdump -h main.elf > sections.txt
	arm-none-eabi-objdump -d main.elf > disasm.txt
run:
	qemu-system-arm -M versatilepb -m 128M -nographic -kernel main.bin -gdb tcp::1234
clean:
	rm startup.o main.o main.elf main.bin sections.txt disasm.txt thread.o
