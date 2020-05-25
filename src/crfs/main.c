#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../cr_API/cr_API.h"

char* binPath;

int main() {
    cr_mount("simdiskfilled.bin");
    printf("%s\n", binPath);
    // cr_bitmap(1, true);
    // cr_bitmap(1, false);
    cr_bitmap(0, true);
    return 0;
}