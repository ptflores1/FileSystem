#include <stdbool.h>
#include <stdio.h>
#include "../cr_API/cr_API.h"
#include <stdlib.h>

char* binPath;

int main() {
    cr_mount("simdiskfilled.bin");
    crFILE* file = cr_open(1, "text.txt", 'r');

    unsigned char buffer[8192*3];
    int bytesRead = cr_read(file, &buffer, sizeof(buffer));


    return 0;
}