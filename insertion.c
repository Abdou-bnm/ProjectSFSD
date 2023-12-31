#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "structs.h"
file File;
block* allocBlock(){
    for(int i = 0; i < 16; i++)
        if(MS[i].isUsed == 0){
            MS[i].isUsed = 1;
            return &MS[i];
        }
    
    return NULL; 
}

void createFile(file* file){
    file->head = (fBlock*)malloc(sizeof(fBlock));
    file->head->data = allocBlock();
    if(file->head->data == NULL){
        fprintf(stderr, "\nERROR! [allocBlock in createFile]: No space to allocate block.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    file->head->next = NULL;
    file->header.NbStructs = 0;
    printf("Enter the name of the file (max size is 35 characters, spaces NOT allowed): ");
    scanf("%36s", file->header.name);
}

block insertion(){
        //  will consider the character of end of key as " "
char tab[KEY_MAX_SIZE];
int i=0;
while((char c != " ") && (i < KEY_MAX_SIZE))
    tab[i]=buffer.tab[i];
    i++;
//index
if(Index.IndexSize==0)
    Index.tab->key=File.head
    Index.IndexSize=1;
else 
    int j=0;
    while(j<Index.IndexSize && strcmp(Index.tab[j],tab)<0) 
        j++
    if(j>=Index.IndexSize)
        strcpy(Index.tab[j],tab);
        Index.IndexSize++;
    else 
        for(int k=j;k<Index.IndexSize;k++)
           strcpy(Index.tab[k+1],Index.tab[k]);
        strcpy(Index.tab[j],tab);
        Index.IndexSize++;
        


}

