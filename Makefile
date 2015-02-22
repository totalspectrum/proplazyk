#
# targets to build
#

PROPGCC=propeller-elf-gcc
PROPSRCS=lazy.c FullDuplexSerial.c

all: lazy compiler

lazy: lazy.c parser.c lazy.h
	$(CC) -g -o lazy lazy.c parser.c

compiler: runtime.binary

runtime.binary: runtime.elf
	propeller-load -s runtime.elf

runtime.elf: $(PROPSRCS) lazy.h propeller-cell.h
	$(PROPGCC) -Os -o $@ -DRUNTIME $(PROPSRCS)

FullDuplexSerial.c FullDuplexSerial.h: FullDuplexSerial.spin
	spin2cpp --ccode FullDuplexSerial.spin
