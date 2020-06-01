#include <stdbool.h>
#include <stdio.h>
#include "../cr_API/cr_API.h"
#include <stdlib.h>

char* binPath;

int main() {
    cr_mount("simdiskfilled.bin");
    cr_ls(3);
    crFILE* file = cr_open(2, "caca.txt", 'w');
    unsigned char buffer[8192*3];
    cr_write(file, buffer, sizeof(buffer));
    cr_close(file);

    return 0;
}