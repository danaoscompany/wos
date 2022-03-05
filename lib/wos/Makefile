ASM_SOURCES=$(wildcard *.asm)
ASM_OBJECTS=$(ASM_SOURCES:.asm=.o)
ASM_COMPILER=nasm
C_SOURCES=$(wildcard *.c)
C_OBJECTS=$(C_SOURCES:.c=.o)
C_COMPILER=i386-wos-gcc
AR=i386-wos-ar
LD=i386-wos-ld
INCLUDEDIR=-I./include -I/home/dana/wos/usr/include/freetype2
CFLAGS=-w -fpermissive $(INCLUDEDIR)

all: $(C_OBJECTS) $(ASM_OBJECTS)
	$(AR) rcs libwos.a $^
	cp libwos.a /home/dana/wos/lib

%.o: %.c
	$(C_COMPILER) $(CFLAGS) -c $^ -o $@

%.o: %.asm
	$(ASM_COMPILER) -f elf32 -o $@ $^

clean:
	rm -rf *.o
