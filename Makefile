#
# targets to build
#

PROPGCC=propeller-elf-gcc
#PROPGCC=/opt/parallax.default/bin/propeller-elf-gcc
PROPSRCS=lazy.c FullDuplexSerial.c

all: lazy compiler

lazy: lazy.c parser.c lazy.h
	$(CC) -g -DINTERPRETER -o lazy lazy.c parser.c

compiler: compiler.c parser.c lazy.c lazy.h runtime_bin.h fnmap.h
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
	rm -f *.elf *.bin *.binary *.o FullDuplexSerial.[ch] fnmap.h
