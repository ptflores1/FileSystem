#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../cr_API/cr_API.h"

int main() {
    cr_mount("simdiskfilled.bin");
    // cr_unload(2, "text.txt", "text.txt");
    // cr_unload(2, 0, "outdir-2");
    printf("Partition 1:\n");
    cr_ls(1);
    printf("Partition 2:\n");
    cr_ls(2);
    printf("Partition 3:\n");
    cr_ls(3);
    printf("Partition 4:\n");
    cr_ls(4);
    cr_unload(0, 0, "unload-output");
    return 0;
}
