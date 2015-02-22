#!/bin/sh
echo "struct map { CellFunc *fn; uint32_t addr; } fnmap[] = {"
nm -n runtime.elf | fgrep _func | awk '{printf "    {&%-10s,0x%s},\n",substr($3,2),$1;}'
echo "};"

