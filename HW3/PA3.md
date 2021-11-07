---
geometry:
-               margin=0.5in
-               letterpaper
classoption:    table
documentclass:  extarticle
urlcolor:       blue
fontsize:       11pt
header-includes: |
    \rowcolors{2}{gray!10}{gray!5}
...

\normalsize
\pagenumbering{gobble}
\newgeometry{margin=5cm}

\title{%
  Programming Assignment 3 \\
  CS450 Fall, 2021}
\author{David Anderson, and Isaias Rivera}
\date{Nov 8, 2020}

\maketitle

\newgeometry{margin=1in}
\normalsize

# Task 1 - Valgrind Memory Leaks

## Test Cases

\newpage

# Task 2 - System Call GetSharedPage() & FreeSharedPage()

## Man Pages

The following are the manual pages for each system call.

### GetSharedPage()

#### Name

\mbox{}

GetSharedPage - get a region of shared memory pages

#### Synopsis

\mbox{}

``` C
char* GetSharedPage(int key, int pages);
```

#### Description

\mbox{}

GetSharedPage() allows the user to allocate a range of pages in physical memory to be shared between multiple processes.
Takes two arguments, a key ranging from 0-63, which indicates a unique set of shared pages in memory, and a number of pages which ranges from 1-64.
Multiple user processes that call GetSharedPage with matching keys share a range of pages associated with that key.
A new range of shared pages can be created by calling GetSharedPages with a new and unique key and size.
Resizing is not supported but if the page is already allocated and the requested size is smaller than the actual size, the full size of the page is mapped and returned.

### FreeSharedPage()

#### Name

\mbox{}

FreeSharedPage - free a region of shared memory pages

#### Synopsis

\mbox{}

``` C
char* FreeSharedPage(int key);
```

#### Description

\mbox{}

FreeSharedPage() allows the user to deallocate the range of pages in physical memory that the process had been sharing.
FreeSharePage uses the key to identify and deallocate the range of shared pages.

\newpage

## Design

In order to implement the system call `GetSharedPage` and `FreeSharedPage` we had to add additional kernel code. 

\newpage

## Major changes made

+-----------------+-------------------------------------------------------------------------------------------------------------------------+
|    File Name    |                                                         Changes                                                         |
+=================+=========================================================================================================================+
| **`vm.c`**      | Added the functions `mapSharedRegion`; maps a shared region to a given process,                                         |
|                 | `allocSharedVM`; allocates a shared region to the current process,                                                      |
|                 | `deallocProcSharedVM`; deallocates a shared region from a given process,                                                |
|                 | `deallocProcAllSharedVM`; deallocates all the *abandoned* pages that were not freed,                                    |
|                 | `deallocSharedVM`; deallocates a shared region from the current process                                                 |
+-----------------+-------------------------------------------------------------------------------------------------------------------------+
| **`sysproc.c`** | Added the syscall definitions for `GetSharedPage` and `FreeSharedPage`                                                  |
+-----------------+-------------------------------------------------------------------------------------------------------------------------+
| **`proc.c`**    | Added a call to `deallocProcAllSharedVM` when processes are reaped                                                      |
+-----------------+-------------------------------------------------------------------------------------------------------------------------+
| **`proc.h`**    | Added definition for `sharedRegion_t` struct type, which is used to store a shared region in memory                     |
|                 | Added definition for `sharedReference_t` struct type, which is used to reference a shared region in memory by a process |
|                 | Added the `shared` array of `sharedReference_t`s for each proc                                                          |
+-----------------+-------------------------------------------------------------------------------------------------------------------------+

\newpage

## Test Cases

Every test case was run multiple times to ensure that the OS does not panic for any reason.
The test code is placed inside a test program's main function.

### Test Case 0

#### Code

``` C
#include "types.h"
#include "stat.h"
#include "user.h"
int main(int argc, char *argv[]) {
    char *testPage = GetSharedPage(8, 64);
    exit();
}
```

#### Output

```
$ test0 
Shared page not freed! pid:3 id:8 refs:0
```

#### Description

\mbox{}

This test case shows what occurs when a shared page is not freed by a process.
This indicates that the reference counter is working.
This test case also shows that setup for all the other test cases with what `#includes` to have and the `main` function.

### Test Case 1

#### Code

``` C
char *testPage = GetSharedPage(4, 8);
strcpy(testPage, "Hello!");
printf(1, "%s\n", testPage);
FreeSharedPage(4);
exit();
```

#### Output

```
$ test0
Hello!
```

#### Description

\mbox{}

This test case simply shows that we can write and read from a shared page.

\newpage

### Test Case 2

#### Code

``` C
char *testPage = GetSharedPage(4, 8);
strcpy(testPage, "Hello child!");
if (fork() == 0) {
    char *testPage = GetSharedPage(4, 8);
    printf(1, "%s\n", testPage);
    FreeSharedPage(4);
    exit();
}
wait();
FreeSharedPage(4);
exit();
```

#### Output

```
$ test0
Hello child!
```

#### Description

\mbox{}

Because our implementation does not specificlly state that child processes get passed the same shared pages, they must call
the new syscalls themselves. With that said, this test case creates a child proccess that accesses the same shared page as the parent and prints whatever the parent has put there. This shows that communication between a child process and a parent process
is possible.

\newpage

### Test Case 3

#### Code

``` C
char *testPage = GetSharedPage(4, 8);
strcpy(testPage, "Hello child!");
if (fork() == 0) {
    char *testPage = GetSharedPage(4, 8);
    printf(1, "%s\n", testPage);
    FreeSharedPage(4);
    exit();
}
wait();
FreeSharedPage(4);
exit();
```

#### Output

```
$ test0
Hello child!
```

#### Description

\mbox{}

Because our implementation does not specificlly state that child processes get passed the same shared pages, they must call
the new syscalls themselves. With that said, this test case creates a child proccess that accesses the same shared page as the parent and prints whatever the parent has put there. This shows that communication between a child process and a parent process
is possible.

\newpage

### Test Case 4

#### Code

``` C
if (fork() == 0) {
    sleep(100);
    char *tx = GetSharedPage(0, 6);
    char *rx = GetSharedPage(6, 6);
    printf(1, "Child!\n");
}
exit();
```

#### Output

```
$ test0
$ Child!
Shared page not freed! pid:10 id:0 refs:0
Shared page not freed! pid:10 id:6 refs:0
zombie!
```

#### Description

\mbox{}

This test case was used to show what happens when a zombie child attempts to create a shared page and not free it.
It shows how abandoned processes are still handled.

\newpage

### Test Case 5

#### Code
\tiny

``` C
// test0.c
int main(int argc, char *argv[]) {
    char *tx = GetSharedPage(0, 6);
    char *rx = GetSharedPage(6, 6);
    tx = GetSharedPage(0, 6);
    tx = GetSharedPage(0, 6);

    while (tx[0] != 0) {
    }
    strcpy(tx, "0: Hello!");

    while (rx[0] == 0) {
    }
    printf(1, "0 Received: %s\n", rx);
    memset(rx, 0, 4096);

    while (tx[0] != 0) {
    }
    strcpy(tx, "0: says Goodbye!");

    while (rx[0] == 0) {
    }
    printf(1, "0 Received: %s\n", rx);
    memset(rx, 0, 4096);

    printf(1, "0 %p %d\n", tx, FreeSharedPage(0));
    printf(1, "0 %p %d\n", rx, FreeSharedPage(6));

    exit();
}
// test1.c
int main(int argc, char *argv[]) {
    char *rx = GetSharedPage(0, 6);
    char *tx = GetSharedPage(6, 6);
    tx = GetSharedPage(6, 6);
    tx = GetSharedPage(6, 6);
    tx = GetSharedPage(6, 6);

    while (tx[0] != 0) {
    }
    strcpy(tx, "1: Hello!");

    while (rx[0] == 0) {
    }
    sleep(10);
    printf(1, "1 Received: %s\n", rx);
    memset(rx, 0, 4096);

    while (tx[0] != 0) {
    }
    strcpy(tx, "1: says Goodbye!");

    while (rx[0] == 0) {
    }
    sleep(10);
    printf(1, "1 Received: %s\n", rx);
    memset(rx, 0, 4096);

    sleep(20);
    printf(1, "1 %p %d\n", rx, FreeSharedPage(0));
    printf(1, "1 %p %d\n", tx, FreeSharedPage(6));
    exit();
}
```
\normalsize
#### Output

```
test0|test1
0 Received: 1: Hello!
1 Received: 0: Hello!
0 Received: 1: says Goodbye!
0 7FFFA000 1
0 7FFF4000 1
1 Received: 0: says Goodbye!
1 7FFFA000 0
1 7FFF4000 0
```

#### Description

\mbox{}

This test case is an example of communication between two separate processes. The xv6 shell was modified to make the pipe `|` symbol to 
only run programs in parallel to make this test case easy to run. Programs `test0` and `test1` are run concurrently with the following
command `test0|test1`. Test program 1 has a few sleep functions to ensure each process prints on a separate line.