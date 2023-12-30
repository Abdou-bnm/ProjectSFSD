#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "structs.h"

block* allocBlock(){
    for(int i = 0; i < 16; i++)
        if(MS[i].isUsed == 0){
            MS[i].isUsed = 1;
            return &MS[i];
        }
    
    return NULL; 
}

bool __recuSearch(unsigned short startIndex, unsigned short endIndex, char* key){
    if(startIndex == endIndex)
        if(Index[startIndex].isDeletedLogically)
            return false;
    
    unsigned short median = (startIndex + endIndex) / 2;
    int strcmpResult = strncmp(key, Index[median].key, KEY_MAX_SIZE);

    if(startIndex == endIndex && strcmpResult)
        return false;

    if(!strcmpResult)       return true;
    if(strcmpResult < 1)
        __recuSearch(startIndex, median, key);
    else
        __recuSearch(median + 1, endIndex, key);
}

bool searchElement(){
    // char *key = (char*)malloc(sizeof(KEY_MAX_SIZE + 1));
    // if(key == NULL){
    //     fprintf(stderr, "ERROR! [malloc in searchElement]: Couldn't allocate memory")
    //      exit(EXIT_FAILURE);
    // }
    // key = strncpy(key, buffer.tab, KEY_MAX_SIZE);
    if(indexSize == 0)                  return false;
    return __recuSearch(0, indexSize - 1, buffer.tab);
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

void DeleteElementLogique(){
    if(searchElement() == -1){
        return -1;
    }
    else
    {
        Index[searchElement()].isDeletedLogically = true;
    }
}

void ElementShift(char* NewElementPosition,char* StartCurrentElementPosition,char* EndCurrentElementPosition)
{
    //Shift the Element to New position
    while(StartCurrentElementPosition != EndCurrentElementPosition){
    {
        *NewElementPosition = *StartCurrentElementPosition;
        *StartCurrentElementPosition = "\0";
        NewElementPosition += sizeof(char);
        StartCurrentElementPosition += sizeof(char);
    }
    NewElementPosition += 2*sizeof(char);
}

int CalculateSpace(char *StartEspaceAddress, char *EndEspaceAddress){
    return EndEspaceAddress - StartEspaceAddress; 
}

void UpdateIndexDelete(int IndexElementDeleted){
    for (int ElementIndex = IndexElementDeleted; ElementIndex < indexSize ; ElementIndex++)
    {
        Index[ElementIndex].key = Index[ElementIndex+1].key;                          
        Index[ElementIndex].blockAddress = Index[ElementIndex+1].blockAddress;
        Index[ElementIndex].endAddress = Index[ElementIndex+1].endAddress;                
        Index[ElementIndex].isDeletedLogically = Index[ElementIndex+1].isDeletedLogically;
    }

    indexSize -- ;
}


int DeleteElementPhysique(file* file){
    if(searchElement() == -1){
        return -1;
    }
    else
    {
        int FreeSpace = 0;
        char* EndCurrentElementPosition,StartCurrentElementPosition;
        int indexElementDeleted = searchElement();
        char *NewElementPosition = Index[indexElementDeleted].key;
        block* blockAddressRecover = Index[indexElementDeleted].blockAddress;
        for(int i=indexElementDeleted + 1 ; i<indexSize ; i++)
        {
            if(Index[i].blockAddress <> Index[i-1].blockAddress) // To verify if the Shift Will be in the Same Block or not
            {
                FreeSpace = ((Index[i-1].blockAddress)->header).EndAddresse - NewElementPosition; // The Remaining Free Space in the Bloc
                if(CalculateSpace(Index[i].key,Index[i].endAddress) == FreeSpace)
                {   
                    EndCurrentElementPosition = Index[i].endAddress;
                    StartCurrentElementPosition = Index[i].key;
                    ElementShift(NewElementPosition,StartCurrentElementPosition,EndCurrentElementPosition);
                }
                else
                {
                    NewElementPosition = blockAddressRecover->tab;
                    EndCurrentElementPosition = Index[i].endAddress;
                    StartCurrentElementPosition = Index[i].key;
                    ElementShift(NewElementPosition,StartCurrentElementPosition,EndCurrentElementPosition);
                }             
            }
            else
            {
                EndCurrentElementPosition = Index[i].endAddress;
                StartCurrentElementPosition = Index.[i].key;
                ElementShift(NewElementPosition,StartCurrentElementPosition,EndCurrentElementPosition);
            }
            blockAddressRecover = Index[i].blockAddress;
        }
        UpdateIndexDelete(indexElementDeleted);
    }

}


int main(int argc, char const *argv[]){
    unsigned short answer;                                  // Used to get user's answers
    file file;
    memset(&file, 0, sizeof(file));
    
    printf("Do you want to create a new file?\n");
    printf("1- yes\n");
    printf("2- no\n");
    scanf("%hu", &answer);                                  // "%hu" format specifier for unsigned short
    if(answer == 1)
        createFile(&file);
    else
        goto end;

    do{
        memset(&buffer, sizeof(buffer), 0);                 // Clear the buffer for future uses.

        printf("Enter what you want to do in numbers\n");
        printf("1- Insert an element\n");
        printf("2- Delete an element\n");
        printf("3- Search for an element\n");
        printf("4- Display contents of file\n");

        scanf("%hu", &answer);
        switch (answer)
        {
        case 1:
            // Insert function
            break;
        
        case 2:
            // Delete function
            break;

        case 3:
            printf("Enter the key to search (Keys does NOT contain spaces) and a maximum size of %d: ", KEY_MAX_SIZE - 1);
            scanf("%16s", buffer.tab);
            if( searchElement())
                printf("Element exists.\n");
            else
                printf("Element doesn't exist.\n");
            
            break;

        case 4:
            // Print function
            break;

        default:
            fprintf(stderr, "Invalid choice, exiting...");
            exit(EXIT_FAILURE);
            break;
        }

        printf("Want to leave or do another thing?\n");
        printf("1- yes\n");
        printf("2- no\n");
        scanf("%hu", &answer);
    }while(answer == 1);

    end:
        printf("\nExiting...\n");
        return 0;
}