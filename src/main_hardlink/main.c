#include <stdbool.h>
#include <stdio.h>
#include "../cr_API/cr_API.h"
#include <stdlib.h>

void pause()
{
    int aux = 0;
    while (aux != 1) scanf("%d", &aux);
}

void filerefs(){
    FILE *f = fopen("/Volumes/TOURO/simdiskformat.bin", "r");

    unsigned char *buffer = (unsigned char *)malloc(8 * 1024);
    fseek(f, 512 * 1024 * 1024 * 2, SEEK_SET);
    fread(buffer, 1, 8 * 1024, f);
    int dir;
    for (int i = 0; i < 8 * 1024; i += 32)
    {
        if (buffer[i] & 0x80)
        {
            printf("%02x %02x %02x %s\n", buffer[i], buffer[i + 1], buffer[i + 2], &buffer[i + 3]);
            dir = 0 | (buffer[i + 2]) | (buffer[i + 1] << 8) | ((buffer[i + 0] ^ 0x80) << 16);
        }
    }
    printf("Numero de bloque del archivo: %d\n");

    fseek(f, dir * 8 * 1024, SEEK_SET);
    fread(buffer, 1, 8 * 1024, f);

    swap(&buffer[0], &buffer[3]);
    swap(&buffer[1], &buffer[2]);
    int *refs = (int *)buffer;
    printf("Referencias al archivo: %d\n", (*refs));

    free(buffer);
    fclose(f);
}

int main(){
    cr_mount("/Volumes/TOURO/simdiskformat.bin");

    printf("\nCargando tanjiro.png...\n");
    cr_load(3, "tanjiro.png");
    printf("ls\n");
    cr_ls(3);

    filerefs();

    pause();

    printf("\nHaciendo hardlink de tanjiro.png...\n");
    cr_hardlink(3, "tanjiro.png", "otro_tanjiro.png");
    printf("ls\n");
    cr_ls(3);

    filerefs();

    pause();

    printf("\nBorrando tanjiro.png...\n");
    cr_rm(3, "tanjiro.png");
    filerefs();

    pause();

    cr_rm(3, "otro_tanjiro.png");
    return 0;
}