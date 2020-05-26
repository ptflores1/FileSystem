#include <stdio.h>
#include <math.h>
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
/*
    const int S_1KB = 1024;
    const int S_1MB = 1024 * S_1KB;
    const int S_BLOCK = 8 * S_1KB;
    const int S_PARTITION = 512 * S_1MB;
    const int S_DIR_ENTRY = 32;
*/
const int S_1KB = 1024;
const int S_1MB = 1024 * 1024;
const int S_BLOCK = 8 * 1024;
const int S_PARTITION = 512 * 1024 * 1024;
const int S_DIR_ENTRY = 32;

// General functions
void cr_mount(char *diskname)
{
    binPath = diskname;
}

int _cr_bitmap(unsigned int disk, bool hex)
{
    FILE *storage = fopen(binPath, "rb");
    fseek(storage, (disk - 1) * S_PARTITION + S_BLOCK, SEEK_SET);
    char *buffer = malloc(S_BLOCK);
    fread(buffer, 1, S_BLOCK, storage);

    int used_blocks = 0;
    if (hex)
    {
        for (int i = 0; i < S_BLOCK; i++)
        {
            if (i % 16 == 0)
                printf("\n%04x:  ", (disk - 1) * S_PARTITION + S_BLOCK + i);
            printf("%02x ", (unsigned char)buffer[i]);

            used_blocks += count_bits((unsigned char)buffer[i]);
        }
    }
    else
    {
        for (int i = 0; i < S_BLOCK; i++)
        {
            if (!(i % 16))
                printf("\n%04x:  ", (disk - 1) * S_PARTITION + S_BLOCK + i);
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
    if (disk)
    {
        _cr_bitmap(disk, hex);
    }
    else
    {
        int used_blocks[] = {0, 0, 0, 0};
        for (int i = 1; i <= 4; i++)
        {
            printf("\n\nDisk %d\n", i);
            used_blocks[i - 1] = _cr_bitmap(i, hex);
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

int cr_exists(unsigned int disk, char *filename)
{
    FILE *storage = fopen(binPath, "rb");
    unsigned char *buffer = (unsigned char *)malloc(S_BLOCK);
    fread(buffer, 1, S_BLOCK, storage);

    int ret = 0;
    for (int i = 0; i < S_BLOCK; i += 32)
    {
        if (cmp_filename(&buffer[i], filename))
        {
            ret = 1;
            break;
        }
    }

    free(buffer);
    fclose(storage);
    return ret;
}

void cr_ls(unsigned int disk)
{
    FILE *f;
    unsigned char buffer[S_BLOCK];
    int offset = (disk - 1) * 512 * pow(1024, 2);

    f = fopen(binPath, "rb");
    fseek(f, offset, SEEK_SET);
    fread(buffer, S_BLOCK, 1, f);

    for (int i = 0; i < S_BLOCK; i += 32)
    {
        if (buffer[i] & 0x80)
        {
            printf("> ");
            for (int j = 3; j < 32; j++)
                printf("%c", buffer[i + j]);
            printf("\n");
        }
    }
    fclose(f);
}

// File management functions

crFILE *cr_open(unsigned int disk, char *filename, char mode) {}

int cr_read(crFILE *file_desc, void *buffer, int nbytes) {}

int cr_write(crFILE *file_desc, void *buffer, int nytes) {}

int cr_close(crFILE *file_desc) {}

int cr_rm(unsigned int disk, char *filename) {}

int cr_hardlink(unsigned int disk, char *orig, char *dest) {}

int cr_softlink(unsigned int disk_orig, unsigned int disk_dest, char *orig, char *dest) {
    char filename[29];
    sprintf(filename, "%d/%s", disk_orig, orig);
    
    FILE *storage = fopen(binPath, "rb+");
    fseek(storage, (disk_dest - 1) * S_PARTITION, SEEK_SET);
    unsigned char *buffer = (unsigned char *)malloc(S_BLOCK);
    fread(buffer, 1, S_BLOCK, storage);

    for (int i = 0; i < S_BLOCK; i += 32)
    {
        if(!(buffer[i] & 0x80)){
            buffer[i] |= 0x80;
            for (int j = 3; j < 32; j++)
                buffer[i + j] = filename[j - 3];
            break;
        }
    }
    fseek(storage, (disk_dest - 1) * S_PARTITION, SEEK_SET);
    fwrite(buffer, 1, S_BLOCK, storage);

    free(buffer);
    fclose(storage);
}

int cr_unload(unsigned disk, char *orig, char *dest) {}

int cr_load(unsigned disk, char *orig) {}
