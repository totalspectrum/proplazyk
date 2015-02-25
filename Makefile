#
# Makefile for lazyk compiler for Propeller (and
# interpreters for the PC)
#
# To build for Linux, just do "make"
# To build for Windows, do "make TARGET=win32"
#

#
# general defines
#

ifeq ($(TARGET),win32)
    CC=i586-mingw32msvc-gcc
    EXE=.exe
else ifeq ($(TARGET),rpi)
    CC=arm-linux-gnueabihf-gcc
    EXE=.pi
else
    CC=gcc
    EXE=
endif

PROPGCC=propeller-elf-gcc
#PROPGCC=/opt/parallax.default/bin/propeller-elf-gcc
PROPSRCS=lazy.c FullDuplexSerial.c

all: lazy$(EXE) lazys$(EXE) proplazy$(EXE)

lazy$(EXE): lazy.c parser.c lazy.h
	$(CC) -g -DINTERPRETER -o $@ lazy.c parser.c

lazys$(EXE): lazy.c parser.c lazy.h
	$(CC) -g -DINTERPRETER -DSMALL -o $@ lazy.c parser.c

proplazy$(EXE): compiler.c parser.c lazy.c lazy.h runtime_bin.h fnmap.h
	$(CC) -g -o $@ compiler.c parser.c lazy.c

runtime_bin.h: runtime.binary
	xxd -i runtime.binary > runtime_bin.h

runtime.binary: runtime.elf
	propeller-load -s runtime.elf

runtime.elf: $(PROPSRCS) lazy.h propeller-cell.h
	$(PROPGCC) -Os -o $@ -DRUNTIME $(PROPSRCS)

FullDuplexSerial.c FullDuplexSerial.h: FullDuplexSerial.spin
	spin2cpp --ccode FullDuplexSerial.spin

fnmap.h: runtime.elf
	./mkdefs.sh > fnmap.h

clean:
	rm -f *.elf *.bin *.binary *.o FullDuplexSerial.[ch] fnmap.h *.exe *.pi lazy lazys proplazy


proplazy.zip: lazy.exe lazy.pi proplazy.exe proplazy.pi ab.lazy hello.lazy fib.lazy rot13.lazy Readme.md COPYING.MIT
	zip -r $@ $^ lazier
