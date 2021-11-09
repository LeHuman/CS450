
/**
 * Isaias Rivera - A20442116
 * David Anderson - A20472540
 * 2021-11-08
 */

#include "types.h"
#include "stat.h"
#include "user.h"

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
