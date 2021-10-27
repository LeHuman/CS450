#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>

int main() {
    printf("Allocing\n");
    void *ptr = malloc(512);
    printf("Exiting\n");
    sleep(1);
}