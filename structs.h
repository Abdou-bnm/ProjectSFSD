#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define KEY_MAX_SIZE 16
#define BLOCK_SIZE 50
#define INDEX_SIZE 36
#define MS_SIZE 16

// The declaration of the header of a file.
typedef struct fileHeader{
    unsigned int NbStructs;
    char name[36];
}fileHeader;

// The declaration of the header of a block.
typedef struct blockHeader{
    unsigned short NbStructs;  
    unsigned short usedSpace;
    char* StartAddress;
    char* EndAddress;
    char* StartFreeSpaceAddress;
}blockHeader;

// The declaration of a single block in memory, which is capable of containing multiple structs.
typedef struct block{
    blockHeader header;                     // The header of the block, containing useful/essential information.
    char tab[BLOCK_SIZE];                   // The actual memory of the block.
    bool isUsed;                            // A boolean that indicates the availability of the block, needed for insertion into a new block.
}block;

// The declaration of a single block in memory when used by a the file declared below.
typedef struct fBlock{
    block* data;                             // The actual block struct in memory.
    struct fBlock *next;                     // A pointer to the next element (block) in the linked list.
}fBlock;

// The declaration of the file, which will be a sorted linked list, with variable size and no overlap (chevauchement).
typedef struct File{
    fileHeader header;                      // The header of the file, containing useful/essential information.
    fBlock *head;                           // The head of the linked list containing all the data of the file.
}File;

// The declaration of an element of the "index" array, which contains information needed for search, insertion, ...etc.
typedef struct indexElement{        
    char *key;                              // Pointer to the first byte of the element.
    fBlock *blockAddress;                   // Pointer to the block of the element.
    char *endAddress;                       // Pointer to the last byte of the element.
    bool isDeletedLogically;                // Whether the element is deleted logically or not. An element which is deleted logically will be ignored.
}indexElement;                  

typedef struct IndexType{
    indexElement tab[INDEX_SIZE];           // An array of elements that are indexed under this key.
    unsigned short indexSize;               // The index of the first free element in the index.
}IndexType;

// Global variables
char buffer[BLOCK_SIZE];                    // A buffer to transfer data between RAM and Memory (used for file manipulation operations).
IndexType Index;                            // An index associated to the file containing useful information for various operations.
block MS[MS_SIZE];                          // The Memory which will contain all the blocks of the linked list and other blocks used by default.
File file;                                  // The actual file.