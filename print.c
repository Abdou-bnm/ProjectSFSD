#include "structs.h"
// Print Section

// Prints the data in decimal and below it the asscoiated character to that ASCII code, similair to hexdump, useful for debugging
void decimalPrint(char *src, int size)
{
    for (int i = 0; i < size; i++)
        printf("%d\t", src[i]);
    printf("\n");
    for (int i = 0; i < size; i++)
        printf("%c\t", src[i]);
    printf("\n\n");
}

// Prints a block, struct by struct
void __printBlock(block *block)
{
    int i = 0, nbStruct = 1;
    while (block->tab[i] != '\0')
    {
        printf("Struct number %d:\n", nbStruct);

        printf("Key: \"%s\".\n", &block->tab[i]);
        i += strlen(&block->tab[i]) + 1;

        printf("Data: \"%s\".\n\n", &block->tab[i]);
        i += strlen(&block->tab[i]) + 1;
        nbStruct++;
    }
}

// Prints the entire file, block by block
void printFile(file file)
{
    int nbFBlock = 1;
    while (file.head != NULL)
    {
        printf("fBlock number %d:\n", nbFBlock);
        __printBlock(file.head->data);
        printf("\n-------------------------------------------\n");
        file.head = file.head->next;
    }
}

// Prints the index's elements in an organized manner
void printIndex(){
    printf("Printing index of size IndexSize = %hu:\n\n", Index.IndexSize);
    for(int i = 0; i < Index.IndexSize; i++){
        printf("Element %d:\n", i + 1);
        printf("Block address: %p\n", Index.tab[i].blockAddress);
        printf("Key (first element) address: %p\n", Index.tab[i].key);
        printf("endAddress (last element): %p\n", Index.tab[i].endAddress);
        printf("Position in RFile: %lu\n", Index.tab[i].filePos);
        if(Index.tab[i].isDeletedLogically)
            printf("IsDeletedLogically: True\n\n");
        else
            printf("IsDeletedLogically: False\n\n");
    }
}
///-----------------------------------------------------------------------------