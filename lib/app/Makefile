ASM_COMPILER=nasm
C_COMPILER=i686-myos-gcc
CPP_COMPILER=i686-myos-g++
INCLUDEDIR=-I ./include -I/home/dana/compiler/usr/include/freetype2
LIBS=-lfreetype
CFLAGS=-w -masm=intel $(INCLUDEDIR)

all:
	# app 1
	$(CPP_COMPILER) $(CFLAGS) -c app1.cpp -o app1.o
	$(C_COMPILER) -T linker.ld -o app1 app1.o libdanaos.a $(LIBS)

clean:
	rm -rf app1
	rm -rf *.o
