Here is Ben Rudiak-Gould's "lazier" program, by way of Michael
Sullivan (https://github.com/msullivan/LazyK). lazier can
translate from a subset of Scheme to lazy-k.

The example programs for lazier are in the "eg" folder. I've modified
them to produce unlambda style output by default, except for the
"jot-in-jot" and similar examples.


Note that I had trouble finding a Scheme interpreter that could run
lazier.scm; it turns out that the notation '() is no longer widely
supported. racket does seem to still support it, and is in the Ubuntu
12.04 repositories, so I used that. For example, to convert fib.scm to
fib.lazy I did:

$ cd eg
$ racket
Welcome to Racket v5.1.3
> (load "fib.scm")

Then cut and paste the output to a file "fib.lazy".
