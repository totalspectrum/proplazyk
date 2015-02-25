# An Implementation of Lazy K for the Propeller

Copyright 2015 Total Spectrum Software, Inc.
Distributed under the terms of the MIT Licence (see COPYING.MIT).
The lazier compiler is copyrighted by Ben Rudiak-Gould and distributed
under the terms of the GPL (see lazier/COPYING).

## Introduction

Imagine a simple computer language -- one without complex data types, or indeed without data types at all. Without variables or side effects. Without even numbers. That language is Lazy K, an implementation of the SKI combinator calculus, and it's now available for the Propeller.

Lazy K  has just one operation, function application (denoted by backquote), and three builtin functions S, K, and I (the last of which is redundant and may be considered syntactic sugar). It is nontheless Turing complete. Provided in the distribution is a translator (called "lazier") from a subset of Scheme to Lazy K.


## Language Basics

Lazy K is a pure functional programming language. The only objects it natively provides are functions. In fact there are just 3 basic functions: s, k, and i (and even i is redundant: it can be expressed in terms of the other two). There are no variables. There are no side effects. It is an incredibly pure language!

The "lazy" part of Lazy K refers to its evaluation order, which is lazy. That is, functions are evaluated only when they are needed. This means that (for example) you can write a function which produces an infinite stream of data, but the interpreter will only evaluate as much of that function as it needs at any time.

Lazy K was designed by Ben Rudiak-Gould.

### Some useful links

[Esoteric Languages Entry](http://esolangs.org/wiki/Lazy_K)
[Original Lazy-K Readme](http://tromp.github.io/cl/lazy-k.html)

[The SKI combinator calculus](http://en.wikipedia.org/wiki/SKI_combinator_calculus)

[Another implementation of Lazy K for Linux](https://github.com/msullivan/LazyK)

### Lazy K Syntax

(I strongly recommend reading the web resources linked above for clarification on the language!)

There are only functions in Lazy K, and all functions take exactly one argument and return another function. The traditional combinator calculus way of writing this is to write the function and its argument in parentheses, e.g. `(A B)` is the result of applying A to B. All whitespace is optional (and ignored). Comments are started with a pound sign `#` and continue until the end of line.

The functions built in to Lazy K are:

 - `I` is the identity function: `(I x) -> x`
 - `K` is the constant function: `((K x) y) -> x`
 - `S` is the function such that `(((S x) y) z) -> ((x z)(y z))`

Note that `I` is redundant, since for any x `(((S K) K) x)` = `((K x) (K x))` = `x`, so `((S K) K)` = `I`.

This version of Lazy K is case insensitive, so `i` and `I` both mean the same thing.

Lazy K also accepts "Unlambda" syntax, where function application is denoted by a backquote (so ``` ``kxy ``` is parsed the same as `((kx)y)`. The astute reader will note that backquote functions just like an open parenthesis, and the close parenthesis is implied because exactly two terms must appear within parentheses.

### Input and Output

Lazy K programs are functions from an input list (each list element is a single character) to an output list. End of input/output is indicated by a list element greater than 255. The lists are constructed in the "usual" encoding; each cons pair ```(a b)``` is encoded as the function ```p``` such that ```(p x)``` equals ```((x a) b)```. The individual elements are thus retrieved by applying this function to ```K``` (to get the first element) or ```(K I)``` (to get the second element).

## The Lazier Compiler

Actually writing programs in Lazy K is challenging (to put it mildly). Roudiak-Gould wrote a translator from a subset of Scheme to Lazy K, called "lazier". It may be found in the lazier/ directory, along with a few notes on usage and some example programs.

## Propeller Version

This propeller version of Lazy K is somewhat different from the standard Lazy K language:

- It does not support the Iota or Jot styles, only the Combinator calculus (CC) and Unlambda style syntax are accepted.
- The propeller version accepts numbers, though! Numbers are enclosed in square brackets. If the number has a $ in front of it, it's treated as hex, otherwise decimal. Numbers act like the corresponding church numerals, e.g. [1] is identical to i, and [2] acts the same as ``` ``s``s`kski ```.
- The propeller version also understands the letter c to mean a cons operator, i.e. a function such that cxyf = fxy. This is the canonical way to represent lists in the SKI calculus.

### How to run on the Propeller

The proplazy compiler builds executable programs. There is no GUI, only a command line version. From the command line, type:
```
proplazy -O hello.lazy
```
to compile the hello.lazy file into a hello.binary. `-O` says to optimize, which causes the compiler to look for some common patterns and replace them with shorter sequences, and is probably a good thing. The resulting hello.binary can be loaded with any Propeller loader. I use the propeller-load tool from PropGCC:
```
propeller-load hello.binary -r -t
```

proplazy also has a `-v` (for verbose) flag, which will dump the parsed tree to output. This is mostly useful for debugging the compiler itself.

### How to run Lazy K programs on the PC

There are also host versions of the interpreter. `lazy hello.lazy` will launch the interpreter with file `hello.lazy`. `lazys` (available if you build from source) is similar to `lazy` but is a special restricted memory version to simulate the constraints of the Propeller.

### How to build from source

There's a Makefile that assumes that you have a native C compiler (gcc) and a version of PropGCC (propeller-elf-gcc) available on your path. You only need PropGCC to rebuild the proplazy compiler; it isn't needed for using proplazy.

You can also cross compile for Windows or Raspberry Pi by doing `make TARGET=win32` or `make TARGET=rpi`.

### Implementation Details

The parser builds a tree of function applications in the obvious way. On the Propeller, each node of the tree takes up 32 bits. 3 bits of this is a tag indicating the type of node, 1 bit is used by the garbage collector, and the other 28 bits is either used for a number or for two 14 bit pointers (since the pointers are always to longs the bottom 2 bits are 0).

## Future Directions

Since there are no side effects in Lazy K, in principle it should be possible use multiple COGs in parallel to evaluate the program. That would be pretty cool.

The garbage collector and cell structure could certainly be used for other functional programming languages (such as Lisp). For that matter it wouldn't be too hard to write a Lisp front end for Lazy K (perhaps building on the lazier framework).

It would be interesting to try change the I/O model of Lazy K for the Propeller. In principle any Propeller program could be considered a function that takes as input time and pin state and produces an output pin state.

Extending Lazier to take advantage of the Propeller specific features (like numbers) might be useful.

The actual runtime system is quite small, but (as presently coded) doesn't fit in COG memory. With some re-arrangement and/or re-coding in assembly we might be able to do that. The garbage collector is about 600 bytes; that could go in a COG of its own. The remaining code is about 3K in C, so with care it might just fit in the 2K of COG RAM. That would speed things up quite a bit.
