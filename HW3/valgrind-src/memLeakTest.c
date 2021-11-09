#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>

#ifndef TEST
#define TEST 0
#endif

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
#if TEST == 0 // Simple Malloc to show that valgrind is working
    void *ptr = malloc(512);
#elif TEST == 1 // Show that a loop is also detected
    for (size_t i = 0; i < 10; i++) {
        void *ptr = malloc(4096);
    }
#elif TEST == 2 // Show that not freeing the struct elements that are malloced is also detected
    test_t *test = malloc(sizeof(test_t));
    test->str = malloc(1024);
    test->str2 = "StaticString!";
    test->a = 200;
#elif TEST == 3 // More general example of test 2
    void *ptr = malloc(sizeof(void *));
    *((void **)ptr) = malloc(4096);
#elif TEST == 4 // Show that static pointers are also lost
    sPtr = malloc(4096);
#elif TEST == 5 // Show that malloc in function also shows up
    test5();
#elif TEST == 6 // Show that malloc in a function to a static pointer also causes diffrent behavior
    test6();
#elif TEST == 7 // show that calloc also leaks similarly
    int n, i;
    n = 10;
    int *ptr = (int *)calloc(n, sizeof(int));
    for (i = 0; i < n; i++) {
        ptr[i] = i + 1;
    }
    return 0;
#elif TEST == 8 // show that realloc also leaks
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
    return 0;
#endif
    printf("Exiting\n");
    sleep(1);
}