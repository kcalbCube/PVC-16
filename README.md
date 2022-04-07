# PVC-16
PVC-16 virtual machine with x86 inspired abstract architecture.
Allows to make graphical applications.

## Examples
located in pvc-asm/examples

- hello-world.pvc 
hello world!
- brainfuck.pvc 
simple brainfuck impl without input.
- bresenham.pvc
buggy impl of bresenham's algorithm.

## Running a example
1. compile asm file
```
pvc-asm examples/hello-world.pvc -o output.bin
```
2. run
```
pvc-16 output.bin
```
