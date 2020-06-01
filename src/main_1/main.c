
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../cr_API/cr_API.h"


int main() {
    cr_mount("simdiskfilled.bin");
    cr_unload(2, "text.txt", "text.txt");


    // cr_rm(2, "yes.mp3");


    return 0;
}