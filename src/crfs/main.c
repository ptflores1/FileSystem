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
    // cr_softlink(1, 2, "somefiel.some", "some");
    // cr_hardlink(3, "yes.mp3", "no.mp3");
    cr_ls(1);
    return 0;
}