#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define SMALL
#include <stdint.h>
#include "lazy.h"
#include "runtime_bin.h"

#define MEM_OFFSET (8196)
Cell mem[NUMCELLS];

uint32_t
convertCellAddr(Cell *c)
{
    size_t idx;
    if (c == NULL) return 0;

    if (c >= &mem[0] && c <= &mem[NUMCELLS]) {
        idx = c - &mem[0];
        idx = MEM_OFFSET + (idx * 4);
        return idx;
    }
    fatal("Unable to convert cell address!");
}

#include "fnmap.h"
uint32_t
convertCellFunc(CellFunc *f)
{
    int i;

    for (i = 0; i < sizeof(fnmap)/sizeof(fnmap[0]); i++) {
        if (f == fnmap[i].fn) {
            return fnmap[i].addr;
        }
    }
    fatal("Unable to convert cell function\n");
    return 0;
}

// convert Propeller address to 14 bits
#define ADDR(x) (((x)>>2) & 0x3fff)

uint32_t
convertCell(Cell *x)
{
    CellType t;
    uint32_t c = 0;
    uint32_t left, right;

    t = gettype(x);
    c = ((uint32_t)t) << 1;
    left = right = 0;

    switch (t) {
    case CT_A_PAIR:
    case CT_S2_PAIR:
    case CT_C2_PAIR:
    case CT_NUM_PAIR:
        left = ADDR(convertCellAddr(getleft(x)));
        right = ADDR(convertCellAddr(getright(x)));
        c |= (left << 18);
        c |= (right << 4);
        break;
    case CT_NUM:
        c |= (getnum(x) << 4);
        break;
    case CT_FUNC:
        left = ADDR(convertCellFunc(getfunc(x)));
        right = ADDR(convertCellAddr(getarg(x)));
        c |= (left << 18);
        c |= (right << 4);
        break;
    case CT_FREE:
        return 0;
    default:
        fatal("Unable to convert cell type");
        break;
    }
    return c;
}

void *
xmalloc(size_t s)
{
    void *r = malloc(s);
    if (!r) {
        fprintf(stderr, "Out of memory!\n");
        exit(2);
    }
    return r;
}

void
WriteLong(FILE *f, uint32_t x)
{
    int c;
    int i;

    for (i = 0; i < 4; i++) {
        c = (x & 0xff);
        x = x >> 8;
        c = fputc(c, f);
        if (c < 0) {
            fatal("Write error");
        }
    }
}

void
WriteCells(FILE *f)
{
    int i;
    uint32_t cell;

    for (i = 0; i < NUMCELLS; i++) {
        cell = convertCell(&mem[i]);
        WriteLong(f, cell);
    }
}

int main(int argc, char **argv)
{
    FILE *f;
    char *outfile;
    char *ext;

    if (argc != 2) {
        fprintf(stderr, "Usage: compile file.lazy\n");
        return 2;
    }
    f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    g_root = parse_whole(f);
    fclose(f);

    gc();

    outfile = xmalloc(strlen(argv[1]) + 8);
    strcpy(outfile, argv[1]);
    ext = strrchr(outfile, '.');
    if (ext) {
        strcpy(ext, ".binary");
    } else {
        strcat(outfile, ".binary");
    }
    f = fopen(outfile, "wb");
    if (!f) {
        perror(outfile);
    }
    fwrite(runtime_bin, 1, sizeof(runtime_bin), f);
    WriteLong(f, convertCellAddr(g_root));
    WriteCells(f);

    fclose(f);
    return 0;
}
