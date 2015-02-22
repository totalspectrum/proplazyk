#
# targets to build
#

lazy: lazy.c parser.c lazy.h
	$(CC) -g -o lazy lazy.c parser.c
