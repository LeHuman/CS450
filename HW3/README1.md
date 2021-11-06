---
geometry:
-               margin=1in
-               letterpaper
fontsize:       11pt
...

# CS450 PA3 - Part 1 - How to build and execute

The following instructions assume that you are running these commands on a linux system with `valigrind` and `build-essential` installed.
An example binary `memLeakTest` is included.

## Building

To build, run the following command in the source folder.

``` sh
gcc -DTEST=X -g memLeakTest.c -o memLeakTest
```

Where `X` in `-DTEST=X` should be the test number `0-6`

## Executing

To run the binary with Valigrind, run the following command in the source folder.

``` sh
valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all ./memLeakTest
```
