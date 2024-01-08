#include "structs.h"

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

void createfile(file* file){
    file->head = (fBlock*)malloc(sizeof(fBlock));
    file->head->data = allocBlock();
    if(file->head->data == NULL){
        fprintf(stderr, "\nERROR! [allocBlock in createfile]: No space to allocate block.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    file->head->next = NULL;
    file->header.NbStructs = 0;
    printf("Enter the name of the file (max size is 35 characters, spaces NOT allowed): ");
    scanf("%36s", file->header.name);
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

/// insertion Section
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