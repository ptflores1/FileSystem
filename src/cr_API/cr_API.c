
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "cr_API.h"
#include <sys/stat.h>
#include <dirent.h> 
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

// Global variables
static char *binPath;

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
    fseek(storage, S_PARTITION * (disk - 1), SEEK_SET);
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
    unsigned char* buffer = (unsigned char*)malloc(S_BLOCK);
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
    free(buffer);
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
        int i, j;
        
        char usedFilename[29] = "";
        unsigned int usedDisk = disk;
        if (filename[1] == '/') {
            char newDisk[1]; newDisk[0] = filename[0];
            usedDisk = (unsigned int)newDisk[0] - 48;
            for (i=2; i<strlen(filename); i++) usedFilename[i-2] = filename[i];
            if (!cr_exists(usedDisk, usedFilename)) {
                printf("[ERROR] No such file \"%s\" on Disk %d (Referenced from softlink \"%s\").\n", usedFilename, usedDisk, filename);
                return 0;
            }
        } else {
            strcpy(usedFilename, filename);
        }

        FILE *f;
        unsigned char* buffer = (unsigned char*)malloc(S_BLOCK);
        unsigned char blockNumber[3];
        int offset = (usedDisk - 1) * 512 * pow(1024, 2);

        f = fopen(binPath, "rb");
        fseek(f, offset, SEEK_SET);
        fread(buffer, S_BLOCK, 1, f);

        for (i = 0; i < S_BLOCK; i += 32) {
            if (buffer[i] & 0x80) {
                // File is found
                if (cmp_filename(&buffer[i], usedFilename)) {
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
                    crFILE* openFile = (crFILE*)calloc(1, sizeof(crFILE));
                    openFile->blockNumber = blockAsUint;
                    openFile->currentBlockToRead = 0;
                    openFile->lastByteRead = 0;
                    memcpy(openFile->filename, usedFilename, strlen(usedFilename));
                    free(buffer);
                    return openFile;
                }
            }
        }
    } else if (mode == 'w') {
         if (cr_exists(disk, filename)) {
            printf("[ERROR] File \"%s\" already exists on Disk %d.\n", filename, disk);
            exit(1);
        }
        crFILE* openFile = (crFILE*)calloc(1, sizeof(crFILE));
        openFile->diskNumber = disk;
        memcpy(openFile->filename, filename, strlen(filename));
        return openFile;
    }
}

int cr_read(crFILE *file_desc, void *buffer, int nbytes) {
    unsigned char* indexBlock = (unsigned char*)calloc(S_BLOCK,1);
    unsigned char* indirectBlock = (unsigned char*)calloc(S_BLOCK,1);
    unsigned char* dataBlock = (unsigned char*)calloc(S_BLOCK,1);

    unsigned int blockPointers[2*(S_BLOCK/4)];
    unsigned int UcharAsUint;
    uint64_t fileSize;
    int i, j, extractSize, startPoint, byteCount = 0, pointerCount = 0, empty = 0;
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
    if(nbytes > fileSize)nbytes = fileSize;
    unsigned char* auxBuffer = (unsigned char*)calloc(nbytes,1);
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
    //If there is remaining data to read in a previus block, is extracted firts
    if(file_desc->lastByteRead != 0){
        startPoint = file_desc->lastByteRead;
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
            auxBuffer[j] = dataBlock[j + startPoint];
        }
        if(file_desc->lastByteRead > fileSize){
            byteCount = fileSize - startPoint;
        }else{
            byteCount += extractSize;
        }
        
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
    free(indexBlock); free(indirectBlock); free(dataBlock); free(auxBuffer);
    return byteCount;
}

int cr_write(crFILE *file_desc, void *buffer, int nbytes) {
    //Define variables and costants to use
    int i, j, currBit , blockCounter,searchCounter=0, bitmapOffset=(file_desc->diskNumber-1)*S_PARTITION+S_BLOCK;
    unsigned int blockNumber, switcher;
    unsigned int* blockPointers;
    FILE* bin;
    unsigned char* bitmap = (unsigned char*)calloc(S_BLOCK, 1);
    //Calculate the number of data blocks to use plus the index block
    blockCounter = (int)ceil((double)nbytes / (double)S_BLOCK) + 1;
    //If simple indirect addressing is needed plus 1 to the block counter
    if(blockCounter>2045)blockCounter++;
    //Determinate in the bitmap wich block to use for de index block and data blocks
    blockPointers = (unsigned int*)calloc(blockCounter, sizeof(unsigned int));
    bin = fopen(binPath, "rb+");
    fseek(bin, bitmapOffset, SEEK_SET);
    fread(bitmap, S_BLOCK, 1, bin);
    for(i = 0; i<S_BLOCK && searchCounter < blockCounter; i++){
        currBit = 0x80;
        for(j=0;j<8 && searchCounter < blockCounter;j++){
            if(!(bitmap[i] & currBit)){
                //save the new pointer
                blockNumber = (i*8 + j) +(file_desc->diskNumber - 1)*S_BLOCK*8;
                blockPointers[searchCounter] = blockNumber;
                searchCounter++;
                //set the bit used in the bitmap to 1
                switcher = 0x80 >> (blockNumber)%8;
                bitmap[i] |= switcher;
            }
        currBit = currBit >> 1;    
        }
    }
    //Determinate if the space found in the disk is enough to write the required nbytes
    if(searchCounter < blockCounter){
        if(searchCounter <= 1){
            free(bitmap);
            free(blockPointers);
            return 0;
        }
        if(searchCounter > 2045){
            nbytes = (searchCounter - 2) * S_BLOCK;
        }else{
            nbytes = (searchCounter - 1) * S_BLOCK;
        }
        blockCounter = searchCounter;
    }
    fseek(bin, bitmapOffset, SEEK_SET);
    fwrite(bitmap, 1, S_BLOCK, bin);
    free(bitmap);
    //Go to the directory block and create a new entry
    unsigned char* directory = (unsigned char*)calloc(S_BLOCK, 1);
    fseek(bin, bitmapOffset - S_BLOCK, SEEK_SET);
    fread(directory, S_BLOCK, 1, bin);
    for (i = 0; i < S_BLOCK; i += 32) {
        if (!(directory[i] & 0x80)) {
            //set the valid bit to one
            directory[i]|= 0x80; 
            //add the index block pointer
            for(j=0;j<3;j++)directory[i+j] |= (blockPointers[0] >> (2-j)*8) & 0xFF;
            //add the name of the file
            for(j=3;j<32;j++)directory[i+j] = file_desc->filename[j-3];
            break;
        }
    }
    fseek(bin, bitmapOffset - S_BLOCK, SEEK_SET);
    fwrite(directory, 1, S_BLOCK, bin);
    free(directory);

    //Next, go to the index block chosen, add the file size and all the pointers. Use the simple indirect addressing if necesary. Set the unused pointer spaces to 0
    unsigned char* indexBlock = (unsigned char*)calloc(S_BLOCK, 1);
    unsigned char* indirectBlock = (unsigned char*)calloc(S_BLOCK, 1);
    fseek(bin, blockPointers[0] * S_BLOCK, SEEK_SET);
    fread(indexBlock, S_BLOCK, 1, bin);
    // add the file size
    for(i=0;i<8;i++)indexBlock[i+4]=((uint64_t)nbytes >> (7-i)*8) & 0xFF;
    // add the pointers to data blocks
    for(i=12;i<8188 && i<(blockCounter-1)*4+12;i+=4){
        //add pointer
        for(j=0;j<4;j++)indexBlock[i+j]=(blockPointers[(i/4)-2] >> (3-j)*8) & 0xFF;
    }
    // if indirect addressing is not necesary, complete the rest of the pointers space with 0 
    if(blockCounter < 2046){
        for(i=(blockCounter-1)*4+12;i<8192;i++)indexBlock[i] = 0;
        fseek(bin, blockPointers[0] * S_BLOCK, SEEK_SET);
        fwrite(indexBlock, 1, S_BLOCK, bin);
    }
    // otherwise, add the rest of the pointers to the simple indirect address block 
    else{
        //set the pointer to the simple indirect address block at the end of the index block
        for(i=0;i<4;i++)indexBlock[8188+i] = (blockPointers[blockCounter-1] >> (3-i)*8) & 0xFF;
        fseek(bin, blockPointers[0] * S_BLOCK, SEEK_SET);
        fwrite(indexBlock, 1, S_BLOCK, bin);

        //go to the simple indirect address block
        fseek(bin, blockPointers[blockCounter-1] * S_BLOCK, SEEK_SET);
        fread(indirectBlock, S_BLOCK, 1, bin);
        //add the remaining pointers to the simple indirect address block
        for(i=0;i<(blockCounter-2046)*4;i+=4){
            //add pointer
            for(j=0;j<4;j++)indirectBlock[i+j]=(blockPointers[(i/4)+2045] >> (3-j)*8) & 0xFF;
        }
        //complete the rest of the pointers space on the simple indirect address block with 0 
        for(i=(blockCounter-2046)*4;i<8192;i++)indexBlock[i] = 0;
        fseek(bin, blockPointers[blockCounter-1] * S_BLOCK, SEEK_SET);
        fwrite(indirectBlock, 1, S_BLOCK, bin);
        blockCounter--;
    }
    free(indexBlock);
    free(indirectBlock);
    //Finally, write the data in the data blocks
    unsigned char* auxBuffer = (unsigned char*)calloc(blockCounter*S_BLOCK, 1);
    unsigned char* dataBlock = (unsigned char*)calloc(S_BLOCK, 1);
    memcpy(auxBuffer, buffer, nbytes);
    for(i=1;i<blockCounter;i++){
        //go to the data block
        fseek(bin, blockPointers[i] * S_BLOCK, SEEK_SET);
        fread(dataBlock, S_BLOCK, 1, bin);

        for(j=0;j<S_BLOCK;j++){
            dataBlock[j]=auxBuffer[j+(i-1)*S_BLOCK];
        }
        fseek(bin, blockPointers[i] * S_BLOCK, SEEK_SET);
        fwrite(dataBlock, 1, S_BLOCK, bin);
    }
    free(dataBlock);
    free(auxBuffer);
    free(blockPointers);
    fclose(bin);
    return nbytes;
}

int cr_close(crFILE *file_desc) { free(file_desc); return 0; }


unsigned int UcharBlockAsUint(unsigned char* UcharBlock) {
    return (unsigned int)UcharBlock[0] << 24 |
           (unsigned int)UcharBlock[1] << 16 |
           (unsigned int)UcharBlock[2] << 8  |
           (unsigned int)UcharBlock[3];
}

unsigned char* UintBlockAsUchar(unsigned int UintBlock) {
    unsigned char* UcharBlock = (unsigned char*)malloc(4);
    UcharBlock[0] = (UintBlock >> 24) & 0xFF;
    UcharBlock[1] = (UintBlock >> 16) & 0xFF;
    UcharBlock[2] = (UintBlock >> 8 ) & 0xFF;
    UcharBlock[3] = (UintBlock      ) & 0xFF;
    return UcharBlock;
}

void _cr_rm_path(unsigned int disk, char* filename) {
    int i, j;
    unsigned char* buffer = (unsigned char*)malloc(S_BLOCK);
    FILE* bin = fopen(binPath, "rb+");
    fseek(bin, (disk-1)*S_PARTITION, SEEK_SET);
    fread(buffer, S_BLOCK, 1, bin);

    for (i = 0; i < S_BLOCK; i += 32) {
        if (buffer[i] & 0x80) {
            // File is found
            if (cmp_filename(&buffer[i], filename)) {
                for (j=0; j<32; j++) buffer[i + j] = 0;
                fseek(bin, (disk-1)*S_PARTITION, SEEK_SET);
                fwrite(buffer, 1, S_BLOCK, bin);
                fclose(bin);
                free(buffer);
                return;
            }
        }
    }
                
}

int cr_rm(unsigned int disk, char *filename) {
    unsigned int blockNumber, hardlinkNumber;
    int i, j;
    crFILE* file = cr_open(disk, filename, 'r');

    /* Remove filename's path from directory block */
    _cr_rm_path(disk, filename);
    /* If its a softlink, end */
    if (filename[1] == '/') { return 0; }

    FILE* bin = fopen(binPath, "rb+");
    unsigned char* indexBuffer = (unsigned char*)malloc(S_BLOCK);
    fseek(bin, file->blockNumber*S_BLOCK, SEEK_SET);
    fread(indexBuffer, S_BLOCK, 1, bin);

    /* Check if the file has hardlinks */
    unsigned char hardlinkCount[4];
    for (i=0; i<4; i++) { hardlinkCount[i] = indexBuffer[i]; }
    hardlinkNumber = UcharBlockAsUint(hardlinkCount);
    if (hardlinkNumber > 1) {
        /* Decreases hardlink count of filename's index block */
        unsigned char* hardlinkCountUpdated = UintBlockAsUchar(hardlinkNumber - 1);
        for (i=0; i<4; i++) { indexBuffer[i] = hardlinkCountUpdated[i]; }
        fseek(bin, file->blockNumber*S_BLOCK, SEEK_SET);
        fwrite(indexBuffer, 1, S_BLOCK, bin);

        fclose(bin);
        free(hardlinkCountUpdated);
        free(indexBuffer);
        return 0; 
    }

    /* Delete blocks [pointers] from bitmap */
    unsigned char* bitmapBuffer = (unsigned char*)malloc(S_BLOCK);
    fseek(bin, (disk - 1) * S_PARTITION + S_BLOCK, SEEK_SET);
    fread(bitmapBuffer, S_BLOCK, 1, bin);

    unsigned char currBlock[4];
    int normalized;
    unsigned int switcher;
    for (i = 12; i < S_BLOCK - 4; i += 4) {
        for (j = 0; j < 4; j++) currBlock[j] = indexBuffer[i + j];   
        blockNumber = UcharBlockAsUint(currBlock);
        if (blockNumber > 0) {
            normalized = (int)floor((blockNumber - 65536*(disk-1))/8);
            switcher = ~(0x80 >> (blockNumber)%8);
            bitmapBuffer[normalized] &= switcher;
        }
        memset(currBlock, 0, sizeof(currBlock));
    }
    /* Deletes index block pointer */
    normalized = (int)floor((file->blockNumber - 65536*(disk-1))/8);
    switcher = ~(0x80 >> (file->blockNumber)%8);
    bitmapBuffer[normalized] &= switcher;
     /* Delete blocks [indirect addressing] from bitmap */
    for (i=S_BLOCK-4; i<S_BLOCK; i++) { currBlock[i - (S_BLOCK-4)] = indexBuffer[i]; }
    unsigned int indirectBlock = UcharBlockAsUint(currBlock);
    if (indirectBlock > 0) {
        unsigned char* indirectBuffer = (unsigned char*)malloc(S_BLOCK);
        fseek(bin, indirectBlock*S_BLOCK, SEEK_SET);
        fread(indirectBuffer, S_BLOCK, 1, bin);

        for (i = 0; i < S_BLOCK; i += 4) {
            for (j = 0; j < 4; j++) currBlock[j] = indirectBuffer[i + j];
            blockNumber = UcharBlockAsUint(currBlock);
            if (blockNumber > 0) {
                normalized = (int)floor((blockNumber - 65536*(disk-1))/8);
                switcher = ~(0x80 >> (blockNumber)%8);
                bitmapBuffer[normalized] &= switcher;
            }
            memset(currBlock, 0, sizeof(currBlock));
        }
        free(indirectBuffer);
        /* Deletes indirect block pointer */
        normalized = (int)floor((indirectBlock - 65536*(disk-1))/8);
        switcher = ~(0x80 >> (indirectBlock)%8);
        bitmapBuffer[normalized] &= switcher;
    }
    
    fseek(bin, (disk - 1) * S_PARTITION + S_BLOCK, SEEK_SET);
    fwrite(bitmapBuffer, 1, S_BLOCK, bin);
    fclose(bin);
    free(bitmapBuffer);
    free(indexBuffer);
    return 0;
}

int cr_hardlink(unsigned int disk, char *orig, char *dest) {

    if (!cr_exists(disk, orig)) return 0;

    unsigned char *new_file_entry = (unsigned char *)calloc(32, 1);
    for (int i = 3; i < 32; i++)
    {
        if (dest[i - 3] == '\0')
            break;
        new_file_entry[i] = dest[i - 3];
    }

    if(cr_exists(disk, &new_file_entry[3])) return 0;

    FILE *storage = fopen(binPath, "rb+");
    unsigned char *buffer = (unsigned char *)malloc(S_BLOCK);
    fseek(storage, (disk - 1) * S_PARTITION, SEEK_SET);
    fread(buffer, 1, S_BLOCK, storage);

    for (int i = 0; i < S_BLOCK; i += 32){
        if(cmp_filename(&buffer[i], orig)){
            for (int j = 0; j < 3; j++)
                new_file_entry[j] = buffer[i + j];
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

    return 1;
}

int cr_softlink(unsigned int disk_orig, unsigned int disk_dest, char *orig, char *dest) {
    char filename[29];
    filename[0] = disk_orig + '0';
    filename[1] = '/';

    int flag = 1;
    for (int i = 2; i < 29; i++)
    {
        filename[i] = 0;
        if(orig[i - 2] == 0) flag = 0;
        if (flag) filename[i] = orig[i - 2];
    }
    
    if (cr_exists(disk_dest, filename)) return 0;

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
    return 1;
}

char **_cr_get_filenames(unsigned disk, int *filename_count);
int _cr_unload_partition(unsigned disk, char *dest);
int _cr_unload_file(unsigned disk, char *orig, char *dest);

int cr_unload(unsigned disk, char *orig, char *dest)
{
    // unload entire partition
    if (disk == 0) {
        char *dirname;
         // create dest if it doesnt exist
        struct stat sb;
        if (stat(dest, &sb) == -1) {
            mkdir(dest, 0755);
        }
        for (int d = 1; d <= 4; d++) {
            char partstr[2]; 
            sprintf(partstr, "%d", d);
            dirname = join_dir_file(dest, partstr);
            if (stat(dirname, &sb) == -1) {
                mkdir(dirname, 0755);
            }  
            _cr_unload_partition(d, dirname);
            free(dirname);
        }
    } else if(!orig) {
        // create dest if it doesnt exist
        struct stat sb;
        if (stat(dest, &sb) == -1) {
            mkdir(dest, 0755);
        }
        _cr_unload_partition(disk, dest);
    } else {
        _cr_unload_file(disk, orig, dest);
    }

    return 0;
}

int _cr_unload_file(unsigned disk, char *orig, char *dest)
{
    FILE *outfile;
    crFILE *infile;
    int nbytes = 0;
    const int BUFF_SIZE = 1000000;
    unsigned char *buffer = malloc(BUFF_SIZE);
    
    infile = cr_open(disk, orig, 'r');
    if (!infile) {
        free(buffer);
        return 0;
    }
    outfile = fopen(dest, "wb");
    if (!outfile) {
        perror(dest);
        exit(EXIT_FAILURE);
    }

    nbytes = cr_read(infile, buffer, BUFF_SIZE);
    fwrite(buffer, 1, nbytes, outfile);
    while(nbytes >= BUFF_SIZE) {
        nbytes = cr_read(infile, buffer, BUFF_SIZE);
        fwrite(buffer, 1, nbytes, outfile);
        // printf("%d\n", nbytes);
    }

    cr_close(infile);
    fclose(outfile);
    free(buffer);
    return 1;
}

int _cr_unload_partition(unsigned disk, char *dest)
{
    char *out_filename;
    char *out_dir_file;
    char **filenames;
    int filename_count = 0;

    filenames = _cr_get_filenames(disk, &filename_count);

    for (int i = 0; i < filename_count; i++) {
        out_filename = str_replace_all(filenames[i], '/', '.');
        out_dir_file = join_dir_file(dest, out_filename);
        _cr_unload_file(disk, filenames[i], out_dir_file);
        free(out_filename);
        free(out_dir_file);
        free(filenames[i]);
    }
    free(filenames);
    return 0;
}



char **_cr_get_filenames(unsigned disk, int *filename_count) {
    FILE *f;
    unsigned char* buffer = (unsigned char*)malloc(S_BLOCK);
    int offset = (disk - 1) * S_PARTITION;

    f = fopen(binPath, "rb");
    fseek(f, offset, SEEK_SET);
    fread(buffer, S_BLOCK, 1, f);

    // 29 + 1 = 30
    // one extra byte for null terminator
    char **filenames = calloc(S_BLOCK/S_DIR_ENTRY, sizeof(char*));
    *filename_count = 0;

    for (int i = 0; i < S_BLOCK; i += 32)
    {
        if (buffer[i] & 0x80)
        {
            filenames[*filename_count] = calloc(1, 30);
            memcpy(filenames[*filename_count], &buffer[i + 3], 29);
            (*filename_count)++;
        }
    }

    // for (int i = 0; i < *filename_count; i++) {
    //     printf("%s\n", filenames[i]);
    // }

    free(buffer);
    fclose(f);
    return filenames;
}

int _cr_load_file( unsigned disk, char *dest, char *src);

int _cr_load_file( unsigned disk, char *dest, char *src)
{
    crFILE *outfile;
    FILE *infile;
    const int BUFF_SIZE = 1000000;
    unsigned char *buffer = malloc(BUFF_SIZE);
    int nbytes;

    infile = fopen(src, "rb");
    outfile = cr_open(disk, dest, 'w');
    while((nbytes = fread(buffer, 1, BUFF_SIZE, infile)) > 0) {
        cr_write(outfile, buffer, nbytes);
    }
    cr_close(outfile);
    fclose(infile);
    free(buffer);
}

int cr_load(unsigned disk, char *orig)
{
    struct stat sb;
    DIR *d;
    struct dirent *dir;
    char *filename;
    
    stat(orig, &sb);
    if (S_ISREG(sb.st_mode)) {
        if (cr_exists(disk, orig)) {
            printf("[ERROR] File \"%s\" on Disk %d exists already.\n", orig, disk);
            return -1;
        } else {
            _cr_load_file(disk, orig, orig);
        }
    } else if (S_ISDIR(sb.st_mode)) {
        d = opendir(orig);
        while ((dir = readdir(d))) {
            filename = join_dir_file(orig, dir->d_name);
            stat(filename, &sb);
            if (S_ISREG(sb.st_mode)) {
                // printf("%s\n", filename);
                if (cr_exists(disk, dir->d_name)) {
                    printf("[ERROR] File \"%s\" on Disk %d exists already.\n", dir->d_name, disk);
                    free(filename);
                    continue;
                } else {
                    _cr_load_file(disk, dir->d_name, filename);
                }
            }
            free(filename);
        }
        closedir(d);
    } else {
        perror(orig);
        exit(EXIT_FAILURE);
    }
}
