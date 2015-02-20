#
# targets to build
#

lazy: lazy.c lazy.h
	$(CC) -g -o lazy lazy.c
