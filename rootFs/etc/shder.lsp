 (fragment
  (^ (% x y) (% y x)))
; The shader above will eventually have a "unknown function???" 
; Nevermind, it just goes wrong after a while.
(println "Hello, world!")


(defun perPixel (x y)
    (print "Hi")
    (print "Hello")
    (ret (^ (% x y) (% y x)))
)
;(println (perPixel 5 2))

;(perPixelHandlr set perPixel)
; OR (perPixelHandlr perPixel)

; (var col (rgb 0 255 (rgb 0 0 255)))
; (println col)
; (print "Typename: ")
; (println (typename col))

; ? (loop (fragment)) == (fragmentLoop)
; Do need to implement loop tho

; (fragment)
