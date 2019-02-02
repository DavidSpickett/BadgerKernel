all:
	~/Downloads/gcc-arm-none-eabi-8-2018-q4-major/bin/arm-none-eabi-gcc main.c --specs=nosys.specs -march=armv8-a -o main.o -O0
	~/Downloads/gcc-arm-none-eabi-8-2018-q4-major/arm-none-eabi/bin/objdump -d main.o > main.txt 
