#include <stdbool.h>
#include "cr_API.h"

typedef struct
{
} crFILE;

// General functions

void cr_mount(char *diskname);

void cr_bitmap(unsigned int disk, bool hex);

int cr_exists(unsigned int disk, char *filename);

void cr_ls(unsigned int disk);

// File management functions

crFILE *cr_open(unsigned int disk, char *filename, char mode);

int cr_read(crFILE *file_desc, void *buffer, int nbytes);

int cr_write(crFILE *file_desc, void *buffer, int nytes);

int cr_close(crFILE *file_desc);

int cr_rm(unsigned int disk, char *filename);

int cr_hardlink(unsigned int disk, char *orig, char *dest);

int cr_softlink(unsigned int disk_orig, unsigned int disk_dest, char *orig, char *dest);

int cr_unload(unsigned disk, char *orig, char *dest);

int cr_load(unsigned disk, char *orig);
