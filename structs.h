#define KEY_MAX_SIZE 16

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
    char tab[50];                           // The actual memory of the block.
    bool isUsed;                            // A boolean that indicates the availability of the block, needed for insertion into a new block.
}block;

// The declaration of a single block in memory when used by a the file declared below.
typedef struct fBlock{
    block* data;                             // The actual block struct in memory.
    block* next;                            // A pointer to the next element (block) in the linked list.
}fBlock;

// The declaration of the file, which will be a sorted linked list, with variable size and no overlap (chevauchement).
typedef struct file{
    fileHeader header;                      // The header of the file, containing useful/essential information.
    fBlock *cell;                           // The head of the linked list containing all the data of the file.
}file;

// The declaration of an element of the "index" array, which contains information needed for search, insertion, ...etc.
typedef struct indexElement{        
    char *key;                              // Pointer to the first byte of the element.
    block *blockAddress;                    // Pointer to the block of the element.
    char *endAddress;                       // Pointer to the last byte of the element.
    bool isDeletedLogically;                // Whether the element is deleted logically or not. An element which is deleted logically will be ignored.
}indexElement;                  

typedef struct IndexType{
    indexElement tab[36]; 
    unsigned short IndexSize;  
}IndexType;

// Global variables
char buffer[50];                            // A buffer to transfer data between RAM and Memory (used for file manipulation operations).
indexElement Index[36];                     // An index associated to the file containing useful information for various operations.
unsigned short indexSize = 0;               // The index of the first free element in the index.
block MS[16];                               // The Memory which will contain all the blocks of the linked list and other blocks used by default.

