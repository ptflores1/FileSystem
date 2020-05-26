#include <stdbool.h>
#include <stdio.h>
#include "../cr_API/cr_API.h"
#include <stdlib.h>

char* binPath;

int main() {
    cr_mount("simdiskfilled.bin");
    // cr_softlink(1, 2, "somefiel.some", "some");
    cr_ls(1);
    return 0;
}