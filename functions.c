#include "structs.h"

/// Part of Variable Declaration ------------------------------------------------------------------------------------------

// Global variables
char buffer[BUFFER_MAX_SIZE]; // A buffer to transfer data between RAM and Memory (used for file manipulation operations).
block MS[MEMORY_BLOCK_MAX];    // The Memory which will contain all the blocks of the linked list and other blocks used by default.
IndexType Index;              // An index associated to the file containing useful information for various operations.
/// -----------------------------------------------------------------------------------------------------------------------

block *allocBlock(){
    for (int i = 0; i < 16; i++)
        if (MS[i].isUsed == 0)
        {
            MS[i].isUsed = 1;
            return &MS[i];
        }

    return NULL;
}

// Update the filePos in indexElement struct of each element of index using the RFile.
void updateIndexFPos(FILE* file){
    if(fseek(file, 0, SEEK_SET)){
        fprintf(stderr, "ERROR! [fseek in file]: returned a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    short curPos;
    char c;

    for(int i = 0; i < Index.IndexSize; i++){
        if(Index.tab[i].isDeletedLogically)         // if element is deleted logically, it won't be in the RFile
            continue;

        curPos = ftell(file);                       // Get the current position in RFile, which will be Index.tab[i].filePos
        if(curPos == -1){                           // Error Handling
            fprintf(stderr, "ERROR! [ftell in file]: returned -1.\nExiting...\n");
            exit(EXIT_FAILURE);
        }

        Index.tab[i].filePos = curPos;
        while(c != '\n')                            // Avancing until the end of the current element
            c = fgetc(file);
        
        fgetc(file);                                // One more time to set it to the start of the next element
    }
}

void loadHeader(file *file)
{
    char *fileName = (char *)malloc(FILE_NAME_MAX_SIZE + 9);
    if (fileName == NULL)
    {
        fprintf(stderr, "ERROR! [malloc 1 in loadHeader]: Couldn't allocate memory for fileName.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    memset(fileName, 0, FILE_NAME_MAX_SIZE + 9);
    sprintf(fileName, ".%s.header", file->header.name);

    file->HFile = fopen(fileName, "rb");
    if (file->HFile == NULL)
    {
        fprintf(stderr, "ERROR! [fopen in leadHeader]: Couldn't open header file \"%s\".\nExiting...\n", fileName);
        exit(EXIT_FAILURE);
    }

    free(fileName);
    fread(&(file->header), sizeof(fileHeader), 1, file->HFile);

    file->head = (fBlock*)malloc(sizeof(fBlock));
    if(file->head == NULL){
        fprintf(stderr, "ERROR! [malloc 2 in loadHeader]: Couldn't allocate memory for fblck.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    file->head->data = NULL;
    file->head->next = NULL;

    fBlock *curElem = file->head;
    fBlock *fblck = file->head->next;

    for(int i = 1; i < file->header.nbBlocks; i++){
        fblck = (fBlock*)malloc(sizeof(fBlock));
        if(fblck == NULL){
            fprintf(stderr, "ERROR! [malloc 2 in loadHeader]: Couldn't allocate memory for fblck.\nExiting...\n");
            exit(EXIT_FAILURE);
        }

        fblck->data = NULL;
        fblck->next = NULL;

        curElem->next = fblck;
        curElem = curElem->next;
        fblck = fblck->next;
    }

    if (fclose(file->HFile))
        fprintf(stderr, "ERROR! [fclose in loadHeader]: returned a non-zero value.\nContinuing...\n");
}

void __setBlockAtStart(block *blck)
{ // Sets the block values when first allocated.
    blck->header.EndAddress = &blck->tab[BUFFER_MAX_SIZE - 1];
    blck->header.StartAddress = blck->tab;
    blck->header.StartFreeSpaceAddress = blck->tab;
    blck->header.NbStructs = 0;
    blck->header.usedSpace = 0;
    memset(blck->tab, 0, BUFFER_MAX_SIZE);
}

void fileOpen(file *file)
{
    loadHeader(file);

    file->RFile = fopen(file->header.name, "r+");
    if (file->RFile == NULL){
        fprintf(stderr, "ERROR! [fopen in fileOpen]: Couldn't open data file \"%s\".\nExiting...\n", file->header.name);
        exit(EXIT_FAILURE);
    }

    Index.IndexSize = 0;
    char c;
    short size, curPos;                // j will be used to track elements in Index
    fBlock *fblck = file->head;

    while (fblck != NULL)
    {
        if(Index.IndexSize > INDEX_ELEMENT_MAX){
            fprintf(stderr, "ERROR! [in fileOpen]: unexpcted Index.IndexSize > INDEX_ELEMENT_MAX before end of file.\nExiting...\n");
            exit(EXIT_FAILURE);
        }

        if (fblck->data == NULL){   // In case the current block is not already set-up, or else we use the current block
            fblck->data = allocBlock();
            if (fblck->data == NULL)
            {
                fprintf(stderr, "ERROR! [allocBlock in fileOpen]: Couldn't allocate block to use.\nReturning...\n");
                return;
            }
            __setBlockAtStart(fblck->data);
        }

        size = 0;
        do
        {
            c = fgetc(file->RFile);
            if (c == EOF)
            {
                fprintf(stderr, "ERROR! [fgetc in fileOpen]: EOF reached before an expected end of struct '\\n'.\nReturning...\n");
                return;
            }
            size++;
        } while (c != '\n');
        
        if(fseek(file->RFile, -1 * size, SEEK_CUR)){
            fprintf(stderr, "ERROR! [fseek 1 in fileOpen]: returned a non-zero value.\nExiting...\n");
            exit(EXIT_FAILURE);
        }

        if (size > BUFFER_MAX_SIZE - fblck->data->header.usedSpace){
            fblck = fblck->next;
            continue; // Go to the next iteration
        }
        
        curPos = ftell(file->RFile);
        if(curPos == -1){
            fprintf(stderr, "ERROR! [ftell in fileOpen]: returned -1.\nExiting...\n");
            exit(EXIT_FAILURE);
        }

        // Updating the element in the Index
        Index.tab[Index.IndexSize].filePos = curPos;    
        Index.tab[Index.IndexSize].key = fblck->data->header.StartFreeSpaceAddress;
        Index.tab[Index.IndexSize].endAddress = fblck->data->header.StartFreeSpaceAddress + size - 1;
        Index.tab[Index.IndexSize].isDeletedLogically = false;
        Index.tab[Index.IndexSize].blockAddress = fblck;
        Index.IndexSize++;
        
        // Reading the actual element into its respective block
        for (int i = 0; i < size; i++){
            c = fgetc(file->RFile);

            switch (c){
                case ':':
                    fblck->data->header.StartFreeSpaceAddress[i] = '\0';
                    break;

                case '\n':
                    fblck->data->header.StartFreeSpaceAddress[i] = '\0';
                    break;

                default:
                    fblck->data->header.StartFreeSpaceAddress[i] = c;
                    break;
                }
        }

        // Updating the block's header info
        fblck->data->header.StartFreeSpaceAddress += size;
        fblck->data->header.NbStructs++;
        fblck->data->header.usedSpace += size;

        // In case of some padding
        while (c == '\n' || c == '\0')
            c = fgetc(file->RFile);
        
        if(fseek(file->RFile, -1, SEEK_CUR)){
            fprintf(stderr, "ERROR! [fseek 2 in fileOpen]: returned a non-zero value.\nExiting...\n");
            exit(EXIT_FAILURE);
        }
        
        // In case of reaching EOF, which means we completed the entire RFile
        if (c == EOF){
            return;
        }
    } // Else, we go into another iteration
}

void createfile(file *file)
{
    file->head = (fBlock *)malloc(sizeof(fBlock));
    file->head->data = allocBlock();
    if (file->head->data == NULL)
    {
        fprintf(stderr, "\nERROR! [allocBlock in createfile]: No space to allocate block.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    file->head->next = NULL;
    file->header.NbStructs = 0;
    file->header.nbBlocks = 1;
    printf("Enter the name of the file (max size is %d characters, spaces NOT allowed, CASE SENSITIVE): ", FILE_NAME_MAX_SIZE - 1);
    scanf("%36s", file->header.name);

    file->RFile = fopen(file->header.name, "w+");
    if (file->RFile == NULL)
    {
        fprintf(stderr, "ERROR! [fopen in createfile]: couldn't create permanent file.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
}

/// Search Section
short __recuSearch(unsigned short startIndex, unsigned short endIndex, char *key)
{
    if (startIndex == endIndex)
        if (Index.tab[startIndex].isDeletedLogically)
            return -1;

    unsigned short median = (startIndex + endIndex) / 2;
    int strcmpResult = strncmp(key, Index.tab[median].key, KEY_MAX_SIZE);

    if (startIndex == endIndex && strcmpResult)
        return -1;

    if (!strcmpResult)
        return median;
    if (strcmpResult < 1)
        return __recuSearch(startIndex, median, key);
    else
        return __recuSearch(median + 1, endIndex, key);
}

short searchElement()
{
    if (Index.IndexSize == 0)
        return false;
    return __recuSearch(0, Index.IndexSize - 1, buffer);
}
//-------------------------------------------------------------------------------------------
// insertion Section

// A function to insert element "element" into the permanent "RFile" in the correct position and changing the position info in Index
void RFile_insert(FILE *file, char *element, unsigned long elementSize)
{
    memset(buffer, 0, BUFFER_MAX_SIZE);     // Clear the current buffer
    memcpy(buffer, element, elementSize);   // Copy the insered element into buffer

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

int RecuInsert(file* file, fBlock* fblck, char* element, unsigned short elementSize, unsigned short shiftStart){
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
            
            free(element);
            return 1;
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
        return 0;
}

int insert(file* file, unsigned short element_Size){
    int elementNumber = insert_inIndex(file, element_Size);
    if(elementNumber == -1)
        return 1;

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

    if(RecuInsert(file, element_fBlck, tmpElement1, element_Size, element_IndiceInBlock))
        return 1;

    RFile_insert(file->RFile, tmpElement2, element_Size);

    return 0;
}
//--------------------------------------------------------------------------------------------
// Suppression Section

// Function to delete an element from the file (Logical)
int DeleteElementLogique(FILE* RFile){
    
    short indexElement = searchElement();
    if(indexElement == -1){
        printf("\nERROR! [Searching for Element]:already deleted or not Existe ");
        return 1;
    }
 
    Index.tab[indexElement].isDeletedLogically = true;
    RFile_Delete(RFile, indexElement);

    return 0;
}

// Delete in RFile
void RFile_Delete(FILE *file, short elementIndexPos)
{
    if (elementIndexPos == -1)
    { // Error handling
        fprintf(stderr, "ERROR! [searchElement in RFile_insert]: Couldn't find element in index.\nreturning...\n");
        return;
    }

    if (fseek(file, 0, SEEK_END))
    { // Error handling
        fprintf(stderr, "ERROR! [fseek 1 in RFile_Delete]: return a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    if (elementIndexPos == Index.IndexSize - 1)
    { // In case the element is the last in index, so no shifting needed
        fseek(file, -(Index.tab->endAddress - Index.tab->key), SEEK_END);
        fputc(-1, file);
        return;
    }

    // In case Not the last element in index, so there is shifting needed

    unsigned short pos = (Index.tab[elementIndexPos].filePos);
    unsigned long taille = (Index.tab[elementIndexPos].endAddress - Index.tab[elementIndexPos].key);
    char c;

    fseek(file, 0, SEEK_END);
    unsigned long shiftSize = ftell(file) - (pos + taille) ;        // formula to get how many bytes will be shifted

    char* tmpBuffer = (char*)malloc(shiftSize);
    if(tmpBuffer == NULL){
        fprintf(stderr, "ERROR! [malloc in RFile_delete]: Couldn't allocate space for tmpBuffer.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    if(fseek(file, pos + taille, SEEK_SET)){
        fprintf(stderr, "ERROR! [fseek in RFile_delete]: returned a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    unsigned long tmp = fread(tmpBuffer, 1, shiftSize, file);
    if(tmp != shiftSize){
        fprintf(stderr, "ERROR! [fread in RFile_delete]: expected to read %lu but read %lu.\nExiting...\n", shiftSize, tmp);
        exit(EXIT_FAILURE);
    }

    if(fseek(file, pos, SEEK_SET)){
        fprintf(stderr, "ERROR! [fseek in RFile_delete]: returned a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    tmp = fwrite(tmpBuffer, 1, shiftSize, file);
    if(tmp != shiftSize){
        fprintf(stderr, "ERROR! [fread in RFile_delete]: expected to write %lu but read %lu.\nExiting...\n", shiftSize, tmp);
        exit(EXIT_FAILURE);
    }

    fputc(-1, file);
}

void indexDelete(unsigned short elementIndex){
    for(unsigned short i = elementIndex; i < Index.IndexSize; i++)
        Index.tab[i] = Index.tab[i + 1];

    Index.IndexSize--;
}

// Function to delete an element from the file (Physique)
int DeleteElementPhysique(file *file){
    short elementIndex = searchElement();
    if(elementIndex == -1){
        fprintf(stderr, "ERROR! [searchElement in DeleteElementPhysique]: Element doesn't exist.\nReturning...\n");
        return 1;
    }

    file->header.NbStructs--;

    RFile_Delete(file->RFile, elementIndex);
    updateIndexFPos(file->RFile);

    if(Index.tab[elementIndex].blockAddress != Index.tab[elementIndex + 1].blockAddress && elementIndex == 0){
        memset(Index.tab[elementIndex].blockAddress->data->tab, 0, BUFFER_MAX_SIZE);
        file->header.NbStructs = 0;
        file->header.nbBlocks = 0;
        __setBlockAtStart(Index.tab[elementIndex].blockAddress->data);
        indexDelete(elementIndex);

        return 0;
    }

    if( (Index.tab[elementIndex - 1].blockAddress != Index.tab[elementIndex].blockAddress) && (Index.tab[elementIndex].blockAddress != \
    Index.tab[elementIndex + 1].blockAddress)){
        Index.tab[elementIndex - 1].blockAddress->next = Index.tab[elementIndex + 1].blockAddress;
        memset(Index.tab[elementIndex].blockAddress->data, 0, sizeof(Index.tab[elementIndex].blockAddress->data));
        file->header.nbBlocks--;
        indexDelete(elementIndex);

        return 0;
    }

    unsigned long elementSize = Index.tab[elementIndex].endAddress - Index.tab[elementIndex].key + 1;

    memcpy(buffer, Index.tab[elementIndex].blockAddress->data->tab, BUFFER_MAX_SIZE);

    // Indice of 1st character of the element after Index.tab[elementIndex]
    unsigned long i = Index.tab[elementIndex].endAddress - Index.tab[elementIndex].blockAddress->data->tab + 1;

    for(i; i < BUFFER_MAX_SIZE; i++)
        buffer[i - elementSize] = buffer[i];

    memcpy(Index.tab[elementIndex].blockAddress->data->tab, buffer, BUFFER_MAX_SIZE);
    fBlock* oldfBlockAdresse = Index.tab[elementIndex].blockAddress;
    indexDelete(elementIndex);
    update_fBlockInIndex(oldfBlockAdresse);
    update_fBlockHeader(oldfBlockAdresse);

    return 0;
}

///----------------------------------------------------------------------------------------------------------------

// Print Section

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
    while (file.head != NULL){
        printf("fBlock number %d:\n", nbFBlock);
        __printBlock(file.head->data);
        printf("\n-------------------------------------------\n");
        file.head = file.head->next;
        nbFBlock++;
    }
}
///-----------------------------------------------------------------------------

// Headers File

void StockHeaderecFile(FILE *Recfile, file *file)
{
    char filename[70]; // Change the filename as needed
    sprintf(filename, ".%s.header", (file->header).name);

    // Open file for writing, create if not exists
    Recfile = fopen(filename, "wb"); // 'w' searches file. if the file exists . its contents are overwritten . if the file doesn't existe . a new file is created.

    if (Recfile == NULL)
    {
        fprintf(stderr, "Error opening file %s\n", filename);
        return;
    }

    // Write fileHeader to the file
    fwrite(&(file->header), sizeof(fileHeader), 1, Recfile);

    // Close the file
    fclose(Recfile);
}