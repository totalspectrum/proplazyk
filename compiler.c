#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define SMALL
#include "lazy.h"
#include "runtime_bin.h"

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
    fclose(f);
    return 0;
}
