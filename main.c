#include <stdio.h>
#include <stdlib.h>

// The declaration of the header of a file
typedef struct fileHeader{
    unsigned short NbStructs;
    char name[50];
}fileHeader;

// The declaration of the header of a block
typedef struct blockHeader{
    unsigned short NbStructs;  
    unsigned short remainingSpace;
}blockHeader;

// The declaration of a single block, which contains multiple structs
typedef struct block{
    blockHeader header;
    char tab[100];
    block *next;
}block;

// The declaration of the file, which will be a sorted linked list, with variable size and no overlap (chevauchement), "head" 
// will be the start of the linked list.
typedef struct fichier{
    fileHeader header;
    block* head;
}fichier;

// The declaration of an element of the "index" array, a pointer to the key. and 2 pointers to the block and the end of the 
// struct of the key
typedef struct IndexElement{        
    char *key;
    block *blockAddress;
    char *endAddress;
}IndexElement;                  

// Global variables
block *buffer;                      // A buffer to transfer data between RAM and Memory (used for file manipulation operations)
IndexElement index[50];             // An index associated to the file containing useful information for various operations