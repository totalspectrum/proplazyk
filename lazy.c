//
// SKN evaluator
//

#include <stdlib.h>
#include "lazy.h"

#ifdef __propeller__
//#define DEBUG_RUNTIME
#endif

Cell *partial_eval(Cell *node);

//
// simple memory allocator
//

#ifndef RUNTIME
// main tree
Cell *g_root;
#endif

#ifdef INTERPRETER
Cell mem[NUMCELLS];
#endif

static Cell *free_list;

void
fatal(const char *msg) {
    putstr(msg); putstr("\r\n");
    abort();
}

// stack of roots that we may have to sweep
static Cell *root_stack[ROOT_STACK_SIZE];
static int root_stack_top;

void push_root(Cell *x) {
    if (root_stack_top >= ROOT_STACK_SIZE) {
        fatal("root stack overflow");
    }
    root_stack[root_stack_top++] = x;
}
Cell * pop_root() {
    Cell *x;

    if (root_stack_top == 0) {
        fatal("root stack underflow");
    }
    --root_stack_top;
    x = root_stack[root_stack_top];
    return x;
}


static void
gc_mark(Cell *root)
{
    CellType t;
    Cell *left, *right;
    Cell *arg;

    if (!root) return;
    if (getused(root)) return;

    setused(root, true);
    t = gettype(root);

    switch(t) {
    case CT_A_PAIR:
    case CT_S2_PAIR:
    case CT_NUM_PAIR:
    case CT_C2_PAIR:
        left = getleft(root);
        right = getright(root);
        if (left) gc_mark(left);
        if (right) gc_mark(right);
        return;
    case CT_FUNC:
        arg = getarg(root);
        if (arg) gc_mark(arg);
        return;
    default:
        return;
    }
}

// in general we shouldn't have many cells allocated but not yet
// assigned types
#define MAX_PENDING 8

static void
gc_sweep(void)
{
    int i;
    bool used;
    Cell *cur;
    int pending_count = 0;

    free_list = NULL;
    // count down so that the free list starts at the bottom of
    // memory; this makes optimizing the compiler output easier
    for (i = NUMCELLS-1; i >= 0; --i) {
        cur = &mem[i];
        used = getused(cur);
        if (used) {
            setused(cur, false); // in preparation for the next mark round
        } else if (ispending(cur)) {
            // allocated but not yet used; do not reclaim it
            pending_count++;
        } else {
            // add to free list
	    settype(cur, CT_FREE);
            setstack(cur, free_list);
            free_list = cur;
        }
    }
    if (pending_count > MAX_PENDING) {
        fatal("unexpectedly high number of pending cells in gc");
    }
}

//
// garbage collection function
//
void gc(void)
{
    int i;

    gc_mark(g_root);
    for (i = 0; i < root_stack_top; i++) {
        gc_mark(root_stack[i]);
    }
    gc_sweep();
}

Cell *alloc_cell() {
    Cell *next = free_list;
    if (!next) {
        gc();
        next = free_list;
    }
    if (!next) {
        fatal("unable to alloc");
    }
    assert(gettype(next) == CT_FREE);
    free_list = getstack(next);
    setpending(next);
    return next;
}

//
// make a cell into a number
//
void
mknum(Cell *c, int n)
{
    settype(c, CT_NUM);
    setnum(c, n);
}

//
// make an s2 pair (eval of SXY)
//
void
mkpair(Cell *c, Cell *X, Cell *Y, CellType t)
{
    settype(c, t);
    setleft(c, X);
    setright(c, Y);
}

//
// make a cell into a function
//
void
mkfunc(Cell *c, CellFunc *func, Cell *arg)
{
    settype(c, CT_FUNC);
    setfunc(c, func);
    setarg(c, arg);
}

//
// function applications
//
// Kx -> (K1 x)
// (K1 x)y -> x
//
Cell *
K1_func(Cell *r, Cell *self, Cell *rhs)
{
  return getarg(self);
}

Cell *
K_func(Cell *r, Cell *self, Cell *rhs)
{
  mkfunc(r, K1_func, rhs);
  return r;
}

//
// special case: KI_func
// KIx = i = 1

Cell *
KI_func(Cell *r, Cell *self, Cell *rhs)
{
  mknum(r, 1);
  return r;
}

//
// Sx -> (S1 x)
// ((S1 x)y) -> (S2 x y)
//
Cell *
S1_func(Cell *r, Cell *self, Cell *rhs)
{
    Cell *lhs;
    lhs = getarg(self);
    mks2(r, lhs, rhs);
    return r;
}

Cell *
S_func(Cell *r, Cell *self, Cell *rhs)
{
  mkfunc(r, S1_func, rhs);
  return r;
}

//
// Increment function
// Must be applied to a number (or something that evaluates to a number)
//
Cell *
Inc_func(Cell *r, Cell *self, Cell *rhs)
{
    int n;

    while (gettype(rhs) == CT_A_PAIR) {
        push_root(rhs);
        rhs = partial_eval(rhs);
        pop_root();
    }

    if (gettype(rhs) != CT_NUM) {
        fatal("Inc called on non-number");
    }
    n = getnum(rhs);

    mknum(r, n+1);
    return r;
}

//
// useful utility to build a cons
// cons is represented by (lambda (f) (f X Y))
// which is then applied to either the car function (K) or cdr (KI)
// \f.``fXY
// we simplify this with a special C2 pair internally
//
//
Cell *
Cons_func(Cell *r, Cell *X, Cell *Y)
{
    mkc2(r, X, Y);
    return r;
}

Cell *
C1_func(Cell *r, Cell *self, Cell *rhs)
{
    return Cons_func(r, getarg(self), rhs);
}

Cell *
C_func(Cell *r, Cell *self, Cell *rhs)
{
    mkfunc(r, &C1_func, rhs);
    return r;
}

//
// a cons C2(X,Y) applied to a func f
// produces (fX)Y
//
// so ```Cxyf -> ``fxy
//
Cell *
apply_C2(Cell *r, Cell *self, Cell *f)
{
    Cell *A;
    Cell *x = getleft(self);
    Cell *y = getright(self);

    A = alloc_cell();
    mkapply(A, f, x);
    mkapply(r, A, y);
    return r;
}

//
// function which reads the next character
// Read x -> cons( getchar(), Read x )
//
Cell *
Read_func(Cell *r, Cell *self, Cell *rhs)
{
    int c;
    Cell *getresult;
    Cell *apply;
    Cell *readf;

#ifdef DBUG_RUNTIME
    putstr("Read_func\r\n");
#endif
    c = getch();
    if (c < 0) c = 256;
#ifdef DEBUG_RUNTIME
    putstr("Read_func got: ");
    puthex(c);
    putstr("\r\n");
#endif
    getresult = alloc_cell();
    apply = alloc_cell();
    readf = alloc_cell();

    // Cons_func allocates memory,
    // so we can't set the types
    // of our newly allocated cells until
    // after we call it
    Cons_func(r, getresult, apply);

    mknum(getresult, c);
    mkfunc(readf, Read_func, NULL);
    mkapply(apply, readf, rhs);
    return r;
}

//
// some special case evaluations
//

// for a number: 
//   0f x -> 1x -> x (so 0f = ki)
//   1f x -> fx
//   2f x -> f(fx)
//   Nf x -> f((N-1)f x)
//
// the way that works is:
//  CT_NUM(n) applied to f -> CT_NUM_PAIR(n,f)
//  CT_NUM_PAIR(n,f) applied to x -> f(f(f(...f(x))))
//  so for example ((3f)x) -> (f(f(fx)))
Cell *
apply_Num(Cell *r, Cell *self, Cell *rhs)
{
    int n = getnum(self);
    if (n == 0) {
        mknum(r, 1); return r;
    }
    if (n == 1) {
        return rhs;
    }
    mknumpair(r, self, rhs);
    return r;
}

Cell *
apply_NumPair(Cell *r, Cell *self, Cell *rhs)
{
  Cell *N = getleft(self);
  Cell *F = getright(self);
  Cell *Asub, *N_1_x, *N_1;
  int n;

  if (gettype(N) != CT_NUM) {
    fatal("NumPair without a NUM");
  }
  n = getnum(N);
  if ( n == 0 ) return rhs;

  // special case the Inc operator
  if (gettype(F) == CT_FUNC && getfunc(F) == Inc_func) {
      // (N+)x -> (N + x)
      if (gettype(rhs) == CT_NUM) {
          int m = getnum(rhs);
          mknum(r, m+n);
          return r;
      }
  }

  //
  // (NF)x -> F ((N-1)Fx )
  //
  if (n == 1) {
    mkapply(r, F, rhs);
    return r;
  }
  Asub =  alloc_cell();
  N_1_x = alloc_cell();
  N_1 = alloc_cell();
  mknum(N_1, n-1);
  mknumpair(N_1_x, N_1, F);
  mkapply(Asub, N_1_x, rhs);
  mkapply(r, F, Asub);
  return r;
}

//
// to apply S2:
// self(x,y) z -> Apply(Apply(x, z), Apply(y, z))
//
Cell *
apply_S2(Cell *r, Cell *self, Cell *z)
{
  Cell *A1, *A2;
    Cell *x, *y;

    A1 = alloc_cell();
    A2 = alloc_cell();

    x = getleft(self);
    y = getright(self);

    mkapply(A1, x, z);
    mkapply(A2, y, z);
    setleft(r, A1);
    setright(r, A2);
    return r;
}

//
// partial function application
// 
Cell *
partial_apply_primitive(Cell *A)
{
    Cell *lhs = getleft(A);
    Cell *rhs = getright(A);
    CellType t = gettype(lhs);
    CellFunc *f = NULL;

#ifdef DEBUG_RUNTIME
    puthex(A); putstr(" "); puthex(lhs); putstr(" "); puthex(rhs);
    putstr(" type="); puthex(t); putstr("\r\n");
#endif
    switch (t) {
    case CT_S2_PAIR:
        f = apply_S2;
        break;
    case CT_C2_PAIR:
        f = apply_C2;
        break;
    case CT_NUM_PAIR:
        f = apply_NumPair;
	break;
    case CT_NUM:
        f = apply_Num;
        break;
    case CT_FUNC:
        f = getfunc(lhs);
        break;
    default:
        fatal("apply_primitive to a non-primitive");
	return lhs;
    }
#ifdef DEBUG_RUNTIME
    putstr("Calling "); puthex(f); putstr("\r\n");
#endif
    return (*f)(A, lhs, rhs);
}

Cell *
partial_eval(Cell *node)
{
    Cell *prev;
    Cell *lhs;
    Cell *cur;

    push_root(node);

    cur = node;
    prev = 0;

    for(;;) {
        while (gettype(cur) == CT_A_PAIR) {
	    push_root(prev);
            prev = cur;
            cur = getleft(cur);
        }
        if (!prev) break;
        // lhs is not an A_PAIR, so apply it to the rhs of prev
        lhs = cur;
        cur = prev; // go back to the apply node
        prev = pop_root();

        assert(lhs == getleft(cur));
        assert( prev == 0 || cur == getleft(prev));
        cur = partial_apply_primitive(cur);
	//make sure it goes in the tree
	if (prev) {
	  setleft(prev, cur);
	}
    }

    pop_root();
    return cur;
}

Cell *car_cdr(Cell *list, CellFunc *fn)
{
    Cell *a_node = alloc_cell();
    Cell *cK = alloc_cell();

    mkfunc(cK, fn, NULL);
    mkapply(a_node, list, cK);
    return a_node;
}

Cell *car(Cell *list)
{
    return car_cdr(list, K_func);
}

Cell *cdr(Cell *list)
{
    return car_cdr(list, KI_func);
}

//
// fully evaluate
// basically we evaluate ((list Inc) 0)
// to get a numeric value
//
int
getintvalue(Cell *X)
{
    Cell *inc;
    Cell *a1, *a2;
    Cell *zero;

    push_root(X);

    // want to evaluate X as a number, so
    //  ((X inc) 0)
    a1 = alloc_cell();
    a2 = alloc_cell();
    inc = alloc_cell();
    zero = alloc_cell();

    mknum(zero, 0);
    mkfunc(inc, Inc_func, NULL);
    mkapply(a2, X, inc);
    mkapply(a1, a2, zero);

    a1 = partial_eval(a1);
    if (gettype(a1) != CT_NUM) {
        fatal("getintval evaluated to a non-integer");
    }
    pop_root();

    return getnum(a1);
}

//
// the main loop
//
int
eval_loop()
{
    int outc;
    Cell *head;

    for(;;) {
        g_root = partial_eval(g_root);
        head = car(g_root);
        outc = getintvalue(head);
        if (outc >= 256) {
            return outc - 256;
        }
        putch(outc);
        g_root = cdr(g_root);
    }
}

#ifdef USE_FDS
FullDuplexSerial ser;
#endif

#ifdef RUNTIME
int
main(int argc, char **argv)
{
# ifdef USE_FDS
    FullDuplexSerial_start(&ser, 31, 30, 0, 115200);
# endif
    return eval_loop(g_root);
}
#endif
#ifdef INTERPRETER
int
main(int argc, char **argv)
{
    FILE *f;

    if (argc != 2) {
        fprintf(stderr, "Usage: lazy file.lazy\n");
        return 2;
    }
    f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    g_root = parse_whole(f);

    return eval_loop(g_root);
}
#endif
