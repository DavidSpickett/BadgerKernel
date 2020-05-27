#arm-none-eabi-gcc -mthumb -mcpu=cortex-m4 demos/loadable.c -I include/ -Wl,--just-symbols=load,-T,loadable_linker.ld -nostdlib --entry=worker -ffreestanding -nostdlib -o loadable
#arm-none-eabi-gcc -mcpu=cortex-a15 demos/loadable.c -I include/ -Wl,--just-symbols=load,-T,loadable_linker.ld -nostdlib --entry=worker -ffreestanding -nostdlib -o loadable
#--build-id prevents it putting an ID section at the start where our entry should be
aarch64-elf-gcc -mcpu=cortex-a57 -mgeneral-regs-only demos/loadable.c -I include/ -Wl,--just-symbols=load,-T,loadable_linker.ld,--build-id=none,-N -nostdlib --entry=worker -ffreestanding -nostdlib -o loadable

#arm-none-eabi-objcopy -O binary loadable loadable.bin
aarch64-elf-objcopy -O binary loadable loadable.bin
binsz=$(wc -c < loadable.bin)
if (( $binsz > 256 )); then
  echo "Program too big for loadable memory!"
  exit 1
fi
