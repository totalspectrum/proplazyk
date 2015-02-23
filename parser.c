//
// SKN evaluator
//

#include <stdio.h>
#include <stdlib.h>

//#define SMALL

#include "lazy.h"


static void PrintTree_1(Cell *t, int val);
void PrintTree(Cell *t) { PrintTree_1(t, 1); }

//
// parsing function
//
// this parses a file into a cell
//

Cell *cK;
Cell *cS;
Cell *cI;
Cell *cC;

void
init_parse()
{
    cK = alloc_cell();
    cS = alloc_cell();
    cC = alloc_cell();
    cI = alloc_cell();
    mkfunc(cK, &K_func, NULL);
    mkfunc(cS, &S_func, NULL);
    mkfunc(cC, &C_func, NULL);
    mknum(cI, 1);
}

Cell *
parse_part(FILE *f)
{
    int c;
    Cell *r;

    // skip comments and newlines
    c = fgetc(f);
    for(;;) {
        if (c == ' ' || c == '\n' || c == '\t') {
            c = fgetc(f);
            continue;
        }
        if (c == '#') {
            do {
                c = fgetc(f);
            } while (c != '\n' && c > 0);
            continue;
        }
        break;
    }

    if (c < 0) {
        fprintf(stderr, "Unexpected EOF\n");
        abort();
    }
    if (c >= '0' && c <= '9') {
        int val = 0;
        while (c >= '0' && c <= '9') {
            val = 10*val + (c-'0');
            c = fgetc(f);
        }
        ungetc(c, f);
        r = alloc_cell();
        mknum(r, val);
        return r;
    } else {
        switch (c) {
        case '`':
        {
            Cell *A = alloc_cell();
            Cell *x, *y;

            x = parse_part(f);
            push_root(x);
            y = parse_part(f);
            pop_root();
            mkapply(A, x, y);
            return A;
        }
        case 'k': case 'K':
            return cK;
            break;
        case 's': case 'S':
            return cS;
            break;
        case 'i': case 'I':
            return cI;
            break;
        case 'c': case 'C':
            return cC;
            break;
        default:
            fprintf(stderr, "Invalid character %c\n", c);
            abort();
        }
    }
    return NULL;
}

static void
PrintTree_1(Cell *cell, int top)
{
    CellFunc *fn;
    int n;
    if (!cell) {
      printf("?");
      return;
    }
    switch(gettype(cell)) {
    case CT_A_PAIR:
    case CT_NUM_PAIR:
        printf("`");
        PrintTree_1(getleft(cell), 0);
        PrintTree_1(getright(cell), 0);
        break;
    case CT_S2_PAIR:
        printf("``S");
        PrintTree_1(getleft(cell), 0);
        PrintTree_1(getright(cell), 0);
        break;
    case CT_C2_PAIR:
        printf("``C");
        PrintTree_1(getleft(cell), 0);
        PrintTree_1(getright(cell), 0);
        break;
    case CT_FUNC:
        fn = getfunc(cell);
        if (fn == K_func) {
            printf("K");
        } else if (fn == S_func) {
            printf("S");
        } else if (fn == C_func) {
            printf("C");
        } else if (fn == K1_func) {
            printf("`K"); PrintTree_1(getarg(cell), 0);
        } else if (fn == S1_func) {
            printf("`S"); PrintTree_1(getarg(cell), 0);
        } else if (fn == C1_func) {
            printf("`C"); PrintTree_1(getarg(cell), 0);
	} else if (fn == KI_func) {
	    printf("`KI");
	} else if (fn == Inc_func) {
	  printf("+");
	} else if (fn == Read_func) {
	  printf("R");
        } else {
	    if (getarg(cell)) printf("`");
            printf("<func:%p>", getfunc(cell));
            PrintTree_1(getarg(cell), 0);
        }
        break;
    case CT_NUM:
        n = getnum(cell);
        if (n == 1) printf("i");
        else printf("%d ", n);
        break;
    default:
        printf("?");
        break;
    }
    if (top) printf("\n");
}

Cell *parse_whole(FILE *f)
{
    init_parse();
    g_root = parse_part(f);

    // append a lazy read
    {
        Cell *readf;
        Cell *zero;
        Cell *a1, *a2;

        zero = alloc_cell();
        readf = alloc_cell();
        a1 = alloc_cell();
        a2 = alloc_cell();

        mknum(zero, 0);
        mkfunc(readf, Read_func, 0);
        mkapply(a2, readf, zero);
        mkapply(a1, g_root, a2);
        g_root = a1;
    }
    return g_root;
}
