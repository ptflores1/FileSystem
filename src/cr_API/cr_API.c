#include <stdio.h>
#include <stdlib.h>
#include "cr_API.h"
#include "../utils/utils.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
    (byte & 0x80 ? '1' : '0'),     \
        (byte & 0x40 ? '1' : '0'), \
        (byte & 0x20 ? '1' : '0'), \
        (byte & 0x10 ? '1' : '0'), \
        (byte & 0x08 ? '1' : '0'), \
        (byte & 0x04 ? '1' : '0'), \
        (byte & 0x02 ? '1' : '0'), \
        (byte & 0x01 ? '1' : '0')

// constants
const int S_1KB = 1024;
const int S_1MB = 1024 * S_1KB;
const int S_BLOCK = 8 * S_1KB;
const int S_PARTITION = 512 * S_1MB;
const int S_DIR_ENTRY = 32;

// General functions
void cr_mount(char *diskname) {
    binPath = diskname;
}

int _cr_bitmap(unsigned int disk, bool hex) {
    FILE *storage = fopen(binPath, "rb");
    fseek(storage, (disk - 1) * S_PARTITION + S_BLOCK, SEEK_SET);
    char *buffer = malloc(S_BLOCK);
    fread(buffer, 1, S_BLOCK, storage);

    int used_blocks = 0;
    if (hex)
    {
        for (int i = 0; i < S_BLOCK; i++)
        {
            if (i % 16 == 0) printf("\n%04x:  ", (disk - 1) * S_PARTITION + S_BLOCK + i);
            printf("%02x ", (unsigned char)buffer[i]);

            used_blocks += count_bits((unsigned char)buffer[i]);
        }
    }
    else
    {
        for (int i = 0; i < S_BLOCK; i++)
        {
            if (!(i % 16)) printf("\n%04x:  ", (disk - 1) * S_PARTITION + S_BLOCK + i);
            printf(BYTE_TO_BINARY_PATTERN " ", BYTE_TO_BINARY(buffer[i]));

            used_blocks += count_bits((unsigned char)buffer[i]);
        }
    }
    printf("\n");
    free(buffer);
    fclose(storage);
    
    return used_blocks;
}

void cr_bitmap(unsigned int disk, bool hex)
{
    if(disk){
        _cr_bitmap(disk, hex);
    }
    else
    {
        int used_blocks[] = {0,0,0,0};
        for (int i = 1; i <= 4; i++)
        {
            printf("\n\nDisk %d\n", i);
            used_blocks[i-1] = _cr_bitmap(i, hex);
        }
        printf("\n\n");
        for (int i = 1; i <= 4; i++)
        {
            printf("Disk %d\n", i);
            printf("Used blocks: %d\n", used_blocks[i - 1]);
            printf("Unused blocks: %d\n", S_PARTITION / S_BLOCK - used_blocks[i - 1]);
        }
    }
}

int cr_exists(unsigned int disk, char *filename) {}

void cr_ls(unsigned int disk) {}

// File management functions

crFILE *cr_open(unsigned int disk, char *filename, char mode) {}

int cr_read(crFILE *file_desc, void *buffer, int nbytes) {}

int cr_write(crFILE *file_desc, void *buffer, int nytes) {}

int cr_close(crFILE *file_desc) {}

int cr_rm(unsigned int disk, char *filename) {}

int cr_hardlink(unsigned int disk, char *orig, char *dest) {}

int cr_softlink(unsigned int disk_orig, unsigned int disk_dest, char *orig, char *dest) {}

int cr_unload(unsigned disk, char *orig, char *dest) {}

int cr_load(unsigned disk, char *orig) {}
