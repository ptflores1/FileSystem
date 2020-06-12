#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../cr_API/cr_API.h"

int main() {
    int i, BUFFER_SIZE = 8192*2050;
    unsigned char* buffer = (unsigned char*)malloc(BUFFER_SIZE);
    for (i=0; i<BUFFER_SIZE; i++) buffer[i] = 'a';
    cr_mount("_simdiskformat - copia.bin");
    cr_ls(1);
    printf("Press [ENTER] to continue");
    getchar();
    printf("First state\n");
    cr_bitmap(1, false);
    printf("Press [ENTER] to continue");
    getchar();
    /* WRITE */
    char* pathWrite = "really_big_file.txt";
    printf("\nCurrent files:\n");
    crFILE* fileWrite = cr_open(1, pathWrite, 'w');
    cr_write(fileWrite, buffer, BUFFER_SIZE);
    cr_ls(1);
    cr_close(fileWrite);
    printf("Press [ENTER] to continue");
    getchar();

    /* REMOVE */
    printf("Before remove\n");
    cr_bitmap(1, false);
    printf("Press [ENTER] to continue");
    getchar();
    cr_rm(1, pathWrite);
    printf("After remove\n");
    cr_bitmap(1, false);
    printf("Press [ENTER] to continue");
    getchar();
    cr_ls(1);
    return 0;
}
