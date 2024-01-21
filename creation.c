#include "structs.h"
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
