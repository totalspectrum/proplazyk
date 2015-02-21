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

#ifdef __propeller__
#include "propeller-cell.h"
#else
#include "linux-cell.h"
#endif

#define setstack(c, x) setright(c, x)
#define getstack(c) getright(c)

/* for freshly allocated cells that should not be reclaimed by gc */
#define setpending(c) settype(c, CT_PENDING)
#define ispending(c) (gettype(c) == CT_PENDING)


#endif
