#pragma once
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

// Global variables
extern char* binPath;

// Structs
typedef struct {
    char filename[29];
    unsigned int blockNumber;
    unsigned int diskNumber;
    unsigned int currentBlockToRead;
    unsigned int lastByteRead;
} crFILE;

// General functions

/**
 * @brief Stablishes the path to the disk file as a global variable 
 * 
 * @param diskname Disk file path
 */
void cr_mount(char *diskname);

/**
 * @brief Prints the bitmap corresponding to disk. If disk is 0 prints the entire bitmap, 
 * ocuppied chunks and free chunks
 * 
 * @param disk Disk number to be printed
 * @param hex false to print as binary. true to print as hexadecimal.
 */
void cr_bitmap(unsigned int disk, bool hex);

/**
 * @brief Checks if the file exists inside the partition
 * 
 * @param disk Partition number
 * @param filename Name of the file to check
 * @return int 1 if found. 0 if not found.
 */
int cr_exists(unsigned int disk, char *filename);

/**
 * @brief Lists all elements inside a partition
 * 
 * @param disk Partition number
 */
void cr_ls(unsigned int disk);

// File management functions

/**
 * @brief Opens a file
 * 
 * @param disk Partition number
 * @param filename Path to the file
 * @param mode 'r' for reading. 'w' for writing
 * @return crFILE* Pointer to a file descriptor
 */
crFILE *cr_open(unsigned int disk, char *filename, char mode);

/**
 * @brief Reads bytes from a file
 * 
 * @param file_desc File descriptor from where to read
 * @param buffer Pointer to write read bytes
 * @param nbytes Amount of bytes to read
 * @return int Amount of bytes read
 */
int cr_read(crFILE *file_desc, void *buffer, int nbytes);

/**
 * @brief Write bytes to a file
 * 
 * @param file_desc File descriptor from where to write
 * @param buffer Pointer of bytes to write
 * @param nbytes Amount of bytes to write from buffer
 * @return int Amount of bytes written
 */
int cr_write(crFILE *file_desc, void *buffer, int nytes);

/**
 * @brief Closes a file and saves it's changes
 * 
 * @param file_desc File descriptor
 * @return int 
 */
int cr_close(crFILE *file_desc);

/**
 * @brief Removes file from disk
 * 
 * @param disk Partition where the file is located
 * @param filename Path to the file
 * @return int 
 */
int cr_rm(unsigned int disk, char *filename);

/**
 * @brief Creates a hardlink for a file
 * 
 * @param disk Partiton where the file is located
 * @param orig Path to the file to be linked
 * @param dest Path to the new file reference
 * @return int 
 */
int cr_hardlink(unsigned int disk, char *orig, char *dest);

/**
 * @brief Creates a softlink of a file
 * 
 * @param disk_orig Partition of the file
 * @param disk_dest Partition of the new reference
 * @param orig Path to the file
 * @param dest Path to the new reference
 * @return int 
 */
int cr_softlink(unsigned int disk_orig, unsigned int disk_dest, char *orig, char *dest);

/**
 * @brief Copies a file to the computer
 * 
 * @param disk Partition where the file is located
 * @param orig Path to the file
 * @param dest Path to file to be unloaded
 * @return int 
 */
int cr_unload(unsigned disk, char *orig, char *dest);

/**
 * @brief Loads a file into the file system
 * 
 * @param disk Partition where to load the file
 * @param orig Path to the file to be loaded
 * @return int 
 */
int cr_load(unsigned disk, char *orig);
