# An Implementation of Lazy K for the Propeller

Copyright 2015 Total Spectrum Software, Inc.
Distributed under the terms of the MIT Licence (see COPYING.MIT).
The lazier compiler is copyrighted by Ben Rudiak-Gould and distributed
under the terms of the GPL (see lazier/COPYING).

## Introduction

Lazy K is a pure functional programming language. The only objects it natively provides are functions. In fact there are just 3 basic functions: s, k, and i (and even i is redundant: it can be expressed in terms of the other two). There are no variables. There are no side effects. It is an incredibly pure language!

Lazy K was designed by Ben Rudiak-Gould.

Some useful links:

[Esoteric Languages Entry](http://esolangs.org/wiki/Lazy_K)
[Original Lazy-K Readme](http://tromp.github.io/cl/lazy-k.html)

[The SKI combinator calculus](http://en.wikipedia.org/wiki/SKI_combinator_calculus)

[Another implementation of Lazy K for Linux](https://github.com/msullivan/LazyK)

## Language Basics

There are only functions in Lazy K, and all functions take exactly one argument and return another function. The traditional combinator calculus way of writing this is to write the function and its argument in parentheses, e.g. `(A B)` is the result of applying A to B. All whitespace is optional (and ignored). Comments are started with a pound sign `\#` and continue until the end of line.

The functions built in to Lazy K are:
`I` is the identity function: `(I x) -> x`
`K` is the constant function: `((K x) y)` -> x`
`S` is the function such that `(((S x) y) z) -> ((x z)(y z))`

Note that `I` is redundant, since for any x `(((S K) K) x)` = `((K x) (K x))` = `x`, so `((S K) K)` = `I`.

This version of Lazy K is case insensitive, so `i` and `I` both mean the same thing.

Lazy K also accepts "Unlambda" syntax, where function application is denoted by a backquote (so `\`\`kxy` is parsed the same as `((kx)y)`.

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
