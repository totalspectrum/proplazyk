(load "../lazier.scm")
(load "../prelude.scm")
(load "../prelude-numbers.scm")

(lazy-def '(main input)
 '(
   (cons 72 (cons 101 (cons 108 (cons 108 (cons 111 (cons 44
            (cons 32
            (cons 119 
	    (cons 111 (cons 114 (cons 108 (cons 100 (cons 33
     (cons 10
     (cons 256 256)))))))))))))))
  )
)

(print-as-unlambda (laze 'main))
