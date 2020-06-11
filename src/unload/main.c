#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../cr_API/cr_API.h"

int main() {
    cr_mount("simdiskfilled.bin");
    // cr_unload(2, "text.txt", "text.txt");
    // cr_unload(2, 0, "outdir-2");
    cr_ls(0);
    cr_unload(0, 0, "unload-output");
    return 0;
}
