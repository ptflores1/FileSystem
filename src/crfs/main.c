#include <stdbool.h>
#include <stdio.h>
#include "../cr_API/cr_API.h"
#include <stdlib.h>

char* binPath;

int main() {
    cr_mount("simdiskfilled.bin");
    //cr_ls(1);
    
    cr_open(1, "Baroque.mp3", 'r');
    cr_open(1, "Baroque_new.mp3", 'w');
    return 0;
}