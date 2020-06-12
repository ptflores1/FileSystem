
#include <stdbool.h>
#include <stdio.h>
#include "../cr_API/cr_API.h"
#include <stdlib.h>

char* binPath;

int main() {
    cr_mount("simdiskfilled.bin");
    cr_ls(2);

    // cr_hardlink(2, "Program in C.mkv", "Program in Python.py");
    cr_rm(2, "Program in Python.py");

    FILE *f = fopen("simdiskfilled.bin", "r");
    unsigned char *buffer = (unsigned char *)malloc(8192);

    fseek(f, 512 * 1024 * 1024, SEEK_SET);
    fread(buffer, 1, 8192, f);
    for (int i = 0; i < 8192; i += 32)
    {
        if(buffer[i] & 0x80){
            printf("%02x %02x %02x ", buffer[i], buffer[i + 1], buffer[i + 2]);
            printf("%s\n", &buffer[i + 3]);
        }
    }
    

    fclose(f);
    free(buffer);
    // crFILE* file1 = cr_open(1, "text.txt", 'r');
    // unsigned char* buffer1 = (unsigned char*)calloc(1000, 1);
    // int bytesRead = cr_read(file1, buffer1, 1000);

    // /* printf("%d\n", bytesRead);
    // for(int i = 0; i<bytesRead; i++){
    //     printf("%c",buffer1[i]);
    // }
    // printf("\n");
    // cr_close(file1); */

    // crFILE* file2 = cr_open(1, "nuevo.txt", 'w');
    // int bytesWrite = cr_write(file2, buffer1, 848);
    // cr_close(file2);
    // free(buffer1);
    // cr_bitmap(1,false);
    // cr_ls(1);
    // getchar();
    // cr_rm(1,"nuevo.txt");
    // cr_bitmap(1,false);
    // cr_ls(1);
    // crFILE* file3 = cr_open(1, "nuevo.txt", 'r');
    // cr_close(file3);
    return 0;
}