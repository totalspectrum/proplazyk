//
// SKN evaluator
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

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
Cell *cInc;

void
init_parse()
{
    cK = alloc_cell();
    cS = alloc_cell();
    cC = alloc_cell();
    cI = alloc_cell();
    cInc = alloc_cell();

    mkfunc(cK, &K_func, NULL);
    mkfunc(cS, &S_func, NULL);
    mkfunc(cC, &C_func, NULL);
    mkfunc(cInc, &Inc_func, NULL);
    mknum(cI, 1);
}

//
// getNextChar: get next significant character,
// skipping all whitespace
//
int
getNextChar(const char **s_ptr)
{
    const char *s = *s_ptr;
    int c;

    for(;;) {
        c = *s++;
        if (!c) return c;
        // ignore whitespace
        if (c == ' ' || c == '\n' || c == '\t') {
            continue;
        }
        // ignore comments
        if (c == '#') {
            do {
                c = *s++;
            } while (c && c != '\n');
            continue;
        }

        // translate from CC style to unlambda style
        if (c == '(') c = '`';
        if (c == ')')
            continue;

        break;
    }
    *s_ptr = s;
    return tolower(c);
}

//
// try to match string "def", skipping non-essential characters like whitespace
// returns 1 if the strings matched, 0 if not
// if a match happened, updates s_ptr to point after the match
// otherwise leaves s_ptr alone
//
static int
match_part(const char **s_ptr, const char *def)
{
    const char *s = *s_ptr;
    int c = 1;

    while (*def != 0) {
        c = getNextChar(&s);
        if (c != *def) return 0;
        def++;
    }
    *s_ptr = s;
    return 1;
}

//
// table of optimizations
// if the input tree matches the string "orig", then actually parse
// the string "replace"
//
struct optimize {
    const char *orig;
    const char *replace;
} opttab[] = {
    { "``s`k``s``s`kski``s``s`ksk```sii``s``s`kski", "$0a" },
    { "```s``s`ksk``s``s`kski``s``s`kski",           "$08" },
    { "```sii```sii``s``s`kski",                     "$100" /* 256 */},
    { "```s`s``s`ksk``sii``s``s`kski",               "$40"  /* 64 */},
    { "``s``s`ksk```sii``s``s`kski",                 "$05" },
    { "```sii``s``s`kski",                           "$04" },
    { "``s``s`ksk``s``s`kski",                       "$03" },
    { "``s``s`kski",                                 "$02" },
};

#define OPTTAB_SIZE (sizeof(opttab)/sizeof(opttab[0]))

//
// parse a subtree
// modifies *s_ptr to point to the next text past what we
// parsed, and returns a pointer to the tree we did parse
// if "opt" is true, try to optimize the string
//

static Cell *
parse_part_opt(const char **s_ptr, bool opt)
{
    int c;
    Cell *r;

    if (opt) {
        int i;

        // check for an optimization
        for (i = 0; i < OPTTAB_SIZE; i++) {
            if (match_part(s_ptr, opttab[i].orig)) {
                const char *newptr = opttab[i].replace;
                return parse_part_opt(&newptr, false);
            }
        }
    }

    do {
        c = getNextChar(s_ptr);
    } while (c == ')');

    if (c <= 0) {
        fprintf(stderr, "Unexpected EOF\n");
        abort();
    }

    if (c == '$') {
        // hex number
        int val = 0;
        const char *s = *s_ptr;
        c = tolower(*s++);
        for (;;) {
            if ((c >= '0' && c <= '9')) {
                val = 16*val + (c-'0');
            } else if (c >= 'a' && (c <= 'f')) {
                val = 16*val + (10+(c-'a'));
            } else {
                break;
            }
            c = tolower(*s++);
        }
        *s_ptr = s - 1;
        r = alloc_cell();
        mknum(r, val);
        return r;
    } else {
        switch (c) {
        case '`':
        {
            Cell *A;
            Cell *x, *y;

            A = alloc_cell();
            x = parse_part(s_ptr);
            push_root(x);
            y = parse_part(s_ptr);
            pop_root();
            mkapply(A, x, y);
            return A;
        }
        case 'k':
            return cK;
            break;
        case 's':
            return cS;
            break;
        case 'i':
            return cI;
            break;
        case 'c':
            return cC;
            break;
        case '+':
            return cInc;
        default:
            fprintf(stderr, "Invalid character %c\n", c);
            abort();
        }
    }
    return NULL;
}

// optimizer setting; off by default
int gl_optimize = false;

Cell *
parse_part(const char **s_ptr)
{
    return parse_part_opt(s_ptr, gl_optimize);
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
        printf("``s");
        PrintTree_1(getleft(cell), 0);
        PrintTree_1(getright(cell), 0);
        break;
    case CT_C2_PAIR:
        printf("``c");
        PrintTree_1(getleft(cell), 0);
        PrintTree_1(getright(cell), 0);
        break;
    case CT_FUNC:
        fn = getfunc(cell);
        if (fn == K_func) {
            printf("k");
        } else if (fn == S_func) {
            printf("s");
        } else if (fn == C_func) {
            printf("c");
        } else if (fn == K1_func) {
            printf("`k"); PrintTree_1(getarg(cell), 0);
        } else if (fn == S1_func) {
            printf("`s"); PrintTree_1(getarg(cell), 0);
        } else if (fn == C1_func) {
            printf("`c"); PrintTree_1(getarg(cell), 0);
	} else if (fn == KI_func) {
	    printf("`ki");
	} else if (fn == Inc_func) {
	  printf("+");
        } else {
	    if (getarg(cell)) printf("`");
            printf("<func:%p>", getfunc(cell));
            PrintTree_1(getarg(cell), 0);
        }
        break;
    case CT_NUM:
        n = getnum(cell);
        if (n == 1) printf("i");
        else printf("$%x", n);
        break;
    default:
        printf("?");
        break;
    }
    if (top) printf("\n");
}

char *
alloc_file(FILE *f)
{
    size_t curlen, maxlen;
    char *base;
    int c;

    curlen = maxlen = 0;
    base = NULL;

    for(;;) {
        if (curlen >= maxlen) {
            maxlen += 1024*1024;
            base = realloc(base, maxlen);
            if (!base) {
                fprintf(stderr, "out of memory\n");
                exit(2);
            }
        }
        c = fgetc(f);
        if (c < 0) {
            base[curlen] = 0;
            break;
        }
        base[curlen++] = c;
    }
    return base;
}

Cell *parse_whole(FILE *f)
{
    init_parse();
    const char *s = alloc_file(f);

    g_root = parse_part(&s);

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
