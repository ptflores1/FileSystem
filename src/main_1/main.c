
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../cr_API/cr_API.h"

int main() {
    cr_mount("simdiskfilled.bin");
    // cr_unload(2, "text.txt", "text.txt");
    // cr_unload(2, 0, "outdir-2");
    cr_ls(0);
    cr_unload(0, 0, "unload-filled");


    cr_mount("simdiskformat.bin");

    cr_ls(0);
    cr_bitmap(1, 0);

    // cr_rm(1, "great_success.gif");
    cr_load(1, "test-input");

    cr_ls(0);
    cr_bitmap(1, 0);

    cr_unload(0, 0, "load-unload");
    return 0;
}