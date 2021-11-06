---
geometry:
-               margin=1in
-               letterpaper
fontsize:       11pt
...

# CS450 PA3 - Part 2 - How to build and execute

The following instructions assume that you are running these commands on a linux system with `qemu`, `qemu-kvm`, and `build-essential` installed.
The *executables* `xv6.img` and `fs.img` are included in the source folder.

## Building

To build, run the following command in the source folder.

``` sh
make
```

## Executing

To run the compiled images, run the following command in the source folder.

``` sh
make qemu-nox
```

The shell prompt for xv6 should appear shortly after in the same terminal.
After that, run the test commands as shown in the test cases.

i.e.

`` sh
test0 | test1
``