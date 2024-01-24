#include "structs.h"
// insertion Section

// A function to insert element "element" into the permanent "RFile" in the correct position and changing the position info in Index
void RFile_insert(FILE *file, char *element, unsigned long elementSize)
{
    memset(buffer, 0, sizeof(buffer));    // Clear the current buffer
    memcpy(buffer, element, elementSize); // Copy the insered element into buffer

    short elementIndexPos = searchElement(), cpt = 0;
    if (elementIndexPos == -1)
    { // Error handling
        fprintf(stderr, "ERROR! [searchElement in RFile_insert]: Couldn't find element in index.\nreturning...\n");
        return;
    }

    if (fseek(file, 0, SEEK_END))
    { // Error handling
        fprintf(stderr, "ERROR! [fseek 1 in RFile_insert]: return a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    long maxPos = ftell(file), space, err; // Last position in file
    if (maxPos == -1)
    { // Error handling
        fprintf(stderr, "ERROR! [ftell in RFile_insert]: returned -1.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    if (elementIndexPos == Index.IndexSize - 1)
    { // In case the element is the last in index, so no shifting needed
        Index.tab[elementIndexPos].filePos = maxPos;
        for (int i = 0; i < elementSize; i++)
        {
            if (buffer[i] == '\0')
            { // Add ':' and '\n' instead of '\0' to organise RFile
                if (cpt % 2)
                    fputc('\n', file);
                else
                    fputc(':', file);
                cpt++;
                continue;
            }
            fputc(buffer[i], file); // Copy contents of buffer to file
        }
        return;
    }

    // In case Not the last element in index, so there is shifting needed
    space = maxPos - Index.tab[elementIndexPos + 1].filePos; // Number of bytes that will be shifted

    char *tmp = (char *)malloc(space); // Temporary buffer to hold the shifted string
    if (tmp == NULL)
    { // Error handling
        fprintf(stderr, "ERROR! [malloc in RFile_insert]: Couldn't allocate memory for \"tmp\" variable.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    memset(tmp, 0, space); // Setting the entire tmp buffer to 0

    if (fseek(file, Index.tab[elementIndexPos + 1].filePos, SEEK_SET))
    { // Error handling
        fprintf(stderr, "ERROR! [fseek 2 in RFile_insert]: returned a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    err = fread(tmp, sizeof(char), space, file); // Read the shifted elements into tmp buffer
    if (err != space)
    { // Error handling
        fprintf(stderr, "ERROR! [fread in RFile_insert]: expected to read %li but got %li", space, err);
        exit(EXIT_FAILURE);
    }

    Index.tab[elementIndexPos].filePos = Index.tab[elementIndexPos + 1].filePos; // Changing the filePos in the Index
    Index.tab[elementIndexPos + 1].filePos += elementSize;                       // Changing the filePos of the next element in the Index

    if (fseek(file, Index.tab[elementIndexPos].filePos, SEEK_SET))
    { // Error handling
        fprintf(stderr, "ERROR! [fseek 3 in RFile_insert]: returned a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < elementSize; i++)
    { // Add ':' and '\n' instead of '\0' to organise RFile
        if (buffer[i] == '\0')
        {
            if (cpt % 2)
                fputc('\n', file);
            else
                fputc(':', file);
            cpt++;
            continue;
        }
        fputc(buffer[i], file); // Writing the new element
    }

    err = fwrite(tmp, sizeof(char), space, file); // Writing the rest of the old elements (shifted ones)
    if (err != space)
    { // Error handling
        fprintf(stderr, "ERROR! [fwrite 2 in RFile_insert]: expected to read %li but got %li", space, err);
        exit(EXIT_FAILURE);
    }

    free(tmp); // Free the tmp buffer
}
int insert_inIndex(file* file, unsigned short elementSize){
    unsigned short i = 0;
    int cmpResult, j = Index.IndexSize;

    if(j == INDEX_ELEMENT_MAX){
        fprintf(stderr, "ERROR! [insert_inIndex]: Index full, cannot insert another element, Returning...\n");
        return -1;
    }

    for(i; i < Index.IndexSize; i++){
        cmpResult = strncmp(buffer, Index.tab[i].key, KEY_MAX_SIZE);
        if(!cmpResult){
            fprintf(stderr, "ERROR! [strncmp in insert_inIndex]: returned -1; key already exists in index.\nReturning...\n");
            return -1;
        }
        if(cmpResult < 0)
            break;
    }

    for(j; j > i; j--)
        Index.tab[j] = Index.tab[j - 1];


    memset(&Index.tab[i], 0, sizeof(indexElement));
    Index.tab[i].isDeletedLogically = false;

    // In case we need to insert into the start of the file, so no Index.tab[i - 1] to use
    if(i == 0){
        Index.tab[i].blockAddress = file->head;
        Index.tab[i].key = file->head->data->tab;
        goto endFunc;
    }
    
    // In case there is enough space in the current fBlock (the previous element's fBlock)
    unsigned short freeSpace = BUFFER_MAX_SIZE - Index.tab[i - 1].blockAddress->data->header.usedSpace;
    if(freeSpace >= elementSize || Index.tab[i - 1].blockAddress == Index.tab[i + 1].blockAddress){
        Index.tab[i].blockAddress = Index.tab[i - 1].blockAddress;
        Index.tab[i].key = Index.tab[i - 1].endAddress + 1;
        goto endFunc;
    }

    // WILL TRY TO IMPLEMENT IT IN THE PREVIOUS CASE, BECAUSE THEY SHARE EXACTLY THE SAME CODE
    // // In case we are inserting into the middle of an already full block (in the previous case, we had enough space. Now we don't)
    // if(Index.tab[i - 1].blockAddress == Index.tab[i + 1].blockAddress){
    //     Index.tab[i].blockAddress = Index.tab[i - 1].blockAddress;
    //     Index.tab[i].key = Index.tab[i - 1].endAddress + 1;
    // }

    // In case we need to put it at the start of the next fBlock and it is already set up (exists already)
    if(Index.tab[i + 1].blockAddress != NULL){
        Index.tab[i].blockAddress = Index.tab[i + 1].blockAddress;
        Index.tab[i].key = Index.tab[i + 1].blockAddress->data->tab;
        goto endFunc;
    }
    
    // The final is we need to put the new element at the start of the next fBlock  and it is not set up at all (Index.tab[i + 1].blockAddress == NULL)
    Index.tab[i].blockAddress = (fBlock*)malloc(sizeof(fBlock));
    if(Index.tab[i].blockAddress == NULL){
        fprintf(stderr, "ERROR! [malloc in recuInsert]: Couldn't allocate space for next element's fBlock.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    
    Index.tab[i].blockAddress->data = allocBlock();
    if(Index.tab[i].blockAddress->data == NULL){
        fprintf(stderr, "ERROR! [allocBlock in recuInsert]: Couldn't find a block in MS.\nReturning...\n");
        return -1;
    }
    
    // Set the new element's starting adresse to the first adresse of the new fBlock
    Index.tab[i].key = Index.tab[i].blockAddress->data->tab;                
    Index.tab[i - 1].blockAddress->next = Index.tab[i].blockAddress;        // Links the new fBlock to the previous fBlock
    __setBlockAtStart(Index.tab[i].blockAddress->data);
    file->header.nbBlocks++;

    endFunc:
        file->header.NbStructs++;
        Index.IndexSize++;
        return i;
}

// this is the old one
void update_fBlockInIndex(fBlock *fblck){
    unsigned int i = 0, j = 0;
    while(i < Index.IndexSize && Index.tab[i].blockAddress != fblck)
        i++;

    // if any error handling in this function ever happened, then we are in big trouble
    if(i == Index.IndexSize){
        fprintf(stderr, "ERROR! [updateInIndex]: Couldn't find any element with fBlock adresse for some reason.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    while(Index.tab[i].blockAddress == fblck){
        Index.tab[i].key = &fblck->data->tab[j];

        // increment until end of key
        while(j < BUFFER_MAX_SIZE && fblck->data->tab[j] != '\0')
            j++;

        if(j == BUFFER_MAX_SIZE){
            fprintf(stderr, "ERROR! [updateInIndex]: reached end-of-block before expected end of key.\nExiting...\n");
            exit(EXIT_FAILURE);
        }

        j++;
        // this time until end of struct
        while(j < BUFFER_MAX_SIZE && fblck->data->tab[j] != '\0')
            j++;

        if(j == BUFFER_MAX_SIZE){
            fprintf(stderr, "ERROR! [updateInIndex]: reached end-of-block before expected end of element.\nExiting...\n");
            exit(EXIT_FAILURE);
        }
        Index.tab[i].endAddress = &fblck->data->tab[j];

        j++;
        i++;
    }
}

void update_fBlockHeader(fBlock* fblck){
    unsigned int i = 0, j;
    while(i < Index.IndexSize && Index.tab[i].blockAddress != fblck)
        i++;

    fblck->data->header.NbStructs = 0;
    // if any error handling in this function ever happened, then we are in big trouble
    if(i == Index.IndexSize){
        fprintf(stderr, "ERROR! [update_fBlockHeader]: Couldn't find any element with fBlock adresse for some reason.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    // Advancing into the last element belonging to this fBlock in the index
    while(Index.tab[i].blockAddress == fblck){
        fblck->data->header.NbStructs++;
        i++;
    }

    i--;

    j = (unsigned long) Index.tab[i].endAddress - (unsigned long) &fblck->data->tab[0];

    if(j >= BUFFER_MAX_SIZE || j <= 0){
        fprintf(stderr, "ERROR! [update_fBlockHeader]: last character of last element in index in this fBlock - adresse of tab[0]  \
        >= BUFFER_MAX_SIZE OR negative-or-null, WTF!\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    fblck->data->header.EndAddress = &fblck->data->tab[BUFFER_MAX_SIZE - 1];
    fblck->data->header.StartAddress = &fblck->data->tab[0];
    fblck->data->header.StartFreeSpaceAddress = Index.tab[i].endAddress + 1;
    fblck->data->header.usedSpace = j + 1;
}

void RecuInsert(file* file, fBlock* fblck, char* element, unsigned short elementSize, unsigned short shiftStart){
    if(BUFFER_MAX_SIZE - fblck->data->header.usedSpace >= elementSize){
        for(int i = fblck->data->header.usedSpace; i >= shiftStart; i--)
            fblck->data->tab[i + elementSize] = fblck->data->tab[i];

        memcpy(&fblck->data->tab[shiftStart], element, elementSize);

        goto endOfFunction;
    }

    // In case the next fBlock isn't already prepared
    if(fblck->next == NULL){
        fblck->next = (fBlock*)malloc(sizeof(fBlock));
        if(fblck->next == NULL){
            fprintf(stderr, "ERROR! [malloc in recuInsert]: Couldn't allocate space for fblck->next.\nExiting...\n");
            exit(EXIT_FAILURE);
        }

        fblck->next->data = allocBlock();
        if(fblck->next->data == NULL){
            fprintf(stderr, "ERROR! [allocBlock in recuInsert]: Couldn't find a block in MS.\nReturning...\n");
            goto endOfFunction;
        }

        file->header.nbBlocks++;
        __setBlockAtStart(fblck->next->data);
    }

    // go to the 1st element belonging to this fBlock in index
    unsigned short j;
    for(j = 0; j < Index.IndexSize; j++)
        if(Index.tab[j].blockAddress == fblck)
            break;

    // continue to the last element in index belonging to this fBlock
    while (Index.tab[j].blockAddress == fblck)
        j++;

    j--;

    unsigned short lastElement = j;
    Index.tab[lastElement].blockAddress = fblck->next;


    while(elementSize > (&fblck->data->tab[BUFFER_MAX_SIZE - 1] - Index.tab[j].key)){
        Index.tab[j].blockAddress = fblck->next;
        j--;
    }

    //calculated it using paper
    unsigned long nextElementSize = (unsigned long) Index.tab[lastElement].endAddress - (unsigned long) Index.tab[j].key + 1;

    char* nextElement = (char*)malloc(nextElementSize);
    int i;

    memcpy(nextElement, Index.tab[j].key , nextElementSize);

    // there is no shifting to be done, the new element will occupy the entire block
    if(Index.tab[j].key == fblck->data->tab)
        goto nextFunctionCall;

    else
        // the "Index.tab[j - 1].endAddress" can be replaced by "Index.tab[j].key - 1" if something is wrong in Index
        i = Index.tab[j - 1].endAddress - fblck->data->tab;

    for(i; i >= shiftStart; i--)            // Start shifting from that character into the element of shiftStart
        fblck->data->tab[i + elementSize] = fblck->data->tab[i];

    // copy the new element into tab starting from shiftStart
    memcpy(&fblck->data->tab[shiftStart], element, elementSize);

    update_fBlockInIndex(fblck);

    nextFunctionCall:
        RecuInsert(file, fblck->next, nextElement, nextElementSize, 0);

    endOfFunction:
        free(element);
        update_fBlockInIndex(fblck);
        update_fBlockHeader(fblck);
        return;
}

void insert(file* file, unsigned short element_Size){
    int elementNumber = insert_inIndex(file, element_Size);
    if(elementNumber == -1)
        return;

    // Creating 2 temporary strings, 1 RecuInsert (it will be freed) and the 2nd for RFile_insert (buffer will be set to 0)
    char* tmpElement1 = (char*)malloc(element_Size + 1);
    if(tmpElement1 == NULL){
        fprintf(stderr, "ERROR! [malloc 1 in insert]: Couldn't allocate space for tmpElement1.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    memset(tmpElement1, 0, element_Size + 1);
    memcpy(tmpElement1, buffer, element_Size);

    char* tmpElement2 = (char*)malloc(element_Size + 1);
    if(tmpElement2 == NULL){
        fprintf(stderr, "ERROR! [malloc 2 in insert]: Couldn't allocate space for tmpElement2.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    memset(tmpElement2, 0, element_Size + 1);
    memcpy(tmpElement2, buffer, element_Size);

    fBlock* element_fBlck = Index.tab[elementNumber].blockAddress;

    unsigned long element_IndiceInBlock;

    element_IndiceInBlock = (unsigned long) Index.tab[elementNumber].key - (unsigned long) element_fBlck->data->tab;

    RecuInsert(file, element_fBlck, tmpElement1, element_Size, element_IndiceInBlock);

    RFile_insert(file->RFile, tmpElement2, element_Size);

    printf("insert function completed executing successfully.\n");
}
