#include "structs.h"

/// Part of Variable Declaration ------------------------------------------------------------------------------------------

// Global variables
char buffer[BUFFER_MAX_SIZE]; // A buffer to transfer data between RAM and Memory (used for file manipulation operations).
block MS[MEMORY_BLOCK_MAX];    // The Memory which will contain all the blocks of the linked list and other blocks used by default.
IndexType Index;              // An index associated to the file containing useful information for various operations.
/// -----------------------------------------------------------------------------------------------------------------------

block *allocBlock()
{
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
            fgetc(file);
        
        fgetc(file);                                // One more time to set it to the start of the next element
    }
}

void loadHeader(file *file)
{
    char *fileName = (char *)malloc(70);
    if (fileName == NULL)
    {
        fprintf(stderr, "ERROR! [malloc 1 in loadHeader]: Couldn't allocate memory for fileName.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    memset(fileName, 0, 70);
    sprintf(fileName, ".%s.header", file->header.name);

    file->HFile = fopen(fileName, "rb");
    if (file->HFile == NULL)
    {
        fprintf(stderr, "ERROR! [fopen in leadHeader]: Couldn't open header file \"%s\".\nExiting...\n", fileName);
        exit(EXIT_FAILURE);
    }

    free(fileName);
    fread(&(file->header), sizeof(fileHeader), 1, file->HFile);

    fBlock *fblck = file->head;
    for(int i = 0; i < file->header.nbBlocks; i++){
        fblck = (fBlock*)malloc(sizeof(fBlock));
        if(fblck == NULL){
            fprintf(stderr, "ERROR! [malloc 2 in loadHeader]: Couldn't allocate memory for fblck.\nExiting...\n");
            exit(EXIT_FAILURE);
        }

        fblck->data = NULL;
        fblck->next = NULL;

        fblck = fblck->next;
    }

    if (fclose(file->HFile))
        fprintf(stderr, "ERROR! [fclose in loadHeader]: returned a non-zero value.\nContinuing...\n");
}

void __setBlockAtStart(block *blck)
{ // Sets the block values when first allocated.
    blck->header.EndAddress = blck->tab;
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
            fprintf(stderr, "ERROR! [fseek in fileOpen]: returned a non-zero value.\nExiting...\n");
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
        Index.tab[Index.IndexSize].endAddress = fblck->data->header.StartFreeSpaceAddress + size;
        Index.tab[Index.IndexSize].isDeletedLogically = false;
        Index.tab[Index.IndexSize].blockAddress = fblck;
        Index.IndexSize++;
        
        // Reading the actual element into its respective block
        for (int i = 0; i < size; i++)
        {
            c = fgetc(file->RFile);

            switch (c){
                case ':':
                    fblck->data->header.StartFreeSpaceAddress[i] = '\0';
                    break;

                case '\n': // character end of struct, change it to '#' if you are still using it, chahinez.
                    fblck->data->header.StartFreeSpaceAddress[i] = '\0';
                    break;

                default:
                    fblck->data->header.StartFreeSpaceAddress[i] = c;
                    break;
                }
        }

        // Updating the block's header info
        fblck->data->header.EndAddress += size;
        fblck->data->header.StartFreeSpaceAddress += size;
        fblck->data->header.NbStructs++;
        fblck->data->header.usedSpace += size;

        file->header.NbStructs++;

        do{ // In case of some padding
            c = fgetc(file->RFile);
        } while (c == '\0');

        if (c == EOF)
        { // In case of reaching EOF, which means we completed the entire RFile
            printf("Copying RFile completed, EOF reached after end of struct.\nReturning...\n");
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
    printf("Enter the name of the file (max size is 35 characters, spaces NOT allowed, CASE SENSITIVE): ");
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

//--------------------------------------------------------------------------------------------
// Suppression Section
void ElementShift(char **NewElementPos, char *StartCurElementPos, char *EndCurElementPos)
{
    // Shift the Element to New Pos
    while (StartCurElementPos != EndCurElementPos)
    {
        **NewElementPos = *StartCurElementPos;
        *StartCurElementPos = 0;
        *NewElementPos += sizeof(char);
        StartCurElementPos += sizeof(char);
    }
    **NewElementPos = *StartCurElementPos;
}

int CalculateSpace(char *StartEspaceAddress, char *EndEspaceAddress)
{
    return EndEspaceAddress - StartEspaceAddress;
}

// Function to update index array (Delete Element)
void UpdateIndexDelete(int IndexElementDeleted)
{
    for (int ElementIndex = IndexElementDeleted; ElementIndex < Index.IndexSize; ElementIndex++)
    {
        Index.tab[ElementIndex] = Index.tab[ElementIndex + 1];
    }

    Index.IndexSize--;
}

// Function to update File
void UpdateFileStruct(file *file)
{
    fBlock *ftmp = (*file).head;
    while ((ftmp)->next != NULL)
    {
        ftmp = ftmp->next;
    }
}

// Function to delete an element from the file (Logical)
void DeleteElementLogique(FILE* RFile)
{
    short indexElement = searchElement();
    if (indexElement == -1)
    {
        printf("\nERROR! [Searching for Element]:already deleted or not Existe ");
        return;
    }
    else
    {
        Index.tab[indexElement].isDeletedLogically = true;
        RFile_Delete(RFile, indexElement);
    }
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
    fseek(file, pos + taille, SEEK_SET);
    // Shift the remaining characters to the left
    int size;
    int i = 0;
    while ((c = fgetc(file)) != EOF)
    {
        memset(buffer, 0, sizeof(buffer)); // Clear the current buffer
        i = 1;
        size=1;
        buffer[0] = c;
        
        do  // Copy Record to Buffer
        {   
            c = fgetc(file);
            buffer[i] = c;
            i++;
        } while (c != '\n');
        
        buffer[i] = '\0';

        fseek(file,pos, SEEK_SET);
        fputs(buffer, file); //Shift the record to new pos
        pos = pos + i;
        fseek(file, ftell(file) + taille, SEEK_SET);// Jump to next Record
    }
}

// Function to delete an element from the file (Physique)
int DeleteElementPhysique(file *file,FILE *Rfile){

    // Search for the index of the element to be deleted
    short indexElementDeleted = searchElement();

    // Check if the element is not found or already deleted
    if (indexElementDeleted == -1)
    {
        printf("\nERROR! [Searching for Element]:already deleted or not Existe");
        return -1;
    }
    else
    {
        unsigned short NbElement = -1;
        int FreeSpace = 0;
        char *EndCurElementPos, *StartCurElementPos;
        char *NewElementPos = Index.tab[indexElementDeleted].key;
        block *blockAddressdataRecover = (Index.tab[indexElementDeleted].blockAddress)->data; // Save Address of Block
        for (int i = indexElementDeleted + 1; i < Index.IndexSize; i++)
        {
            // Verify if the Shift Will be in the Same Block or not
            if ((Index.tab[i].blockAddress)->data != (Index.tab[i - 1].blockAddress)->data) // Shift the element into a different block
            {
                FreeSpace = (((Index.tab[i - 1].blockAddress)->data)->header).EndAddress - NewElementPos; // Calculate the remaining free space in the block
                // Verify if FreeSpace is sufficient for the Element
                if (CalculateSpace(Index.tab[i].key, Index.tab[i].endAddress) <= FreeSpace) // FreeSpace is sufficient => Make the Element in the FreeSpace
                {
                    EndCurElementPos = Index.tab[i].endAddress;
                    StartCurElementPos = Index.tab[i].key;
                    Index.tab[i].key = NewElementPos;
                    ElementShift(&NewElementPos, StartCurElementPos, EndCurElementPos);
                    Index.tab[i].endAddress = NewElementPos;
                    *NewElementPos += 2 * sizeof(char);
                    NbElement++;
                }
                else // FreeSpace isn't sufficient => Make the Element in New Block
                {
                    if (NbElement == -1) // element deleted was in base of block and the next element have more space
                    {
                        (((Index.tab[i].blockAddress)->data)->header).NbStructs -= NbElement + 1; // Update Number of Element in New Block
                    }
                    (((Index.tab[i - 1].blockAddress)->data)->header).NbStructs += NbElement; // Update Number of Element in Current Block

                    NbElement = 0;
                    NewElementPos = blockAddressdataRecover->tab;
                    EndCurElementPos = Index.tab[i].endAddress;
                    StartCurElementPos = Index.tab[i].key;
                    if (NewElementPos == StartCurElementPos)
                    {
                        return -1; // Shift Complete (Block no need to do shift on it)
                    }
                    Index.tab[i].key = NewElementPos;
                    ElementShift(&NewElementPos, StartCurElementPos, EndCurElementPos);
                    Index.tab[i].endAddress = NewElementPos;
                    *NewElementPos += 2 * sizeof(char);
                }
            }
            else // Shift the element within the same block
            {
                EndCurElementPos = Index.tab[i].endAddress;
                StartCurElementPos = Index.tab[i].key;
                Index.tab[i].key = NewElementPos;
                ElementShift(&NewElementPos, StartCurElementPos, EndCurElementPos);
                Index.tab[i].endAddress = NewElementPos;
                *NewElementPos += 2 * sizeof(char);
            }
            // Update the block address for the next iteration
            blockAddressdataRecover = (Index.tab[i].blockAddress)->data;
        }

        // Update IndexArray to reflect the deletion
        UpdateIndexDelete(indexElementDeleted);

        // Update Last Block
        if ((((Index.tab[Index.IndexSize].blockAddress)->data)->header).NbStructs == 0)
        {
            ((Index.tab[Index.IndexSize].blockAddress)->data)->isUsed = false;
        }

        // Update nb Element in file header
        file->header.NbStructs--;

        // Delete in the Rfile  
        RFile_Delete(Rfile,indexElementDeleted);

        printf("\nElement Deleted Successfully!\n");
    }
}
///----------------------------------------------------------------------------------------------------------------

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