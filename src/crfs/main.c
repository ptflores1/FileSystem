#include <stdbool.h>
#include <stdio.h>
#include "../cr_API/cr_API.h"
#include <stdlib.h>

char* binPath;

int main() {
    cr_mount("simdiskfilled.bin");
    crFILE* file = cr_open(2, "text.txt", 'r');
    cr_ls(1);
    unsigned char buffer[300];
    int bytesRead = cr_read(file, &buffer, sizeof(buffer));
    printf("\n%d\n", bytesRead);
    printf("Content:\n");
    for(int i = 0; i < bytesRead; i++){
        printf("%c", buffer[i]);
    } 
    printf("\n"); 
    unsigned char buffer2[300];
    bytesRead = cr_read(file, &buffer2, sizeof(buffer2));
    printf("\n%d\n", bytesRead);
    printf("Content:\n");
    for(int i = 0; i < bytesRead; i++){
        printf("%c", buffer2[i]);
    } 
    printf("\n"); 
    unsigned char buffer3[300];
    bytesRead = cr_read(file, &buffer3, sizeof(buffer3));
    printf("\n%d\n", bytesRead);
    printf("Content:\n");
    for(int i = 0; i < bytesRead; i++){
        printf("%c", buffer3[i]);
    } 
    printf("\n"); 
    

    return 0;
}