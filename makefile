all:
	/Users/davidspickett/Downloads/gcc-arm-none-eabi-8-2018-q4-major/arm-none-eabi/bin/as -mcpu=arm926ej-s -g startup.s -o startup.o
	arm-none-eabi-gcc -c -mcpu=arm926ej-s -g main.c -o main.o
	arm-none-eabi-gcc -c -mcpu=arm926ej-s -g thread.c -o thread.o
	/Users/davidspickett/Downloads/gcc-arm-none-eabi-8-2018-q4-major/arm-none-eabi/bin/ld -T linker.ld main.o thread.o startup.o -o main.elf
	/Users/davidspickett/Downloads/gcc-arm-none-eabi-8-2018-q4-major/arm-none-eabi/bin/objcopy -O binary main.elf main.bin
	/Users/davidspickett/Downloads/gcc-arm-none-eabi-8-2018-q4-major/arm-none-eabi/bin/objdump -h main.elf > sections.txt
	/Users/davidspickett/Downloads/gcc-arm-none-eabi-8-2018-q4-major/arm-none-eabi/bin/objdump -d main.elf > disasm.txt
run:
	qemu-system-arm -M versatilepb -m 128M -nographic -kernel main.bin -gdb tcp::1234
clean:
	rm startup.o main.o main.elf main.bin sections.txt disasm.txt
