#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

// Change test case
#define TEST_CASE 4

#define __runTestCase(x) testCase##x()
#define _runTestCase(x) __runTestCase(x)
#define runTestCase() _runTestCase(TEST_CASE)

// +0 syscalls
void testCase0(void) {}

// +7 syscalls
void testCase1(void) {
    printf(3, "Hello!\n"); // Each char calls sys_write
}

// +2 syscalls
void testCase2(void) {
    if (fork() == 0) {
        int a = 1 / 0;
        exit();
    }
    wait();
}

// +40 syscalls
void testCase3(void) {
    int p[2], i, fd;
    char *arg = "Hi";
    char *err = (char *)-1;
    char inbuf[16];
    struct stat fs;

    for (i = 0; i < 10; i++) {
        if (fork() == 0) {
            close(1);
            fd = open("backup", O_CREATE | O_RDWR);
            dup(fd);
            printf(1, "Child: Hello!\n");
            close(fd);
            exec("echo", &arg);
        }
        wait();
        kill(0);
        getpid();
    }
}

// +124 syscalls
void testCase4(void) {
    int p[2], i, fd;
    char *arg = "Hi";
    char *err = (char *)-1;
    char inbuf[16];
    struct stat fs;

    for (i = 0; i < 10; i++) {
        if (fork() == 0) {
            close(1);
            fd = open("backup", O_CREATE | O_RDWR);
            dup(fd);
            printf(1, "Child: Hello!\n");
            close(fd);
            exec("echo", &arg);
        }
        wait();
        kill(0);
        getpid();
    }

    for (i = 0; i < 8; i++) {
        sleep(0);
        uptime();
        sbrk(0);
    }

    for (i = 0; i < 5; i++) {
        fd = open("backup", O_CREATE | O_RDWR);
        pipe(p);
        write(fd, arg, 2);
        read(fd, &inbuf, 2);
        fstat(fd, &fs);
        close(fd);
        mknod("", -1, -1);
        unlink(err);
        link(err, err);
        mkdir("test");
        dup(0);
        chdir("");
    }
}

// +3 syscalls because sh calls both sbrk and exec, and test calls countTraps
int main(int argc, char *argv[]) {
    runTestCase();

    countTraps();
    exit();
}
