# An Implementation of Lazy K for the Propeller

## Introduction

Lazy K is a pure functional programming language. The only objects it natively provides are functions. In fact there are just 3 basic functions: s, k, and i (and even i is redundant: it can be expressed in terms of the other two). There are no variables. There are no side effects. It is an incredibly pure language!

Lazy K was designed by Ben Rudiak-Gould.

Some useful links:

Web pages about Lazy K:
http://esolangs.org/wiki/Lazy_K
http://tromp.github.io/cl/lazy-k.html

The SKI combinator calculus:
http://en.wikipedia.org/wiki/SKI_combinator_calculus

Another implementation of Lazy K for Linux:
https://github.com/msullivan/LazyK

## Propeller Version

The propeller version is somewhat different from the standard Lazy K language:

- It does not support the Iota or Jot styles, only the Combinator calculus (CC) and Unlambda style syntax are accepted.
- The propeller version accepts numbers, though! Numbers are enclosed in square brackets. If the number has a $ in front of it, it's treated as hex, otherwise decimal. Numbers act like the corresponding church numerals, e.g. [1] is identical to i, and [2] acts the same as ``s``s`kski.
- The propeller version also understands the letter c to mean a cons operator, i.e. a function such that cxyf = fxy. This is the canonical way to represent lists in the SKI calculus.

### How to run

The proplazy compiler builds executable programs.

### How to build from source
