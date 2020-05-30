#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "cr_API.h"
#include "../utils/utils.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
        (byte & 0x80 ? '1' : '0'), \
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

/* Currently storing just blockNumber and filename */
crFILE* cr_open(unsigned int disk, char *filename, char mode) {
    if (mode == 'r') {
        if (!cr_exists(disk, filename)) {
            printf("[ERROR] No such file \"%s\" on Disk %d.\n", filename, disk);
            exit(1);
        }
        FILE *f;
        int i, j;
        unsigned char buffer[S_BLOCK];
        unsigned char tempPath[29];
        unsigned char blockNumber[3];
        int offset = (disk - 1) * 512 * pow(1024, 2);

        f = fopen(binPath, "rb");
        fseek(f, offset, SEEK_SET);
        fread(buffer, S_BLOCK, 1, f);

        for (i = 0; i < S_BLOCK; i += 32) {
            if (buffer[i] & 0x80) {
                memset(tempPath, 0, sizeof(tempPath));
                for (j = 3; j < 32; j++) {
                    tempPath[j-3] = buffer[i + j];
                }
                // File is found
                if (!strcmp(filename, tempPath)) {
                    for (j = 0; j < 3; j++) {
                        blockNumber[j] = buffer[i + j];
                        // Remove first bit
                        if (j==0) {
                            blockNumber[j] = blockNumber[j] & 0x7F;
                        }
                    }
                    unsigned int blockAsUint = (unsigned int)blockNumber[0] << 16 |
                                               (unsigned int)blockNumber[1] << 8  |
                                               (unsigned int)blockNumber[2];
                    fclose(f);
                    crFILE* openFile = (crFILE*)malloc(sizeof(crFILE));
                    openFile->blockNumber = blockAsUint;
                    openFile->currentBlockToRead = 0;
                    openFile->lastByteRead = 0;
                    strcpy(openFile->filename, filename);
                    return openFile;
                }
            }
        }

    } else if (mode == 'w') {
         if (cr_exists(disk, filename)) {
            printf("[ERROR] File \"%s\" already exists on Disk %d.\n", filename, disk);
            exit(1);
        }
        crFILE* openFile = (crFILE*)malloc(sizeof(crFILE));
        strcpy(openFile->filename, filename);
        return openFile;
    }
}

int cr_read(crFILE *file_desc, void *buffer, int nbytes) {
    unsigned char indexBlock[S_BLOCK], indirectBlock[S_BLOCK], dataBlock[S_BLOCK], auxBuffer[nbytes], test[8];
    unsigned int blockPointers[2*(S_BLOCK/4)];
    unsigned int UcharAsUint;
    uint64_t fileSize;
    int i, j, extractSize, starPoint, byteCount = 0, pointerCount = 0, empty = 0;
    FILE* f = fopen(binPath, "rb");
    //Extract block pointers from idex block
    fseek(f, file_desc->blockNumber*S_BLOCK, SEEK_SET);
    fread(indexBlock, S_BLOCK, 1, f);
            fileSize =      (uint64_t)indexBlock[4] << 56 |
                            (uint64_t)indexBlock[5] << 48 | 
                            (uint64_t)indexBlock[6] << 40 |
                            (uint64_t)indexBlock[7] << 32 |
                            (uint64_t)indexBlock[8] << 24 |
                            (uint64_t)indexBlock[9] << 16 | 
                            (uint64_t)indexBlock[10] << 8 |
                            (uint64_t)indexBlock[11]; 
    printf("%ld", fileSize);
    printf("\n");
    for (i=12; i<8188; i+=4) {
            UcharAsUint = (unsigned int)indexBlock[i] << 24 |
                            (unsigned int)indexBlock[i+1] << 16 | 
                            (unsigned int)indexBlock[i+2] << 8  |
                            (unsigned int)indexBlock[i+3];
            if (UcharAsUint) {
                
                blockPointers[pointerCount] = UcharAsUint;
                pointerCount ++;
            }
    };
    //Check for simple indirect addressing
    UcharAsUint = (unsigned int)indexBlock[8188] << 24 |
                    (unsigned int)indexBlock[8189] << 16 | 
                    (unsigned int)indexBlock[8190] << 8  |
                    (unsigned int)indexBlock[8191];
    //If there is one, extract his pointers
    if (UcharAsUint!=0) {
                fseek(f, UcharAsUint*S_BLOCK, SEEK_SET);
                fread(indirectBlock, S_BLOCK, 1, f);
                for (i=0; i<8192; i+=4) {
                    UcharAsUint = (unsigned int)indirectBlock[i] << 24 |
                                    (unsigned int)indirectBlock[i+1] << 16 | 
                                    (unsigned int)indirectBlock[i+2] << 8  |
                                    (unsigned int)indirectBlock[i+3];
                    if (UcharAsUint!=0) {
                        blockPointers[pointerCount] = UcharAsUint;
                        pointerCount ++;
                    }
                };
            }
    //Then data is extracted from data blocks until reach the nbytes required
    if(nbytes > fileSize){
            nbytes = fileSize;
        }
   
    //If there is remaining data to read in a previus block, is extracted firts
    if(file_desc->lastByteRead != 0){
        starPoint = file_desc->lastByteRead;
        if(S_BLOCK - file_desc->lastByteRead > nbytes){
            extractSize = nbytes;
            file_desc->lastByteRead += nbytes;
        }else{
            extractSize = file_desc->lastByteRead;
            file_desc->lastByteRead = 0;
        }
        fseek(f, blockPointers[file_desc->currentBlockToRead - 1]*S_BLOCK, SEEK_SET);
        fread(dataBlock, S_BLOCK , 1, f);
        for(j=0; j < extractSize; j++){
            auxBuffer[j] = dataBlock[j + file_desc->lastByteRead];
        }
        byteCount += extractSize;
    }
    //Then the data is extracted from the point 
    for(i=file_desc->currentBlockToRead; i < pointerCount; i++){
        fseek(f, blockPointers[i]*S_BLOCK, SEEK_SET);
        if(byteCount + S_BLOCK > nbytes){
            extractSize = nbytes - byteCount;
            file_desc->currentBlockToRead = i + 1;
            file_desc->lastByteRead = extractSize;
            i = pointerCount;
        }else{
            extractSize = S_BLOCK;
            file_desc->currentBlockToRead = i + 1;
            file_desc->lastByteRead = 0;
        }
        
        fread(dataBlock, extractSize , 1, f);
        for(j=0; j < extractSize; j++){
            auxBuffer[byteCount + j] = dataBlock[j];
        }
        byteCount += extractSize;
    };
    
    if(byteCount == fileSize){
        file_desc->currentBlockToRead = 0;
        file_desc->lastByteRead = 0;
    }
    memcpy(buffer, auxBuffer, nbytes); 
    fclose(f);
    return byteCount;
}

int cr_write(crFILE *file_desc, void *buffer, int nytes) {}

int cr_close(crFILE *file_desc) {}

int cr_rm(unsigned int disk, char *filename) {
    unsigned int UcharAsUint;
    crFILE* file = cr_open(disk, filename, 'r');
    int i;

    FILE* bin = fopen(binPath, "rb");
    unsigned char buffer[S_BLOCK];
    fseek(bin, file->blockNumber*S_BLOCK, SEEK_SET);
    fread(buffer, S_BLOCK, 1, bin);

    unsigned char hardlinkCount[4];
    for (i=0; i<4; i++) { hardlinkCount[i] = buffer[i]; }
    UcharAsUint = (unsigned int)hardlinkCount[0] << 24 |
                    (unsigned int)hardlinkCount[1] << 16 |
                    (unsigned int)hardlinkCount[2] << 8  |
                    (unsigned int)hardlinkCount[3];
    /*if (UcharAsUint > 0) {
        printf("[ERROR] File \"%s\" can't be deleted because it has a hardlink.\n", filename);
        fclose(bin);
        exit(1);
    }*/
    
    int counter = 0;
    unsigned char currBlock[4];
    for (i=12; i<=8188; i++) {
        if (i%4 == 0 && i>12) {
            UcharAsUint = (unsigned int)currBlock[0] << 24 |
                          (unsigned int)currBlock[1] << 16 | 
                          (unsigned int)currBlock[2] << 8  |
                          (unsigned int)currBlock[3];
            if (UcharAsUint > 0) {
                printf("%u %i\n", UcharAsUint, i);
            }
            
            counter = 0;
            memset(currBlock, 0, sizeof(currBlock));
        }
        currBlock[counter] = buffer[i];
        counter++;
    };
}

int cr_hardlink(unsigned int disk, char *orig, char *dest) {
    FILE *storage = fopen(binPath, "rb+");
    unsigned char *buffer = (unsigned char *)malloc(S_BLOCK);
    fseek(storage, (disk - 1) * S_PARTITION, SEEK_SET);
    fread(buffer, 1, S_BLOCK, storage);
    unsigned char *new_file_entry = (unsigned char *)calloc(32, 1);
    for (int i = 3; i < 32; i++) {
        if(dest[i - 3] == '\0')
            break;
        new_file_entry[i] = dest[i - 3];
    }

    for (int i = 0; i < S_BLOCK; i += 32){
        if(cmp_filename(&buffer[i], orig)){
            for (int j = 0; j < 3; j++)
                new_file_entry[j] = buffer[j];
            break;
        }
    }

    for (int i = 0; i < S_BLOCK; i += 32)
    {
        if (!(buffer[i] & 0x80))
        {
            for (int j = 0; j < 32; j++)
                buffer[i + j] = new_file_entry[j];
            break;
        }
    }

    fseek(storage, (disk - 1) * S_PARTITION, SEEK_SET);
    fwrite(buffer, 1, S_BLOCK, storage);

    int dir = 0 | (new_file_entry[2]) | (new_file_entry[1] << 8) | ((new_file_entry[0] ^ 0x80) << 16);
    fseek(storage, dir * S_BLOCK, SEEK_SET);
    fread(buffer, 1, S_BLOCK, storage);

    swap(&buffer[0], &buffer[3]);
    swap(&buffer[1], &buffer[2]);
    int *refs = (int *)buffer;
    (*refs)++;
    swap(&buffer[0], &buffer[3]);
    swap(&buffer[1], &buffer[2]);

    fseek(storage, dir * S_BLOCK, SEEK_SET);
    fwrite(buffer, 1, S_BLOCK, storage);

    free(buffer);
    fclose(storage);
}

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
