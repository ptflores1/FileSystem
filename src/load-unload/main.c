
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../cr_API/cr_API.h"

int main() {
    cr_mount("simdiskformat.bin");

    printf("Files in partiton 1:\n");
    cr_ls(1);

    cr_load(1, "test-input");

    printf("Files in partiton 1 after loading:\n");
    cr_ls(1);

    cr_unload(0, 0, "load-unload-output");
    return 0;
}
