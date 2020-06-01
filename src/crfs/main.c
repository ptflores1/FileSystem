#include <stdbool.h>
#include <stdio.h>
#include "../cr_API/cr_API.h"
#include <stdlib.h>

char* binPath;

int main() {
    cr_mount("simdiskfilled.bin");
    cr_ls(2);
    // cr_rm(2, "yes.mp3");


    return 0;
}