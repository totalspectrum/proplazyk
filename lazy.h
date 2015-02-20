#ifndef LAZY_H
#define LAZY_H
//
// types of expression evaluation:
// A = apply
//
//  (A (K x)) -> K1_x
//  (A (K1_x y)) -> x (the one saved in K1 x
//  (A (N x))  -> (A (x (A (N-1 x))))
//  (A (S x)) -> S1_x
//  (A (S1_x y)) -> (S2 (x y))
//  (A (S2 (x y)) z) -> (A (x z) (y z))
//
#include <stdbool.h>

typedef enum CellType { CT_A_PAIR, CT_S2_PAIR, CT_C2_PAIR, CT_NUM_PAIR, CT_FUNC, CT_NUM, CT_FREE, CT_PENDING } CellType;
typedef struct cell Cell;

//
// a cell function "func" calculates self(x)
// "A" is the apply node this function was found in,
// which if possible should be overwritten with the new value
//
typedef Cell* (CellFunc)(Cell *A, Cell *self, Cell *x);

typedef struct pair Pair;

struct pair {
    Cell *left;
    Cell *right;
};

typedef struct func Func;

struct func {
    CellFunc *func;
    Cell     *arg;
};

struct cell {
    CellType type;
    unsigned int used;
    union {
        Pair p;
        Func f;
        unsigned int n;
    } u;
};

static inline CellType gettype(Cell *c) { return (CellType)c->type; }
static inline Cell *getleft(Cell *c) { return c->u.p.left; }
static inline Cell *getright(Cell *c) { return c->u.p.right; }
static inline CellFunc *getfunc(Cell *c) { return c->u.f.func; }
static inline Cell *getarg(Cell *c) { return c->u.f.arg; }
static inline unsigned int getnum(Cell *c) { return c->u.n; }
static inline bool getused(Cell *c) { return c->used != 0; }

static inline void settype(Cell *c, CellType k) { c->type = (unsigned int)k; }
static inline void setleft(Cell *c, Cell *x) { c->u.p.left = x; }
static inline void setright(Cell *c, Cell *x) { c->u.p.right = x; }
static inline void setfunc(Cell *c, CellFunc *f) { c->u.f.func = f; }
static inline void setarg(Cell *c, Cell *arg) { c->u.f.arg = arg; }
static inline void setnum(Cell *c, unsigned num) { c->u.n = num; }
static inline void setused(Cell *c, bool yes) { c->used = yes ? 1 : 0; }

/* for freshly allocated cells that should not be reclaimed by gc */
static inline void setpending(Cell *c) { c->type = CT_PENDING; }
static inline bool ispending(Cell *c) { return (c->type == CT_PENDING); }

#define setstack(c, x) setright(c, x)
#define getstack(c) getright(c)

#endif
