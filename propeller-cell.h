#ifndef CELL_H
#define CELL_H

#include <propeller.h>

typedef union cell Cell;

//
// a cell function "func" calculates self(x)
// "A" is the apply node this function was found in,
// which if possible should be overwritten with the new value
//
typedef Cell* (CellFunc)(Cell *A, Cell *self, Cell *x);

typedef struct Pair {
    unsigned int type:3;
    unsigned int used:1;
    unsigned int lp:14; // left pointer
    unsigned int rp:14; // right pointer
} Pair;

typedef struct Num {
    unsigned int type:3;
    unsigned int used:1;
    unsigned int val:28;
} Num;

union cell {
    Pair p;
    Num n;
    uint32_t bits; // force alignment
};

//#define AFUNC static
#define AFUNC _NATIVE

AFUNC CellType gettype(Cell *c) { return (CellType)c->p.type; }
AFUNC Cell *getleft(Cell *c) { return (Cell *)(c->p.lp<<2); }
AFUNC Cell *getright(Cell *c) { return (Cell *)(c->p.rp<<2); }
#define getfunc(c) ((CellFunc *)getleft(c))
#define getarg(c) getright(c)
AFUNC unsigned int getnum(Cell *c) { return c->n.val; }
AFUNC bool getused(Cell *c) { return c->p.used != 0; }

AFUNC void settype(Cell *c, CellType k) { c->p.type = (unsigned int)k; }
AFUNC void setleft(Cell *c, Cell *x) { c->p.lp = ((unsigned)x)>>2; }
AFUNC void setright(Cell *c, Cell *x) { c->p.rp = ((unsigned)x)>>2; }
AFUNC void setnum(Cell *c, unsigned num) { c->n.val = num; }
AFUNC void setused(Cell *c, bool yes) { c->p.used = yes ? 1 : 0; }
#define setfunc(c, f) setleft(c, (Cell *)(f))
#define setarg(c, a) setright(c, a)

#endif
