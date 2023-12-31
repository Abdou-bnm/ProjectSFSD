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
        if(Index->tab[startIndex].isDeletedLogically)
            return false;
    
    unsigned short median = (startIndex + endIndex) / 2;
    int strcmpResult = strncmp(key, Index->tab[median].key, KEY_MAX_SIZE);

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
    // key = strncpy(key, buffer, KEY_MAX_SIZE);
    if(Index->indexSize == 0)                  return false;
    return __recuSearch(0, Index->indexSize - 1, buffer);
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


void ElementShift(char* NewElementPos,char* StartCurElementPos,char* EndCurElementPos)
{
    //Shift the Element to New Pos
    while(StartCurElementPos != EndCurElementPos)
    {
        *NewElementPos = *StartCurElementPos;
        *StartCurElementPos = 0;
        NewElementPos += sizeof(char);
        StartCurElementPos += sizeof(char);
    }
    NewElementPos += 2*sizeof(char);
}

int CalculateSpace(char *StartEspaceAddress, char *EndEspaceAddress){
    return EndEspaceAddress - StartEspaceAddress; 
}

// Function to update index array (Delete Element)
void UpdateIndexDelete(int IndexElementDeleted){
    for (int ElementIndex = IndexElementDeleted; ElementIndex < Index->indexSize ; ElementIndex++)
    {
        Index->tab[ElementIndex].key = Index->tab[ElementIndex+1].key;                          
        Index->tab[ElementIndex].blockAddress = Index->tab[ElementIndex+1].blockAddress;
        Index->tab[ElementIndex].endAddress = Index->tab[ElementIndex+1].endAddress;                
        Index->tab[ElementIndex].isDeletedLogically = Index->tab[ElementIndex+1].isDeletedLogically;
    }

    Index->indexSize -- ;
}

// Function to update File
void UpdateFileStruct(file* file)
{
    fBlock *ftmp = (*file).head ; 
    while((ftmp)->next != NULL)
    {
        ftmp = ftmp->next;
    }
}

// Function to delete an element from the file (Logical)
void DeleteElementLogique(){
    short indexElement = searchElement();
    if(indexElement == -1){
        printf("\nERROR! [Searching for Element]:already deleted or not Existe ");
        return;
    }
    else
    {
        Index->tab[indexElement].isDeletedLogically = true;
    }
}

// Function to delete an element from the file (Physique)
int DeleteElementPhysique(file* file){
    
    // Search for the index of the element to be deleted
    short indexElementDeleted = searchElement();
 
    // Check if the element is not found or already deleted
    if(indexElementDeleted == -1){
        printf("\nERROR! [Searching for Element]:already deleted or not Existe");
        return -1;
    }
    else
    {
        unsigned short NbElement=-1;
        int FreeSpace = 0;
        char* EndCurElementPos,StartCurElementPos;
        char *NewElementPos = Index->tab[indexElementDeleted].key;
        block* blockAddressRecover = Index->tab[indexElementDeleted].blockAddress;
        for(int i=indexElementDeleted + 1 ; i<Index->indexSize ; i++)
        {
            // Verify if the Shift Will be in the Same Block or not
            if(Index->tab[i].blockAddress != Index->tab[i-1].blockAddress) // Shift the element into a different block
            {
                FreeSpace = ((Index->tab[i-1].blockAddress)->header).EndAddress - NewElementPos; // Calculate the remaining free space in the block
                // Verify if FreeSpace is sufficient for the Element
                if(CalculateSpace(Index->tab[i].key,Index->tab[i].endAddress) <= FreeSpace) // FreeSpace is sufficient => Make the Element in the FreeSpace
                {   
                    EndCurElementPos = Index->tab[i].endAddress;
                    StartCurElementPos = Index->tab[i].key;
                    ElementShift(NewElementPos,StartCurElementPos,EndCurElementPos);
                    NbElement++;
                }
                else // FreeSpace isn't sufficient => Make the Element in New Block
                {
                    ((Index->tab[i-1].blockAddress)->header).NbStructs += NbElement; //Update Number of Element in Current Block
                    ((Index->tab[i].blockAddress)->header).NbStructs -= NbElement+1; //Update Number of Element in New Block
                    NbElement= 0; 
                    NewElementPos = blockAddressRecover->tab;
                    EndCurElementPos = Index->tab[i].endAddress;
                    StartCurElementPos = Index->tab[i].key;
                    ElementShift(NewElementPos,StartCurElementPos,EndCurElementPos);
                }             
            }
            else // Shift the element within the same block
            {
                EndCurElementPos = Index->tab[i].endAddress;
                StartCurElementPos = Index->tab[i].key;
                ElementShift(NewElementPos,StartCurElementPos,EndCurElementPos);
            }
            // Update the block address for the next iteration
            blockAddressRecover = Index->tab[i].blockAddress;
        }

        // Update IndexArray to reflect the deletion
        UpdateIndexDelete(indexElementDeleted);

        // Update Last Block
        if (((Index->tab[Index->indexSize].blockAddress)->header.NbStructs) == 0)
        {
            (Index->tab[Index->indexSize].blockAddress)->isUsed = false;
        }

        // Update nb Element in file header
        file->header.NbStructs--;

        printf("\nElement Deleted!");
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
        memset(buffer, sizeof(buffer), 0);                 // Clear the buffer for future uses.

        printf("\n\nEnter what you want to do in numbers\n");
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
            scanf("%16s", buffer);
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