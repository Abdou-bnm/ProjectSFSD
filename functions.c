#include "structs.h"

/// Part of Variable Declaration ------------------------------------------------------------------------------------------

// Global variables
char buffer[BUFFER_MAX_SIZE];               // A buffer to transfer data between RAM and Memory (used for file manipulation operations).
block MS[16];                               // The Memory which will contain all the blocks of the linked list and other blocks used by default.
IndexType Index;                            // An index associated to the file containing useful information for various operations.
/// -----------------------------------------------------------------------------------------------------------------------

char defiler(char file[2*BUFFER_MAX_SIZE],int *Endfile)
{
    char car = file[0];
    for(int i=0;i<99;i++)
    {
        *(file+i) = *(file+i+1);
    }
    (*Endfile)--;
    return car;
}

void enfiler(char file[2*BUFFER_MAX_SIZE],char car,int *Endfile)
{
        *(file + *Endfile) = car;
        (*Endfile)++;
}

block* allocBlock(){
    for(int i = 0; i < 16; i++)
        if(MS[i].isUsed == 0){
            MS[i].isUsed = 1;
            return &MS[i];
        }
    
    return NULL; 
}

void loadHeader(file* file){
    char *fileName = (char*)malloc(70);
    if(fileName == NULL){
        fprintf(stderr, "ERROR! [malloc 1 in loadHeader]: Couldn't allocate memory for fileName.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    memset(fileName, 0, 70);
    sprintf(fileName, ".%s.header", file->header.name);
    
    file->HFile = fopen(fileName, "rb");
    if(file->HFile == NULL){
        fprintf(stderr, "ERROR! [fopen in leadHeader]: Couldn't open header file \"%s\".\nExiting...\n", fileName);
        exit(EXIT_FAILURE);
    }

    free(fileName);
    fread(&(file->header), sizeof(fileHeader), 1, file->HFile);

    file->head = (fBlock*) malloc(sizeof(fBlock));
    if(file->head == NULL){
        fprintf(stderr, "ERROR! [malloc 2 in loadHeader]: Couldn't allocate memory for file->head.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    file->head->data = NULL;
    file->head->next = NULL;
    fBlock *fblck = file->head;
    for(unsigned int i = 0; i < file->header.nbBlocks - 1; i++){        // - 1 because the initial fBlock was made outside of the loop
        fblck = fblck->next;
        fblck = (fBlock*)malloc(sizeof(fBlock));
        if(file->head == NULL){
            fprintf(stderr, "ERROR! [malloc 3 in loadHeader]: Couldn't allocate memory for fblck.\nExiting...\n");
            exit(EXIT_FAILURE);
        }
        fblck->data = NULL;
        fblck->next = NULL;
    }

    if(fclose(file->HFile))
        fprintf(stderr, "ERROR! [fclose in loadHeader]: returned a non-zero value.\nContinuing...\n");
}

void __setBlockAtStart(block* blck){                    // Sets the block values when first allocated.
    blck->header.EndAddress = blck->tab;
    blck->header.StartAddress = blck->tab;
    blck->header.StartFreeSpaceAddress = blck->tab;
    blck->header.NbStructs = 0;
    blck->header.usedSpace = 0;
    memset(blck->tab, 0, BUFFER_MAX_SIZE);
}

void fileOpen(file* file){
    loadHeader(file);
    
    file->RFile = fopen(file->header.name, "rb+");
    if(file->RFile == NULL){
        fprintf(stderr, "ERROR! [fopen in fileOpen]: Couldn't open data file \"%s\".\nExiting...\n", file->header.name);
        exit(EXIT_FAILURE);
    }

    char c;
    int size, tmp = 0;
    fBlock *fblck = file->head;

    while(fblck != NULL){
        if(fblck->data == NULL){                        // In case the current block is not already set-up, or else we use the current block
            fblck->data = allocBlock();
            if(fblck->data == NULL){
                fprintf(stderr, "ERROR! [allocBlock in fileOpen]: Couldn't allocate block to use.\nReturning...\n");
                return;
            }
            __setBlockAtStart(fblck->data);
        }

        size = 0;
        do{
            c = fgetc(file->RFile);
            if(c == EOF){
                fprintf(stderr, "ERROR! [fgetc in fileOpen]: EOF reached before an expected end of struct '\\n'.\nReturning...\n");
                return;
            }
            size++;
        }while(c != '\n');
        
        fseek(file->RFile, - 1 * size, SEEK_CUR);
        
        if(size > BUFFER_MAX_SIZE - fblck->data->header.usedSpace){
            fblck = fblck->next;
            continue;                                   // Go to the next iteration
        }

        for(int i = 0; i < size; i++){
            c = fgetc(file->RFile);
            
            switch(c){
                case ':':
                    fblck->data->header.StartFreeSpaceAddress[i] = '\0';
                    break;
                
                case '\n':                              // character end of struct, change it to '#' if you are still using it, chahinez.        
                    fblck->data->header.StartFreeSpaceAddress[i] = '\0';
                    break;

                default:
                    fblck->data->header.StartFreeSpaceAddress[i] = c;
                    break;
            }
        }

        fblck->data->header.EndAddress += size;
        fblck->data->header.StartFreeSpaceAddress += size;
        fblck->data->header.NbStructs++;
        fblck->data->header.usedSpace += size;

        do{                                             // In case of some padding
            c = fgetc(file->RFile);
        }while(c == '\0');
        
        if(c == EOF){                                   // In case of reaching EOF, which means we completed the entire RFile
            printf("Copying RFile completed, EOF reached after end of struct.\nReturning...\n");
            return;
        }
    }                                                   // Else, we go into another iteration
}

void createfile(file* file){
    file->head = (fBlock*)malloc(sizeof(fBlock));
    file->head->data = allocBlock();
    if(file->head->data == NULL){
        fprintf(stderr, "\nERROR! [allocBlock in createfile]: No space to allocate block.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    file->head->next = NULL;
    file->header.NbStructs = 0;
    printf("Enter the name of the file (max size is 35 characters, spaces NOT allowed, CASE SENSITIVE): ");
    scanf("%36s", file->header.name);

    file->RFile = fopen(file->header.name, "w+");
    if(file->RFile == NULL){
        fprintf(stderr, "ERROR! [fopen in createfile]: couldn't create permanent file.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
}

/// Search Section
short __recuSearch(unsigned short startIndex, unsigned short endIndex, char* key){
    if(startIndex == endIndex)
        if(Index.tab[startIndex].isDeletedLogically)
            return -1;
    
    unsigned short median = (startIndex + endIndex) / 2;
    int strcmpResult = strncmp(key, Index.tab[median].key, KEY_MAX_SIZE);

    if(startIndex == endIndex && strcmpResult)
        return -1;

    if(!strcmpResult)       return median;
    if(strcmpResult < 1)
        return __recuSearch(startIndex, median, key);
    else
        return __recuSearch(median + 1, endIndex, key);
}


short searchElement(){
    if(Index.IndexSize == 0)                  return false;
    return __recuSearch(0, Index.IndexSize - 1, buffer);
}
//-------------------------------------------------------------------------------------------
// insertion Section

// A function to insert element "element" into the permanent "RFile" in the correct position and changing the position info in Index
void RFile_insert(FILE* file, char* element, unsigned long elementSize){
    memset(buffer, 0, sizeof(buffer));              // Clear the current buffer
    memcpy(buffer, element, elementSize);           // Copy the insered element into buffer

    short elementIndexPos = searchElement(), cpt = 0;
    if(elementIndexPos == -1){                      // Error handling
        fprintf(stderr, "ERROR! [searchElement in RFile_insert]: Couldn't find element in index.\nreturning...\n");
        return;
    }

    if(fseek(file, 0, SEEK_END)){                   // Error handling
        fprintf(stderr, "ERROR! [fseek 1 in RFile_insert]: return a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    
    if(elementIndexPos == Index.IndexSize - 1){     // In case the element is the last in index, so no shifting needed
        for(int i = 0; i < elementSize; i++){
            if(buffer[i] == '\0'){                  // Add ':' and '\n' instead of '\0' to organise RFile
                if(cpt % 2)
                    fputc('\n', file);
                else
                    fputc(':', file);
                cpt++;
                continue;
            }
            fputc(buffer[i], file);                 // Copy contents of buffer to file
        }
        return;
    }

    // In case Not the last element in index, so there is shifting needed

    long maxPos = ftell(file), space, err;                                              // Last position in file
    if(maxPos == -1){                                                                   // Error handling
        fprintf(stderr, "ERROR! [ftell in RFile_insert]: returned -1.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    space = maxPos - Index.tab[elementIndexPos + 1].filePos;                            // Number of bytes that will be shifted
    
    char* tmp = (char*)malloc(space);                                                   // Temporary buffer to hold the shifted string
    if(tmp == NULL){                                                                    // Error handling
        fprintf(stderr, "ERROR! [malloc in RFile_insert]: Couldn't allocate memory for \"tmp\" variable.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    memset(tmp, 0, space);                                                              // Setting the entire tmp buffer to 0

    if(fseek(file, Index.tab[elementIndexPos + 1].filePos, SEEK_SET)){                  // Error handling
        fprintf(stderr, "ERROR! [fseek 2 in RFile_insert]: returned a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    
    err = fread(tmp, sizeof(char), space, file);                                        // Read the shifted elements into tmp buffer
    if(err != space){                                                                   // Error handling
        fprintf(stderr, "ERROR! [fread in RFile_insert]: expected to read %li but got %li", space, err);
        exit(EXIT_FAILURE);
    }

    Index.tab[elementIndexPos].filePos = Index.tab[elementIndexPos + 1].filePos;        // Changing the filePos in the Index
    Index.tab[elementIndexPos + 1].filePos += elementSize;                              // Changing the filePos of the next element in the Index

    if(fseek(file, Index.tab[elementIndexPos].filePos, SEEK_SET)){                      // Error handling
        fprintf(stderr, "ERROR! [fseek 3 in RFile_insert]: returned a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < elementSize; i++){                                               // Add ':' and '\n' instead of '\0' to organise RFile
        if(buffer[i] == '\0'){                   
                if(cpt % 2)
                    fputc('\n', file);
                else
                    fputc(':', file);
                cpt++;
                continue;
            }
        fputc(buffer[i], file);                                                         // Writing the new element
    }

    err = fwrite(tmp, sizeof(char), space, file);                                       // Writing the rest of the old elements (shifted ones)
    if(err != space){                                                                   // Error handling
        fprintf(stderr, "ERROR! [fwrite 2 in RFile_insert]: expected to read %li but got %li", space, err);
        exit(EXIT_FAILURE);
    }

    free(tmp);                                                                          // Free the tmp buffer
}

fBlock *insertion(fBlock *head,char TabKey[KEY_MAX_SIZE],int SizeTabKey,int SizeTabRest){

    char TabKeyIndex[KEY_MAX_SIZE];
    fBlock *PtrF=head;

    //while (PtrF->data!=NULL || PtrF->next!=NULL){PtrF=PtrF->next;}//initialiser un ptr sur le block courrent 

    //COMMENCER L'INSERTION
    if (Index.IndexSize==0){
        
        //mettre le header a jour
        if(PtrF->data==NULL){head->data=allocBlock();head->next=NULL;PtrF=head;}
        PtrF->data->header.NbStructs=1;
        PtrF->data->header.usedSpace=SizeTabRest+SizeTabKey+3;
    
        //mettre lbuffer dans le tableau du block 
        for(int i=0;i<SizeTabRest+SizeTabKey+3;i++)
            PtrF->data->tab[i]=buffer[i];
    
        //the block is used, deja fait en haut mais bon
        PtrF->data->isUsed=1;
        
        //inserer dans l'indexe
        Index.IndexSize=1;
        Index.tab[0].key=&(PtrF->data->tab[0]);
        Index.tab[0].blockAddress=PtrF;
        Index.tab[0].endAddress=&(PtrF->data->tab[SizeTabKey-1]);
        Index.tab[0].isDeletedLogically=0;
    }
    else {
        int j=0,i=0;
        char *PtrStart=Index.tab[0].key,*PtrEnd=Index.tab[0].endAddress;
    
    //recuperer la chaine de car(cle) se trouvant dans l'indexe et la comparer avec celle entrée par l utilisateur
        while(PtrStart!=PtrEnd){
            TabKeyIndex[i]=*PtrStart;
            i++;
            PtrStart++;
        }

        printf("test 1 done");
        //chercher la premiere cle se trouvant dans l'indexe > a la cle entree par l'utilisateur 
        while(j<Index.IndexSize && strncmp(TabKeyIndex,TabKey,16)<0){
            j++;
            PtrStart=Index.tab[j].key;
            PtrEnd=Index.tab[j].endAddress;
            i=0;
            while(PtrStart!=PtrEnd){
                TabKeyIndex[i]=*PtrStart;
                i++;
                PtrStart++;
            }
        }
        printf("test 2 done");
        if(j>=Index.IndexSize){
            PtrF=Index.tab[j-1].blockAddress;
            printf("test 3 done");
            if(BUFFER_MAX_SIZE - PtrF->data->header.usedSpace >= SizeTabKey+SizeTabRest+3){
                //inserer dans l'indexe
                Index.tab[j].key=(Index.tab[j-1].endAddress + SizeTabRest + 4); //supposant que EndAderess se trouve au niveau du dernier caractere de la cle(+3 pour les 3 "\0" +1 pour se positionner sur la nouvelle case)
                Index.tab[j].endAddress=(Index.tab[j].key + SizeTabKey ); 
                Index.tab[j].blockAddress=PtrF;
                Index.tab[j].isDeletedLogically=0;
    
                //inserer dans le bloc
                //placer la chaine dans le bloc(elle se trouve a la fin de l'indexe donc c'est le dernier enregistrement du bloc courrant)
                PtrStart=Index.tab[j].key;
                PtrEnd=Index.tab[j].endAddress + SizeTabRest + 3 ;
                i=0;
                while(PtrStart!=PtrEnd){
                    *PtrStart=buffer[i];
                    i++;
                    PtrStart++;
                }
                printf("test 4 done");
            }
            else {
                //allouer un nv bloc
                PtrF->next->data=allocBlock();
                PtrF->next->next=NULL; 
                PtrF=PtrF->next;//deplacer le ptr
                
                //inserer dans l'indexe
                Index.tab[j].key=&(PtrF->data->tab[0]);
                Index.tab[j].endAddress=(Index.tab[j].key + SizeTabKey ); 
                Index.tab[j].blockAddress=PtrF;
                Index.tab[j].isDeletedLogically=0;
    
                //inserer dans le bloc
                //placer la chaine entree par l'utilisateur au niveau du debur du nouveau bloc 
                //a revoir
                PtrStart=Index.tab[j].key;
                PtrEnd=Index.tab[j].endAddress + SizeTabRest + 3;
                i=0;
                while(PtrStart!=PtrEnd){
                    *PtrStart=buffer[i];
                    i++;
                    PtrStart++;
                }  
                printf("test 5 done");
            }
        }
        else{
            printf("test 6 done");
            //pour l'insertion dans l'indexe :
            char *PtrKey=Index.tab[j].key;
            fBlock *PtrBlockKey=Index.tab[j].blockAddress;

            //file pour stocker les donnees
            char file[2*BUFFER_MAX_SIZE];
            int Endfile =0; //fin de file et aussi taille de file


            int N=(SizeTabKey+SizeTabRest+3)-(BUFFER_MAX_SIZE-Index.tab[j].blockAddress->data->header.usedSpace);
            fBlock *P=Index.tab[j].blockAddress; //pointeur sur le block du premier element > a la cle
            char *Qtab =&(P->data->tab[BUFFER_MAX_SIZE - P->data->header.usedSpace -1]); //pointeur sur le dernier element du block du premier element > a la cle
            //enfiler l'enregistrement 
            for(int i=0;i<SizeTabKey+SizeTabRest+3;i++){
            enfiler(file,buffer[i],&Endfile);
            }
            printf("test 7 done");
            while (Endfile!=0)
            {
                int Sizefile=Endfile;//stocker la taille de la pile
                int cpt=0;
                if(N>0){
                while(cpt<N || *Qtab != '#' ){
                        enfiler(file,*Qtab,&Endfile);
                        cpt++;
                        Qtab--;
                    }
                }   
                printf("test 8 done");
                if(P==Index.tab[j].blockAddress){
                    char *QtabPrime=Qtab, *Ptab=Index.tab->key; //pointeur sur element de fin du block + pointeur sur ou on va inserer la cle
                    int nb=Qtab-Ptab; // nb d'occurrences;
                    for (int i=0;i<SizeTabKey+SizeTabRest+3;i++){
                        Qtab=QtabPrime+i;
                        for(int k=0;k<nb-1;k++){
                            *(Qtab+1)=*Qtab;
                            Qtab--;
                        }   
                    }
                    P->data->header.usedSpace=P->data->header.usedSpace - N + SizeTabKey+SizeTabRest+3;//update de l'espace du block
                    //insertion de la donnee
                    for (int i=0;i<SizeTabKey+SizeTabRest+3;i++){
                    *Ptab=defiler(file,&Endfile);
                        Ptab++;
                    }
                            printf("test 9 done");
                }
                else if(P->next!=NULL){
                char *Ptab=&(P->data->tab[P->data->header.usedSpace-1]);//pointeur sur le dernier element du block
                char *PtabPrime=Ptab ,*Rtab=NULL;//Rtab nous aidera pour l'insertion c'est tout
                int nb=P->data->header.usedSpace;//nb d'occurrences 
                //decalage
                for (int i=0;i<Sizefile;i++){
                    Ptab=PtabPrime+i;
                    for(int k=0;k<nb-1;k++){
                        *(Ptab+1)=*Ptab;
                        Ptab--;
                        }   
                    }

                P->data->header.usedSpace=P->data->header.usedSpace - N + Sizefile;//update de l'espace du block
                Rtab=&(P->data->tab[0]);//placer Rtab sur le premier element du block
                //commencer a mettre les elements de la file au tout debut du block
                for (int i=0;i<Sizefile;i++){
                    *Rtab=defiler(file,&Endfile);
                    Rtab++;
                    }
                }
                else {
                            printf("test 10 done");
                P->next->data=allocBlock();
                P->next->next=NULL;
                P=P->next;
                //defiler directement les elements de la pile dans le block
                char *Rtab=&(P->data->tab[0]);
                for (int i=0;i<Sizefile;i++){
                    *Rtab=defiler(file,&Endfile);
                    Rtab++;
                    }    
                }
                P=P->next;
                Qtab =&(P->data->tab[BUFFER_MAX_SIZE - P->data->header.usedSpace-1]);
                N=(Sizefile)-(BUFFER_MAX_SIZE - P->data->header.usedSpace);
            }
        //traitement de l'indexe
            // decaler les elements de l'indexe
        int i=Index.IndexSize;
            while(i>=j){//pour l'egalite pas sure (a revoir)
                Index.tab[i]=Index.tab[i-1];
                i--;
            }
            Index.tab[j].key=PtrKey;
            Index.tab[j].endAddress=PtrKey + SizeTabKey;
            Index.tab[j].blockAddress=PtrBlockKey;
            Index.tab[j].isDeletedLogically=0;
                    printf("test 11 done");
        }
    }
    printf("\nElement Added Successfully!\n");
    return(head);
}
//--------------------------------------------------------------------------------------------
// Suppression Section
void ElementShift(char** NewElementPos,char* StartCurElementPos,char* EndCurElementPos)
{
    //Shift the Element to New Pos
    while(StartCurElementPos != EndCurElementPos)
    {
        **NewElementPos = *StartCurElementPos;
        *StartCurElementPos = 0;
        *NewElementPos += sizeof(char);
        StartCurElementPos += sizeof(char);
    }
    **NewElementPos = *StartCurElementPos;
}

int CalculateSpace(char *StartEspaceAddress, char *EndEspaceAddress){
    return EndEspaceAddress - StartEspaceAddress; 
}

// Function to update index array (Delete Element)
void UpdateIndexDelete(int IndexElementDeleted){
    for (int ElementIndex = IndexElementDeleted; ElementIndex < Index.IndexSize ; ElementIndex++)
    {
        Index.tab[ElementIndex] = Index.tab[ElementIndex+1];                         
    }

    Index.IndexSize -- ;
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
        return ;
    }
    else
    {
        Index.tab[indexElement].isDeletedLogically = true;
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
        char *EndCurElementPos,*StartCurElementPos;
        char *NewElementPos = Index.tab[indexElementDeleted].key;
        block* blockAddressdataRecover = (Index.tab[indexElementDeleted].blockAddress)->data; //Save Address of Block 
        for(int i=indexElementDeleted + 1 ; i<Index.IndexSize ; i++)
        {
            // Verify if the Shift Will be in the Same Block or not
            if((Index.tab[i].blockAddress)->data != (Index.tab[i-1].blockAddress)->data) // Shift the element into a different block
            {
                FreeSpace = (((Index.tab[i-1].blockAddress)->data)->header).EndAddress - NewElementPos; // Calculate the remaining free space in the block
                // Verify if FreeSpace is sufficient for the Element
                if(CalculateSpace(Index.tab[i].key,Index.tab[i].endAddress) <= FreeSpace) // FreeSpace is sufficient => Make the Element in the FreeSpace
                {   
                    EndCurElementPos = Index.tab[i].endAddress;
                    StartCurElementPos = Index.tab[i].key;
                    Index.tab[i].key = NewElementPos;
                    ElementShift(&NewElementPos,StartCurElementPos,EndCurElementPos);
                    Index.tab[i].endAddress = NewElementPos;
                    *NewElementPos += 2*sizeof(char);
                    NbElement++;
                }
                else // FreeSpace isn't sufficient => Make the Element in New Block
                {
                    if(NbElement == -1) // element deleted was in base of block and the next element have more space 
                    {
                        (((Index.tab[i].blockAddress)->data)->header).NbStructs -= NbElement+1; //Update Number of Element in New Block
                    }
                    (((Index.tab[i-1].blockAddress)->data)->header).NbStructs += NbElement; //Update Number of Element in Current Block
                    
                    NbElement= 0; 
                    NewElementPos = blockAddressdataRecover->tab;
                    EndCurElementPos = Index.tab[i].endAddress;
                    StartCurElementPos = Index.tab[i].key;
                    if(NewElementPos == StartCurElementPos)
                    {
                        return -1; // Shift Complete (Block no need to do shift on it)
                    }
                    Index.tab[i].key = NewElementPos;
                    ElementShift(&NewElementPos,StartCurElementPos,EndCurElementPos);
                    Index.tab[i].endAddress = NewElementPos;
                    *NewElementPos += 2*sizeof(char);
                }             
            }
            else// Shift the element within the same block
            {
                EndCurElementPos = Index.tab[i].endAddress;
                StartCurElementPos = Index.tab[i].key;
                Index.tab[i].key = NewElementPos;
                ElementShift(&NewElementPos,StartCurElementPos,EndCurElementPos);
                Index.tab[i].endAddress = NewElementPos;
                *NewElementPos += 2*sizeof(char);
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

        printf("\nElement Deleted Successfully!\n");
    }
}
///----------------------------------------------------------------------------------------------------------------

// Print Section


// Prints the data in decimal and below it the asscoiated character to that ASCII code, similair to hexdump, useful for debugging
void decimalPrint(char* src, int size){
    for(int i = 0; i < size; i++)
        printf("%d\t", src[i]);
    printf("\n");
    for(int i = 0; i < size; i++)
        printf("%c\t", src[i]);
    printf("\n\n");
}

// Prints a block, struct by struct

void __printBlock(block* block){
    int i = 0, nbStruct = 1;
    while(block->tab[i] != '\0'){
        printf("Struct number %d:\n", nbStruct);

        printf("Key: \"%s\".\n", &block->tab[i]);
        i += strlen(&block->tab[i]) + 1;


        printf("Data: \"%s\".\n\n", &block->tab[i]);
        i += strlen(&block->tab[i]) + 1;
        nbStruct++;
    }
}

// Prints the entire file, block by block

void printFile(file file){
    int nbFBlock = 1;
    while(file.head != NULL){
        printf("fBlock number %d:\n", nbFBlock);
        __printBlock(file.head->data);
        printf("\n-------------------------------------------\n");
        file.head = file.head->next;
    }
}
///-----------------------------------------------------------------------------

