#include <stdbool.h>
#include <stdio.h>
#include "../cr_API/cr_API.h"
#include <stdlib.h>


void pause(){
    int aux = 0;
    while (aux != 1)scanf("%d", &aux);
}

int main(int argc, char** argv){
    cr_mount("/Volumes/TOURO/simdiskformat.bin");

    printf("cr_bitmap(1, false)\n");
    cr_bitmap(1, false);

    pause();

    printf("\n\ncr_bitmap(1, true)\n");
    cr_bitmap(1, true);

    pause();
    printf("\nCargando todo.pdf...\n");
    cr_load(1, "todo.pdf");
    printf("\nls\n");
    cr_ls(1);
    printf("\ncr_bitmap(1, true)\n");
    cr_bitmap(1, true);

    pause();

    printf("\nCargando tanjiro.png...\n");
    cr_load(1, "tanjiro.png");
    printf("\nls\n");
    cr_ls(1);
    printf("\ncr_bitmap(1, true)\n");
    cr_bitmap(1, true);

    pause();
    printf("\ncr_bitmap(1, false)\n");
    cr_bitmap(1, false);

    pause();

    printf("\ncr_bitmap(0, true)\n");
    cr_bitmap(0, true);

    pause();

    printf("\nRemoviendo todo.pdf...\n");
    cr_rm(1, "todo.pdf");
    printf("\nls\n");
    cr_ls(1);
    printf("\ncr_bitmap(1, true)\n");
    cr_bitmap(1, true);

    pause();

    printf("\nRemoviendo tanjiro.png...\n");
    cr_rm(1, "tanjiro.png");
    printf("\nls\n");
    cr_ls(1);
    printf("\ncr_bitmap(1, true)\n");
    cr_bitmap(1, true);

    pause();

    printf("\ncr_bitmap(0, true)\n");
    cr_bitmap(0, true);

    return 0;
}
