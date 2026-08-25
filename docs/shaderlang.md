# The RivOS shader language

## What is it?

The RivOS shader language is a custom shader language used by the desktop enviroment and thus native apps

## How does it look like?

The RivOS shader language is a tiny subset from the features of lisp. Features:

- glob varname lit
- set varname lit
- fragment expr
- "+ - * / %" lhs rhs
- print(ln) expr
- defun
- TODO: if, while, maybe functions, >> << > <

## What does a program look like?

A program demonstrating some of the features in the RivOS shader language is:
~~~Lisp
(println "Hello, world!")
(defun fragment (x y) 
    (ret (^ (% x y) (% y x))))
~~~

Let's go over it!

### println "Hello, world!"

Just a simple hello world.

### defun fragment (x y)

This declares a function named fragment, taking input x and y. At the end of any RivOS shader language file should a fragment function be defined which takes in two parameters x and y (Note that names can vary, as long as paramcount == 2).

### ret (^ (% x y) (% y x))

This returns the value calculated by (^ (% x y) (% y x)). Note that the modulo and divison operators are programmed such that with x=a(/or%)b if b=0, x=0. The equivalent C would be:

~~~C
int fragment(int x, int y) {
    return (y != 0 ? (x % y) : 0) ^ (x != 0 ? (y % x) : 0);
}
~~~
Note that ret is required, and unlike in common lisp, the last value will not be seen as the return value.

## Future

### features

This is currently a very bare metal version, later there will be additional support for features like textures, meshes, compute shaders etc..
(As well as work on a real GPU, E.G. a VirtIO device or an AMD card (or simply translate RivOS shader lang -> OpenGL/Vulcan))

### Performance

The current implementation does not perform very well. Later a VM will be used. 
