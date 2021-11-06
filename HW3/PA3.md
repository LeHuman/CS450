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

### Test Case 1

#### Code

``` C
```

#### Output

```
```

#### Description

\mbox{}
