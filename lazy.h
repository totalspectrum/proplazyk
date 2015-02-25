/* Lazy K Interpreter/Compiler
 *
 * Copyright 2015 Total Spectrum Software Inc.
 *
 * +--------------------------------------------------------------------
 * ¦  TERMS OF USE: MIT License
 * +--------------------------------------------------------------------
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * +--------------------------------------------------------------------
 */

#ifndef LAZY_H
#define LAZY_H
//
// types of expression evaluation:
//
// basic combinators
//  (K x) -> K1_x
//  (K1_x y) -> x (the one saved in K1 x
//  (S x) -> S1_x
//  (S1_x y) -> (S2 (x y))
//  ((S2 (x y)) z) -> ((x z) (y z))
// number applications
//  (1 x)  -> x
//  (N x)  -> (x (N-1 x))
// cons pair
//  ((C2 (x y)) z) -> ((z x) y)
#include <stdbool.h>

typedef enum CellType { 
    CT_FREE = 0,
    CT_A_PAIR = 1, 
    CT_S2_PAIR = 2,
    CT_C2_PAIR = 3,
    CT_NUM_PAIR = 4,
    CT_FUNC = 5,
    CT_NUM = 6,
    CT_PENDING = 7, 
} CellType;

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

#ifdef __propeller__

#include "FullDuplexSerial.h"
extern FullDuplexSerial ser;
#define USE_FDS

#define putstr(x) FullDuplexSerial_str(&ser, (int32_t)(x))
#define puthex(x) FullDuplexSerial_hex(&ser, (int32_t)(x), 8)
#define putch(x) FullDuplexSerial_tx(&ser, x)
#define getch() FullDuplexSerial_rx(&ser)

#else

#include <stdio.h>
#define putstr(x) fputs((x), stdout)
#define getch() getchar()
#define putch(c) putchar((c))
#endif

#ifndef RUNTIME
#define DEBUG
#endif

#ifdef DEBUG
#define string_(x) #x
#define string(x) string_(x)
#define assert(x) if (!(x)) fatal("assert failed: " string(x))
#else
#define assert(x)
#endif

void fatal(const char *msg);
void PrintTree(Cell *t);

#ifndef INTERPRETER
#define SMALL
#endif

// here is where our usable area starts
#define PROPELLER_BASE 8192
#define PROPELLER_MEM_ADDR (PROPELLER_BASE + 4)

#ifdef SMALL
#define NUMCELLS (5500)
#define ROOT_STACK_SIZE 400
#endif

// number of cells to allocate
#ifndef NUMCELLS
#define NUMCELLS (8*1024*1024)
#endif

#ifndef ROOT_STACK_SIZE
#define ROOT_STACK_SIZE 2560
#endif

extern Cell *alloc_cell(void);
extern void push_root(Cell *);
extern Cell *pop_root();
extern void gc(void);

void mknum(Cell *c, int n);
void mkpair(Cell *c, Cell *X, Cell *Y, CellType t);
#define mks2(c, x, y) mkpair(c, x, y, CT_S2_PAIR)
#define mkc2(c, x, y) mkpair(c, x, y, CT_C2_PAIR)
#define mkapply(c, x, y) mkpair(c, x, y, CT_A_PAIR)
#define mknumpair(c, x, y) mkpair(c, x, y, CT_NUM_PAIR)

extern void mkfunc(Cell *c, CellFunc *func, Cell *arg);

//
// basic CellFuncs
//
CellFunc K1_func;
CellFunc K_func;
CellFunc KI_func;
CellFunc S1_func;
CellFunc S_func;
CellFunc Inc_func;
CellFunc Cons_func;
CellFunc C1_func;
CellFunc C_func;
CellFunc Read_func;

#ifdef RUNTIME
#define g_root *((Cell **)PROPELLER_BASE)
#define mem ((Cell *)(PROPELLER_MEM_ADDR))

#else
extern Cell *g_root;
extern Cell mem[];
Cell *parse_part(const char **str);
Cell *parse_whole(FILE *f);
#endif

//
// various options to control the parser
//
extern bool gl_optimize;

#endif
