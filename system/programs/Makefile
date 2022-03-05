all:
	nasm -f elf32 -o start.o start.asm
	#i386-wos-gcc -I/home/dana/wos/include -I/home/dana/wos/include/freetype2 -L/home/dana/wos/lib program1.c -fpermissive -o program1 -w -lwos -lfreetype
	i386-wos-gcc -I/home/dana/wos/include -I/home/dana/wos/include/freetype2 -L/home/dana/wos/lib -c program1.c -o program1.o -lfreetype -w
	i386-wos-gcc -T linker.ld -I/home/dana/wos/include -I/home/dana/wos/include/freetype2 -L/home/dana/wos/lib -o program1 start.o program1.o -nostartfiles -w -fomit-frame-pointer -fno-exceptions -fno-asynchronous-unwind-tables -lfreetype
