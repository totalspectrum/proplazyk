#
# targets to build
#

PROPGCC=propeller-elf-gcc
PROPSRCS=lazy.c FullDuplexSerial.c

all: lazy compiler

lazy: lazy.c parser.c lazy.h
	$(CC) -g -o lazy lazy.c parser.c

compiler: runtime.bin

runtime.bin: runtime.binary
	dd if=runtime.binary of=runtime.bin conv=sync bs=8192 count=1

runtime.binary: runtime.elf
	propeller-load -s runtime.elf

runtime.elf: $(PROPSRCS) lazy.h propeller-cell.h
	$(PROPGCC) -Os -o $@ -DRUNTIME $(PROPSRCS)

FullDuplexSerial.c FullDuplexSerial.h: FullDuplexSerial.spin
	spin2cpp --ccode FullDuplexSerial.spin

clean:
	rm -f *.elf *.bin *.binary *.o FullDuplexSerial.[ch]
