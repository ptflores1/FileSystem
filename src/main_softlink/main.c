#include <stdbool.h>
#include <stdio.h>
#include "../cr_API/cr_API.h"
#include <stdlib.h>

void pause()
{
    int aux = 0;
    while (aux != 1)
        scanf("%d", &aux);
}

void lsl()
{
    FILE *f = fopen("/Volumes/TOURO/simdiskformat.bin", "r");

    unsigned char *buffer = (unsigned char *)malloc(8 * 1024);
    fseek(f, 512 * 1024 * 1024 * 1, SEEK_SET);
    fread(buffer, 1, 8 * 1024, f);

    for (int i = 0; i < 8 * 1024; i += 32)
    {
        if (buffer[i] & 0x80)
        {
            printf("%02x %02x %02x %s\n", buffer[i], buffer[i + 1], buffer[i + 2], &buffer[i + 3]);
        }
    }

    free(buffer);
    fclose(f);
}

int main(){
    cr_mount("/Volumes/TOURO/simdiskformat.bin");

    printf("\nCargando tanjiro.png en disco 1...\n");
    cr_load(1, "tanjiro.png");

    pause();

    printf("\nHaciendo softlink de tanjiro.png en disco 2...\n");
    cr_softlink(1, 2, "tanjiro.png", "");

    printf("ls de disco 2\n\n");
    cr_ls(2);


    lsl();
    
    cr_rm(2, "1/tanjiro.png");
    cr_rm(1, "tanjiro.png");
    
    return 0;
}