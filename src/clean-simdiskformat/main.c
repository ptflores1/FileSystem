
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../cr_API/cr_API.h"

int main() {
    cr_mount("simdiskformat.bin");

    printf("Files in partition 1:\n");
    cr_ls(1);

    if (cr_exists(1, "great_success.gif")) {
        cr_rm(1, "great_success.gif");
    }
    if (cr_exists(1, "vid.mkv")) {
        cr_rm(1, "vid.mkv");
    }
    if (cr_exists(1, "tenor.gif")) {
        cr_rm(1, "tenor.gif");
    }
    if (cr_exists(1, "texto-importante.txt")) {
        cr_rm(1, "texto-importante.txt");
    }

    printf("Files in partiton 1 after removing:\n");
    cr_ls(1);

    return 0;
}
