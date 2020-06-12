#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../cr_API/cr_API.h"

int main() {
    int i, BUFFER_SIZE = 8192;
    cr_mount("_simdiskfilled - copia.bin");
    cr_ls(1);
    printf("Press [ENTER] to continue");
    getchar();
    printf("First state\n");
    cr_bitmap(1, false);
    printf("Press [ENTER] to continue");
    getchar();
    /* READ */
    char* pathRead = "text.txt";
    unsigned char* bufferRead = (unsigned char*)malloc(BUFFER_SIZE);
    printf("The content of %s:\n", pathRead);
    crFILE* fileRead = cr_open(1, pathRead, 'r');
    cr_read(fileRead, bufferRead, BUFFER_SIZE);
    for (i=0; i<BUFFER_SIZE; i++) printf("%c", bufferRead[i]);
    cr_close(fileRead);
    printf("Press [ENTER] to continue");
    getchar();
    /* WRITE */
    char* pathWrite = "text_with_dot.txt";
    printf("\nCurrent files:\n");
    crFILE* fileWrite = cr_open(1, pathWrite, 'w');
    for (i=0; i<BUFFER_SIZE; i++) {
        if (bufferRead[i] == ' ') bufferRead[i] = '.';
    }
    cr_write(fileWrite, bufferRead, BUFFER_SIZE);
    cr_ls(1);
    cr_close(fileWrite);
    printf("Press [ENTER] to continue");
    getchar();

    printf("\nThe content of %s:\n", pathWrite);
    crFILE* fileRead2 = cr_open(1, pathWrite, 'r');
    cr_read(fileRead2, bufferRead, BUFFER_SIZE);
    for (i=0; i<BUFFER_SIZE; i++) printf("%c", bufferRead[i]);
    cr_close(fileRead2);

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

    free(bufferRead);
    return 0;
}
