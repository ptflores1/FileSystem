#include <stdbool.h>
#include <stdio.h>
#include "../cr_API/cr_API.h"
#include <stdlib.h>

char* binPath;

int main() {
    cr_mount("simdiskfilled.bin");
    //cr_ls(1);
    //cr_rm(1, "text.txt");
    //cr_rm(1, "Baroque.mp3");
    // cr_open(1, "Baroque_new.mp3", 'w');
    // cr_softlink(1, 2, "somefiel.some", "some");
    // cr_hardlink(3, "yes.mp3", "no.mp3");


    unsigned char auxBuffer[8192];
    FILE* f = fopen(binPath, "rb");
    fseek(f, 61940*8192, SEEK_SET);
    fread(auxBuffer, 8192, 1, f);
    for(int i = 0; i < 8192; i++){
        printf("%c", auxBuffer[i]);
    }
    printf("\n");
    fclose(f);

    return 0;
}