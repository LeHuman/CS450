---
geometry:
-               margin=0.5in
-               letterpaper
classoption:    table
documentclass:  extarticle
urlcolor:       blue
fontsize:       11pt
header-includes: |
    \usepackage{multicol}
    \usepackage{graphicx}
...

\normalsize
\pagenumbering{gobble}
\newgeometry{margin=5cm}

\title{%
  Programming Assignment 3 \\
  CS450 Fall, 2021}
\author{David Anderson, and Isaias Rivera \\
A20472540, and A20442116
}
\date{Nov 8, 2020}

\maketitle

\newgeometry{margin=1in}
\normalsize

\pagenumbering{arabic}
\setcounter{page}{1}

# Task 1 - Valgrind Memory Leaks

## Test Cases

### Equivalence Partitioning

\begin{center}
    \includegraphics[height=0.3\textwidth]{./img/eqp.png}
\end{center}

We used a simple approach to partitioning Valgrind tests by demonstrating leaks after using several of the main C standard library functions for dynamic memory allocation `malloc`, `calloc`, `realloc`. The allocator expands or resizes the heap according to these requests. `malloc` allocates the specified number of bytes, `realloc` increases or decreases the size of the specified block of memory, and `calloc` allocates the number of bytes like `malloc`, but initializes them to zero. `calloc` also takes two arguments, the number of variables to allocate in memory, and the size in bytes of a single variable.  
In general, we found it useful to demonstrate a variety of `malloc` implementations and their different effects on the memory leakage checks by Valgrind. (Tests 0 - 6) In LEAK SUMMARY of Test 2, for instance, can see how the `struct` element pointers were lost directly and that the memory `malloc`'d was lost indirectly. We see similar behavior for Test 3 without a `struct`. For Test 4, interestingly we see that the memory is in the 'still reachable' category because of its global pointer declaration. Tests 5 and 6 demonstrate expected behavior when calling `malloc` via a function call. Tests 7 and 8 are used to demonstrate that memory is leaked the same way it is when allocated by `malloc` without being freed. In Test 7 memory is allocated and zero'd, then an array is instantiated. We see that the length of the array is the amount of memory leaked as expected. Test 8 an array is instantiated and reallocated, with the expected amount of memory leaked. The variation in Valgrind `memcheck` results demonstrated by the variations of `malloc` implementations would be similar in the different usages of `calloc` and `realloc`.

\newpage
### Test Case 0

#### Code

\scriptsize

``` C
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>

typedef struct test_t {
    int a;
    char *str;
    char *str2;
} test_t;

static void *sPtr;

void test5() {
    void *ptr = malloc(1024);
}

void test6() {
    sPtr = malloc(4096);
}

int main() {
    printf("Running Test\n");
    void *ptr = malloc(512);
    printf("Exiting\n");
    sleep(1);
}
```

\normalsize

#### Output

\begin{center}
    \includegraphics[height=0.3\textwidth]{./img/test0.png}
\end{center}

#### Note

\mbox{}

Every other test case simply replaces what is inside the main for this test case.

\newpage

### Test Case 1

#### Code

``` C
for (size_t i = 0; i < 10; i++) {
    void *ptr = malloc(4096);
}
```

#### Output

\begin{center}
    \includegraphics[height=0.3\textwidth]{./img/test1.png}
\end{center}

### Test Case 2

#### Code

``` C
test_t *test = malloc(sizeof(test_t));
test->str = malloc(1024);
test->str2 = "StaticString!";
test->a = 200;
```

#### Output

\begin{center}
    \includegraphics[height=0.3\textwidth]{./img/test2.png}
\end{center}

\newpage

### Test Case 3

#### Code

``` C
void *ptr = malloc(sizeof(void *));
*((void **)ptr) = malloc(4096);
```

#### Output

\begin{center}
    \includegraphics[height=0.3\textwidth]{./img/test3.png}
\end{center}

### Test Case 4

#### Code

``` C
sPtr = malloc(4096);
```

#### Output

\begin{center}
    \includegraphics[height=0.3\textwidth]{./img/test4.png}
\end{center}

\newpage

### Test Case 5

#### Code

``` C
test5();
```

#### Output

\begin{center}
    \includegraphics[height=0.3\textwidth]{./img/test5.png}
\end{center}

### Test Case 6

#### Code

``` C
test6();
```

#### Output

\begin{center}
    \includegraphics[height=0.3\textwidth]{./img/test6.png}
\end{center}

\newpage

### Test Case 7

#### Code

``` C
int n, i;
n = 10;
int *ptr = (int *)calloc(n, sizeof(int));
for (i = 0; i < n; i++) {
    ptr[i] = i + 1;
}
```

#### Output

\begin{center}
    \includegraphics[height=0.3\textwidth]{./img/test7.png}
\end{center}

### Test Case 8

#### Code

\scriptsize

``` C
int n, i;
n = 10;
int *ptr = (int *)calloc(n, sizeof(int));
for (i = 0; i < n; i++) {
    ptr[i] = i + 1;
}
n = 20;
ptr = realloc(ptr, n * sizeof(int));
for (i = 0; i < n; i++) {
    ptr[i] = i + 1;
}
```

\normalsize

#### Output

\begin{center}
    \includegraphics[height=0.3\textwidth]{./img/test8.png}
\end{center}

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

Our idea for implementing `GetSharedPage` and `FreeSharedPage` was to add onto the `struct proc` in `proc.h` where each process would reference
a set size of shared memory. This was done through various additional structs that keep track of the references and the actual pointers to memory,
both virtual, relative to the process, and physical. This means that it does not matter which process first initializes the shared pages as each process references them the same.

Each region of shared memory is represented by a `struct sharedRegion_t` which keeps track of whether this region is valid, the number of references, the number of pages that this region contains,
and the array or pages themselves. These structs are independent of any program. Each program can reference multiple `sharedRegion_t`s with the `struct sharedReference_t`, this allows us to also
check if a process has forgotten to free any shared references, as if the pointer is valid then the reference exists.

Each `proc` also keeps track of the shared memory region within it's own memory space, as the shared memory space *grows* from the top at `KERNBASE` down to `0x0`. We also modified xv6 to ensure that
, when allocating more memory, we do not go over any region of shared memory instead of just `KERNBASE`. This is done with the added `void *shaddr` pointer in `struct proc`.

There were, however, some limitations to keep our implementation simple. We set a maximum limit to the number of share regions and pages per region, this allows keys to point to the
actual index of each region when referenced by an array. We also are unable to resize the pages afterwards as that would involve having to remap the regions for each process and ensuring that nothing
overlaps. Finally, the allocation of shared memory is very basic, in the sense that the shared memory region only grows and does not shrink after freeing shared memory.

## Major changes made
\rowcolors{2}{gray!10}{gray!5}
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
|                 | Initalize the `shaddr` pointer to `KERNBASE`                                                                            |
+-----------------+-------------------------------------------------------------------------------------------------------------------------+
| **`proc.h`**    | Added definition for `sharedRegion_t` struct type, which is used to store a shared region in memory                     |
|                 | Added definition for `sharedReference_t` struct type, which is used to reference a shared region in memory by a process |
|                 | Added the `shared` array of `sharedReference_t`s for each proc                                                          |
+-----------------+-------------------------------------------------------------------------------------------------------------------------+
| **`defs.h`**    | Added definition for `deallocProcAllSharedVM` to be used by `proc.c`                                                    |
+-----------------+-------------------------------------------------------------------------------------------------------------------------+
| **`sh.c`**      | Modified pipe symbol to only run programs in parallel, this is purely done for testing                                  |
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

Because our implementation does not specifically state that child processes get passed the same shared pages, they must call
the new syscalls themselves. With that said, this test case creates a child process that accesses the same shared page as the parent and prints whatever the parent has put there. This shows that communication between a child process and a parent process
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

\mbox{}

\scriptsize

\begin{multicols}{2}
\begin{minipage}{0.5\textwidth}

\begin{Shaded}
\begin{Highlighting}[]
\CommentTok{// test1.c}
\DataTypeTok{int}\NormalTok{ main}\OperatorTok{(}\DataTypeTok{int}\NormalTok{ argc}\OperatorTok{,} \DataTypeTok{char} \OperatorTok{*}\NormalTok{argv}\OperatorTok{[])} \OperatorTok{\{}
    \DataTypeTok{char} \OperatorTok{*}\NormalTok{rx }\OperatorTok{=}\NormalTok{ GetSharedPage}\OperatorTok{(}\DecValTok{0}\OperatorTok{,} \DecValTok{6}\OperatorTok{);}
    \DataTypeTok{char} \OperatorTok{*}\NormalTok{tx }\OperatorTok{=}\NormalTok{ GetSharedPage}\OperatorTok{(}\DecValTok{6}\OperatorTok{,} \DecValTok{6}\OperatorTok{);}
\NormalTok{    tx }\OperatorTok{=}\NormalTok{ GetSharedPage}\OperatorTok{(}\DecValTok{6}\OperatorTok{,} \DecValTok{6}\OperatorTok{);}
\NormalTok{    tx }\OperatorTok{=}\NormalTok{ GetSharedPage}\OperatorTok{(}\DecValTok{6}\OperatorTok{,} \DecValTok{6}\OperatorTok{);}
\NormalTok{    tx }\OperatorTok{=}\NormalTok{ GetSharedPage}\OperatorTok{(}\DecValTok{6}\OperatorTok{,} \DecValTok{6}\OperatorTok{);}

    \ControlFlowTok{while} \OperatorTok{(}\NormalTok{tx}\OperatorTok{[}\DecValTok{0}\OperatorTok{]} \OperatorTok{!=} \DecValTok{0}\OperatorTok{)} \OperatorTok{\{}
    \OperatorTok{\}}
\NormalTok{    strcpy}\OperatorTok{(}\NormalTok{tx}\OperatorTok{,} \StringTok{"1: Hello!"}\OperatorTok{);}

    \ControlFlowTok{while} \OperatorTok{(}\NormalTok{rx}\OperatorTok{[}\DecValTok{0}\OperatorTok{]} \OperatorTok{==} \DecValTok{0}\OperatorTok{)} \OperatorTok{\{}
    \OperatorTok{\}}
\NormalTok{    sleep}\OperatorTok{(}\DecValTok{10}\OperatorTok{);}
\NormalTok{    printf}\OperatorTok{(}\DecValTok{1}\OperatorTok{,} \StringTok{"1 Received: \%s}\SpecialCharTok{\textbackslash{}n}\StringTok{"}\OperatorTok{,}\NormalTok{ rx}\OperatorTok{);}
\NormalTok{    memset}\OperatorTok{(}\NormalTok{rx}\OperatorTok{,} \DecValTok{0}\OperatorTok{,} \DecValTok{4096}\OperatorTok{);}

    \ControlFlowTok{while} \OperatorTok{(}\NormalTok{tx}\OperatorTok{[}\DecValTok{0}\OperatorTok{]} \OperatorTok{!=} \DecValTok{0}\OperatorTok{)} \OperatorTok{\{}
    \OperatorTok{\}}
\NormalTok{    strcpy}\OperatorTok{(}\NormalTok{tx}\OperatorTok{,} \StringTok{"1: says Goodbye!"}\OperatorTok{);}

    \ControlFlowTok{while} \OperatorTok{(}\NormalTok{rx}\OperatorTok{[}\DecValTok{0}\OperatorTok{]} \OperatorTok{==} \DecValTok{0}\OperatorTok{)} \OperatorTok{\{}
    \OperatorTok{\}}
\NormalTok{    sleep}\OperatorTok{(}\DecValTok{10}\OperatorTok{);}
\NormalTok{    printf}\OperatorTok{(}\DecValTok{1}\OperatorTok{,} \StringTok{"1 Received: \%s}\SpecialCharTok{\textbackslash{}n}\StringTok{"}\OperatorTok{,}\NormalTok{ rx}\OperatorTok{);}
\NormalTok{    memset}\OperatorTok{(}\NormalTok{rx}\OperatorTok{,} \DecValTok{0}\OperatorTok{,} \DecValTok{4096}\OperatorTok{);}

\NormalTok{    sleep}\OperatorTok{(}\DecValTok{20}\OperatorTok{);}
\NormalTok{    printf}\OperatorTok{(}\DecValTok{1}\OperatorTok{,} \StringTok{"1 \%p \%d}\SpecialCharTok{\textbackslash{}n}\StringTok{"}\OperatorTok{,}\NormalTok{ rx}\OperatorTok{,}\NormalTok{ FreeSharedPage}\OperatorTok{(}\DecValTok{0}\OperatorTok{));}
\NormalTok{    printf}\OperatorTok{(}\DecValTok{1}\OperatorTok{,} \StringTok{"1 \%p \%d}\SpecialCharTok{\textbackslash{}n}\StringTok{"}\OperatorTok{,}\NormalTok{ tx}\OperatorTok{,}\NormalTok{ FreeSharedPage}\OperatorTok{(}\DecValTok{6}\OperatorTok{));}
\NormalTok{    exit}\OperatorTok{();}
\OperatorTok{\}}
\end{Highlighting}
\end{Shaded}

\end{minipage}

\begin{Shaded}
\begin{Highlighting}[]
\CommentTok{// test0.c}
\DataTypeTok{int}\NormalTok{ main}\OperatorTok{(}\DataTypeTok{int}\NormalTok{ argc}\OperatorTok{,} \DataTypeTok{char} \OperatorTok{*}\NormalTok{argv}\OperatorTok{[])} \OperatorTok{\{}
    \DataTypeTok{char} \OperatorTok{*}\NormalTok{tx }\OperatorTok{=}\NormalTok{ GetSharedPage}\OperatorTok{(}\DecValTok{0}\OperatorTok{,} \DecValTok{6}\OperatorTok{);}
    \DataTypeTok{char} \OperatorTok{*}\NormalTok{rx }\OperatorTok{=}\NormalTok{ GetSharedPage}\OperatorTok{(}\DecValTok{6}\OperatorTok{,} \DecValTok{6}\OperatorTok{);}
\NormalTok{    tx }\OperatorTok{=}\NormalTok{ GetSharedPage}\OperatorTok{(}\DecValTok{0}\OperatorTok{,} \DecValTok{6}\OperatorTok{);}
\NormalTok{    tx }\OperatorTok{=}\NormalTok{ GetSharedPage}\OperatorTok{(}\DecValTok{0}\OperatorTok{,} \DecValTok{6}\OperatorTok{);}

    \ControlFlowTok{while} \OperatorTok{(}\NormalTok{tx}\OperatorTok{[}\DecValTok{0}\OperatorTok{]} \OperatorTok{!=} \DecValTok{0}\OperatorTok{)} \OperatorTok{\{}
    \OperatorTok{\}}
\NormalTok{    strcpy}\OperatorTok{(}\NormalTok{tx}\OperatorTok{,} \StringTok{"0: Hello!"}\OperatorTok{);}

    \ControlFlowTok{while} \OperatorTok{(}\NormalTok{rx}\OperatorTok{[}\DecValTok{0}\OperatorTok{]} \OperatorTok{==} \DecValTok{0}\OperatorTok{)} \OperatorTok{\{}
    \OperatorTok{\}}
\NormalTok{    printf}\OperatorTok{(}\DecValTok{1}\OperatorTok{,} \StringTok{"0 Received: \%s}\SpecialCharTok{\textbackslash{}n}\StringTok{"}\OperatorTok{,}\NormalTok{ rx}\OperatorTok{);}
\NormalTok{    memset}\OperatorTok{(}\NormalTok{rx}\OperatorTok{,} \DecValTok{0}\OperatorTok{,} \DecValTok{4096}\OperatorTok{);}

    \ControlFlowTok{while} \OperatorTok{(}\NormalTok{tx}\OperatorTok{[}\DecValTok{0}\OperatorTok{]} \OperatorTok{!=} \DecValTok{0}\OperatorTok{)} \OperatorTok{\{}
    \OperatorTok{\}}
\NormalTok{    strcpy}\OperatorTok{(}\NormalTok{tx}\OperatorTok{,} \StringTok{"0: says Goodbye!"}\OperatorTok{);}

    \ControlFlowTok{while} \OperatorTok{(}\NormalTok{rx}\OperatorTok{[}\DecValTok{0}\OperatorTok{]} \OperatorTok{==} \DecValTok{0}\OperatorTok{)} \OperatorTok{\{}
    \OperatorTok{\}}
\NormalTok{    printf}\OperatorTok{(}\DecValTok{1}\OperatorTok{,} \StringTok{"0 Received: \%s}\SpecialCharTok{\textbackslash{}n}\StringTok{"}\OperatorTok{,}\NormalTok{ rx}\OperatorTok{);}
\NormalTok{    memset}\OperatorTok{(}\NormalTok{rx}\OperatorTok{,} \DecValTok{0}\OperatorTok{,} \DecValTok{4096}\OperatorTok{);}

\NormalTok{    printf}\OperatorTok{(}\DecValTok{1}\OperatorTok{,} \StringTok{"0 \%p \%d}\SpecialCharTok{\textbackslash{}n}\StringTok{"}\OperatorTok{,}\NormalTok{ tx}\OperatorTok{,}\NormalTok{ FreeSharedPage}\OperatorTok{(}\DecValTok{0}\OperatorTok{));}
\NormalTok{    printf}\OperatorTok{(}\DecValTok{1}\OperatorTok{,} \StringTok{"0 \%p \%d}\SpecialCharTok{\textbackslash{}n}\StringTok{"}\OperatorTok{,}\NormalTok{ rx}\OperatorTok{,}\NormalTok{ FreeSharedPage}\OperatorTok{(}\DecValTok{6}\OperatorTok{));}

\NormalTok{    exit}\OperatorTok{();}
\OperatorTok{\}}
\end{Highlighting}
\end{Shaded}

\end{multicols}

\normalsize

#### Output

```
$ test0|test1
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
Unlike test case 2, these are two separate processes communicating.