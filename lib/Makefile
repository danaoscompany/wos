ASM_SOURCES=$(wildcard *.asm)
ASM_OBJECTS=$(ASM_SOURCES:.asm=.o)
ASM_COMPILER=nasm
CPP_SOURCES=$(wildcard *.cpp)
CPP_OBJECTS=$(CPP_SOURCES:.cpp=.o)
C_COMPILER=i386-myos-gcc
CPP_COMPILER=i386-myos-gcc
AR=i386-myos-ar
INCLUDEDIR=-I. -I/home/dana/compiler/usr/include/freetype2
CFLAGS=-w -fpermissive -masm=intel $(INCLUDEDIR)

all: $(CPP_OBJECTS) $(ASM_OBJECTS)
	$(AR) rcs libdanaos.a $^
	cp *.h app/include
	cp libdanaos.a app

%.o: %.cpp
	$(CPP_COMPILER) $(CFLAGS) -c $^ -o $@

%.o: %.asm
	$(ASM_COMPILER) -f elf32 -o $@ $^

clean:
	rm -rf *.o
