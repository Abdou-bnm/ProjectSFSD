#ifndef PRIMARY_HEADER_H
#define PRIMARY_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define KEY_MAX_SIZE 16
#define BUFFER_MAX_SIZE 50
#define INDEX_ELEMENT_MAX 36
#define MEMORY_BLOCK_MAX 16
#define FILE_NAME_MAX_SIZE 32

/// Part of Struct Declaration -----------------------------------------------------------------------------------

// The declaration of the header of a file.
typedef struct fileHeader{
    unsigned short nbBlocks;                // Number of blocks used by file
    unsigned short NbStructs;               // Number of structs in file
    char name[FILE_NAME_MAX_SIZE];          // Name of file, no path included
}fileHeader;

// The declaration of the header of a block.
typedef struct blockHeader{
    unsigned short NbStructs;               // The number of structs in the block
    unsigned short usedSpace;               // The number of used bytes in the block
    char* StartAddress;                     // The address of the 1st element in the block (block.tab)
    char* EndAddress;                       // The address of the last element in the block (block.tab[BUFFER_MAX_SIZE - 1])
    char* StartFreeSpaceAddress;            // The address of the 1st free element in the block
}blockHeader;

// The declaration of a single block in memory, which is capable of containing multiple structs.
typedef struct block{
    blockHeader header;                     // The header of the block, containing useful/essential information.
    char tab[BUFFER_MAX_SIZE];              // The actual memory of the block.
    bool isUsed;                            // A boolean that indicates the availability of the block, needed for insertion into a new block.
}block;

// The declaration of a single block in memory when used by a the file declared below.
typedef struct fBlock{
    block* data;                            // The actual block struct in memory.
    struct fBlock* next;                    // A pointer to the next element (block) in the linked list.
}fBlock;

// The declaration of the file, which will be a sorted linked list, with variable size and no overlap (chevauchement).
typedef struct file{
    fileHeader header;                      // The header of the file, containing useful/essential information.
    fBlock *head;                           // The head of the linked list containing all the data of the file.
    FILE* RFile;                            // The permanent .txt file that the data will be written to
    FILE* HFile;                            // File where we will store file and blocks headers, raw binary data
}file;

// The declaration of an element of the "index" array, which contains information needed for search, insertion, ...etc.
typedef struct indexElement{        
    char *key;                              // Pointer to the first byte of the element.
    fBlock *blockAddress;                   // Pointer to the block of the element.
    char *endAddress;                       // Pointer to the last byte of the element.
    bool isDeletedLogically;                // Whether the element is deleted logically or not. An element which is deleted logically will be ignored.
    unsigned long filePos;                  // Position of 1st character of key in RFile, providing that the 1st postion in RFile is 0. 
}indexElement;                  

typedef struct IndexType{
    indexElement tab[INDEX_ELEMENT_MAX]; 
    unsigned short IndexSize;  
}IndexType;

///------------------------------------------------------------------------------------------------------------------------

/// Part of function Declaration ------------------------------------------------------------------------------------------

block* allocBlock();
void createfile(file* file);
short __recuSearch(unsigned short startIndex, unsigned short endIndex, char* key);
short searchElement();
void ElementShift(char** NewElementPos,char* StartCurElementPos,char* EndCurElementPos);
int CalculateSpace(char *StartEspaceAddress, char *EndEspaceAddress);
void UpdateIndexDelete(int IndexElementDeleted);
void UpdateFileStruct(file* file);
void DeleteElementLogique(FILE* RFile);
int DeleteElementPhysique(file *file,FILE *Rfile);
void decimalPrint(char* src, int size);
void __printBlock(block* block);
void printFile(file file);
void printIndex();
void __setBlockAtStart(block* blck);
void fileOpen(file* file);
void StockHeaderecFile(FILE *Recfile, file *file);
void loadHeader(file *file);
void RFile_Delete(FILE *file, short elementIndexPos);
void updateIndexFPos(FILE* file);
void RFile_insert(FILE* file, char* element, unsigned long elementSize);
int insert_inIndex(file* file, unsigned short element_Size);
void update_fBlockInIndex(fBlock *fblck);
void update_fBlockHeader(fBlock* fblck);
void RecuInsert(file* file, fBlock* fblck, char* element, unsigned short elementSize, unsigned short shiftStart);
void insert(file* file, unsigned short element_Size);

/// Part of Variable Declaration ------------------------------------------------------------------------------------------

// Global variables
extern char buffer[BUFFER_MAX_SIZE];               // A buffer to transfer data between RAM and Memory (used for file manipulation operations).
extern block MS[MEMORY_BLOCK_MAX];                  // The Memory which will contain all the blocks of the linked list and other blocks used by default.
extern IndexType Index;                            // An index associated to the file containing useful information for various operations.
/// -----------------------------------------------------------------------------------------------------------------------

#endif