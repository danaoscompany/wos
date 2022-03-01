all:
	nasm -f bin -o start.bin boot.asm -Wall
	nasm -f elf32 -o loader_asm.o boot_stage2.asm -Wall
	i686-elf-gcc -w -masm=intel -c loader.c -o loader.o
	i686-elf-gcc -T linker.ld -o loader.bin -ffreestanding -nostdlib loader_asm.o loader.o -lgcc
	cp start.bin ../iso
	cp loader.bin ../iso
