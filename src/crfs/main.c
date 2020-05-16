#include "../cr_API/cr_API.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

char* binPath;

int main() {
    cr_mount("simdiskfilled.bin");
    printf("%s\n", binPath);
    return 0;
}