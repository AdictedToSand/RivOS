# The RivOS shader language

## What is it?

The RivOS shader language is a custom shader language required by the desktop enviroment

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

A program demonstrating all the featurs in the RivOS shader language is:

NOTE: This example will soon be outdated in favor of a perPixel function
~~~Lisp
(fragment (+ (% x y) (% y x))) 

(print (+ x y))
~~~

Let's go over everything in this program:

### Fragment

This is a fragment shader, which you might recognize (if you once did shaders) runs for every pixel on the screen.

The left side of this will be the return value.

### + and %

These are the common plus and modulo operations you can find in other languages like C. Division and modulo will return a 0 if the rhs is 0.

### x and y

If you have a keen eye, you might notice x and y are undefined. However, in the global scope of a RivOS shader will always be x and y. Before any fragment shader, these are 0. Afterwards they will be whatever pixel your fragment shader was last at.

### Print 

This will output the left hand side value to the screen.

## Future

### features

This is currently a very bare metal version, later there will be additional support for features like textures, meshes, compute shaders etc..

### Performance

The current implementation does not perform very well. Later a VM will be used. 
