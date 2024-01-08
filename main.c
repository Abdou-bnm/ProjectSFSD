#include "structs.h"
block* allocBlock(){
    for(int i = 0; i < 16; i++)
        if(MS[i].isUsed == 0){
            MS[i].isUsed = 1;
            return &MS[i];
        }
    
    return NULL; 
}

short __recuSearch(unsigned short startIndex, unsigned short endIndex, char* key){
    if(startIndex == endIndex)
        if(Index.tab[startIndex].isDeletedLogically)

    
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
    if(Index.indexSize == 0)                  return false;
    return __recuSearch(0, Index.indexSize - 1, buffer);
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



fBlock *insertion(fBlock *head,char TabKey[KEY_MAX_SIZE],int SizeTabKey,int SizeTabRest){

char TabKeyIndex[KEY_MAX_SIZE];
fBlock *PtrF=head;
 
//COMMENCER L'INSERTION
if (Index.IndexSize==0){
    printf("%s\n\n",buffer);
    //mettre le header a jour
    if(PtrF->data==NULL){head->data=allocBlock();head->next=NULL;PtrF=head;}
    PtrF->data->header.NbStructs=1;
    PtrF->data->header.usedSpace=SizeTabRest+SizeTabKey+3;
   
    //mettre lbuffer dans le tableau du block 
    for(int i=0;i<SizeTabRest+SizeTabKey+3;i++)
       { PtrF->data->tab[i]=buffer[i];}
    //   printf("\n%s\n\n",buffer);
    // for(int i=0;i<SizeTabRest+SizeTabKey+3;i++){
    //         printf("%c",PtrF->data->tab[i]);}
    //the block is used, deja fait en haut mais bon
    PtrF->data->isUsed=1;
    
    //inserer dans l'indexe 
    Index.IndexSize=1;
    Index.tab[0].key=&(PtrF->data->tab[0]);
    Index.tab[0].blockAddress=PtrF;
    Index.tab[0].endAddress=&(PtrF->data->tab[SizeTabKey+SizeTabRest+2]);
    Index.tab[0].isDeletedLogically=0;
     
}
else {
    
    int j=0,i=0;
    char *PtrStart=Index.tab[0].key,*PtrEnd=EndKey(PtrStart);

    // printf("\nptrstart%p",PtrStart);
    // printf("\nptrend%p",PtrEnd);
   //recuperer la chaine de car(cle) se trouvant dans l'indexe et la comparer avec celle entrée par l utilisateur
    while(PtrStart<=PtrEnd){
        TabKeyIndex[i]=*PtrStart;
        //printf("%s",TabKeyIndex);
        i++;
        PtrStart++;
    }
    //printf("test2"); 
    TabKeyIndex[i]='\0';
    
    //chercher la premiere cle se trouvant dans l'indexe > a la cle entree par l'utilisateur 
    while(j<Index.IndexSize && strncmp(TabKeyIndex,TabKey,16)<0){
        PtrStart=Index.tab[j].key;
        PtrEnd=EndKey(PtrStart);
        i=0;
        while(PtrStart<=PtrEnd){
            TabKeyIndex[i]=*PtrStart;
            i++;
            PtrStart++;
        }
        j++;
    }
    if(j==0){j++;}
    if(j>=Index.IndexSize){
        PtrF=Index.tab[j-1].blockAddress;
        if(BUFFER_MAX_SIZE - PtrF->data->header.usedSpace >= SizeTabKey+SizeTabRest+3){
            //inserer dans l'indexe
            Index.tab[j].key=(Index.tab[j-1].endAddress + 1); 
            Index.tab[j].endAddress=(Index.tab[j].key + SizeTabKey +SizeTabRest+3 ); 
            Index.tab[j].blockAddress=PtrF;
            Index.tab[j].isDeletedLogically=0;
            Index.IndexSize++;
   
            //inserer dans le bloc
            //placer la chaine dans le bloc(elle se trouve a la fin de l'indexe donc c'est le dernier enregistrement du bloc courrant)
            PtrStart=Index.tab[j].key;
            PtrEnd=Index.tab[j].endAddress ;
            i=0;
            while(PtrStart<=PtrEnd){
                *PtrStart=buffer[i];
                i++;
                PtrStart++;
            }
            PtrF->data->header.usedSpace=PtrF->data->header.usedSpace+ SizeTabRest + SizeTabKey +3;
            //  printf("\n%s\n\n",buffer);
            //  PtrStart=Index.tab[j].key;
            // for(int i=0;i<SizeTabRest+SizeTabKey+3;i++){
            //     printf("%c",*PtrStart);
            //     PtrStart++;
            // }
        }
        else {
            //allouer un nv bloc
            PtrF->next= (fBlock*)(malloc(sizeof(fBlock)));
            PtrF->next->data=allocBlock();
            PtrF->next->next=NULL; 
            PtrF=PtrF->next;//deplacer le ptr
            PtrF->data->header.usedSpace=0;
            printf("\nNOUVEAU BLOCK\n");
            //inserer dans l'indexe
            Index.tab[j].key=&(PtrF->data->tab[0]);
            Index.tab[j].endAddress=(Index.tab[j].key + SizeTabKey +SizeTabRest+3); 
            Index.tab[j].blockAddress=PtrF;
            Index.tab[j].isDeletedLogically=0;
            Index.IndexSize++;
   
            //inserer dans le bloc
            //placer la chaine entree par l'utilisateur au niveau du debur du nouveau bloc 
            //a revoir
            PtrStart=Index.tab[j].key;
            PtrEnd=Index.tab[j].endAddress+ SizeTabRest + 3;
            i=0;
            while(PtrStart<=PtrEnd){
                *PtrStart=buffer[i];
                i++;
                PtrStart++;
            }  
         PtrF->data->header.usedSpace=SizeTabKey+SizeTabRest+3;
        }
        //printf("fin");
    }
    else{
        //pour l'insertion dans l'indexe :
        char *PtrKey=Index.tab[j-1].key;
        fBlock *PtrBlockKey=Index.tab[j-1].blockAddress;
        //printf("else\n");
        //file pour stocker les donnees
        char file[2*BUFFER_MAX_SIZE];
        int Endfile =0; //fin de file et aussi taille de file

        int N=(SizeTabKey+SizeTabRest+3)-(BUFFER_MAX_SIZE-Index.tab[j].blockAddress->data->header.usedSpace);
        fBlock *P=Index.tab[j-1].blockAddress; //pointeur sur le block du premier element > a la cle
        char *Qtab =&(P->data->tab[P->data->header.usedSpace ]); //pointeur sur le dernier element du block du premier element > a la cle
        //enfiler l'enregistrement 
        for(int i=0;i<SizeTabKey+SizeTabRest+3;i++){
        printf("enfilerenregistrement\n");

         enfiler(file,buffer[i],&Endfile);
        }
        printf("%d",Endfile);
        
        while (Endfile!=0)
        {
                printf("\n\nwhileEndFile");
            int Sizefile=Endfile;//stocker la taille de la pile
            printf("\nQTAB:%p + Contenu:%c\n",Qtab,*Qtab);
            int cpt=0;
            printf("\n%d\n",N);
            if(N>0){
             while(cpt<N || *Qtab != '#' ){
                    enfiler(file,*Qtab,&Endfile);
                    cpt++;
                    Qtab--;
                }
            }   
            printf("\nQTAB:%p\n",Qtab);
            if(P==Index.tab[j-1].blockAddress){
                //decalage         
                printf("\nIF1\n");

                char *QtabPrime=Qtab, *Ptab=Index.tab[j-1].key; //pointeur sur element de fin du block + pointeur sur ou on va inserer la cle
                int nb=Qtab-Ptab +1 ; // nb d'occurrences;
                printf("%d",nb);
                printf("\nQTAB:%p\nPTAB:%p\n",Qtab,Ptab);
                for (int i=0;i<SizeTabKey+SizeTabRest+3;i++){
                    Qtab=QtabPrime+i;
                    for(int k=0;k<nb;k++){
                        *(Qtab+1)=*Qtab;
                       // printf("%c      %c\n",*Qtab,*(Qtab+1));
                        Qtab--;
                    }   
                }
               // printf("test");
                P->data->header.usedSpace=P->data->header.usedSpace - N + SizeTabKey+SizeTabRest+3;//update de l'espace du block
                //insertion de la donnee
                for (int i=0;i<SizeTabKey+SizeTabRest+3;i++){
                 *Ptab=defiler(file,&Endfile);
                   printf("%c",*Ptab);
                    Ptab++;
                }
                printf("%d",Endfile);
            }
            else if(P!=NULL){
             char *Ptab=&(P->data->tab[P->data->header.usedSpace-1]);//pointeur sur le dernier element du block
             char *PtabPrime=Ptab ,*Rtab=NULL;//Rtab nous aidera pour l'insertion c'est tout
             int nb=P->data->header.usedSpace+1;//nb d'occurrences 
             printf("nombre doccu:%d",nb);
             //decalage
             for (int i=0;i<Sizefile;i++){
                 Ptab=PtabPrime+i;
                 for(int k=0;k<nb;k++){
                     *(Ptab+1)=*Ptab;
                     Ptab--;
                    }   
                }
               // char c=defiler(file,&Endfile);
               if(N>0){
             P->data->header.usedSpace=P->data->header.usedSpace - N + Sizefile ;}
             else{P->data->header.usedSpace=P->data->header.usedSpace + Sizefile ;}//update de l'espace du block
             Rtab=&(P->data->tab[0])+Sizefile-1;//placer Rtab sur le premier element du block
             //commencer a mettre les elements de la file au tout debut du block
             for (int i=0;i<Sizefile;i++){
                 *Rtab=defiler(file,&Endfile);
                 Rtab--;
                }
                printf("\n\n%d\n",Endfile);  
            }
            else {
             P=head;
             while(P->next!=NULL){P=P->next;}
             P->next= (fBlock*)(malloc(sizeof(fBlock)));
             P->next->data=allocBlock();
             P->next->next=NULL;
             P=P->next;
             P->data->header.usedSpace=0;
             //defiler directement les elements de la pile dans le block
             char *Rtab=&(P->data->tab[0])+Sizefile-1;
             for (int i=0;i<Sizefile;i++){
                 *Rtab=defiler(file,&Endfile);
                 Rtab--;
                 P->data->header.usedSpace++;
                }  
            }
            P=P->next;
            if(P!=NULL){
             Qtab =&(P->data->tab[P->data->header.usedSpace]);
             N=(Sizefile)-(BUFFER_MAX_SIZE - P->data->header.usedSpace);
             printf("UsedSpace: %d",P->data->header.usedSpace);
            }
        }
            printf("erruer");
     //traitement de l'indexe
         // decaler les elements de l'indexe
       int i=Index.IndexSize;
        while(i>=j){//pour l'egalite pas sure (a revoir)
            Index.tab[i]=Index.tab[i-1];
            i--;
        }
        Index.tab[j-1].key=PtrKey;
        Index.tab[j-1].endAddress=PtrKey + SizeTabKey+SizeTabRest+1;
        Index.tab[j-1].blockAddress=PtrBlockKey;
        Index.tab[j-1].isDeletedLogically=0;
        Index.IndexSize++;
    //    printf("\n\n");
    //     char *ptr=Index.tab[j].key;
    //     for(int i=0;i<SizeTabRest+SizeTabKey+3;i++){
    //         printf("%c",*ptr);
    //         ptr++;
    //     }
    //     printf("\n\n");
    //     while(*ptr!='#')
    //     {
    //         printf("%c",*ptr);
    //         ptr++;
    //     }

    
    }
    char *ptr=Index.tab[j].key;
        for(int i=0;i<SizeTabRest+SizeTabKey+3;i++){
            printf("%ca",*ptr);
            ptr++;
        }
}
return(head);

}

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
    for (int ElementIndex = IndexElementDeleted; ElementIndex < Index.indexSize ; ElementIndex++)
    {
        Index.tab[ElementIndex] = Index.tab[ElementIndex+1];                         
    }

    Index.indexSize -- ;
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
        for(int i=indexElementDeleted + 1 ; i<Index.indexSize ; i++)
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
        if ((((Index.tab[Index.indexSize].blockAddress)->data)->header).NbStructs == 0)
        {
            ((Index.tab[Index.indexSize].blockAddress)->data)->isUsed = false;
        }

        // Update nb Element in file header
        file->header.NbStructs--;

        printf("\nElement Deleted!");
    }
}


int main(int argc, char const *argv[]){
    unsigned short answer;                                  // Used to get user's answers
    file file;
    Index.IndexSize=0;
    memset(&file, 0, sizeof(file));
    file.head=NULL;
    
    printf("Do you want to create a new file?\n");
    printf("1- yes\n");
    printf("2- no\n");
    scanf("%hu", &answer);                                  // "%hu" format specifier for unsigned short
    if(answer == 1)
        createfile(&file);
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
            //entrer la cle 
            printf("enter the key");
            scanf("%16s", buffer);
            getchar();
             printf("%s\n\n",buffer);
            int i=0;
            char TabKey[KEY_MAX_SIZE],TabRest[BUFFER_MAX_SIZE];
           strcpy(TabKey,buffer);
            int SizeTabKey=strlen(TabKey);
            //printf("%s\n\n",TabKey);
            //printf("%d\n\n",SizeTabKey);
            buffer[SizeTabKey]='#';
           //insertion of the rest of the struct
            printf("enter the rest of the structs");
            fgets(buffer+SizeTabKey+1,BUFFER_MAX_SIZE-SizeTabKey-3,stdin); 
            //printf("%s\n\n",buffer);
            int SizeBuffer=strlen(buffer);
            //printf("\n%d",SizeBuffer);
            SizeBuffer--;
            buffer[SizeBuffer]='\0';
            buffer[SizeBuffer+1]='#';
            buffer[SizeTabKey]='\0';
            int j=0;
            for(int i=SizeTabKey+1;i<=SizeBuffer;i++){
                TabRest[j]=buffer[i]; j++;
            }
            //printf("\n%s\n\n",TabRest);
            int SizeTabRest = strlen(TabRest);
            //printf("\n%d",SizeTabRest);
           file.head=insertion(file.head,TabKey,SizeTabKey,SizeTabRest);
        break;
        
        case 2:
            // Delete function
            printf("Enter the key to Delete Element (Keys does NOT contain spaces) and a maximum size of %d: ", KEY_MAX_SIZE - 1);
            scanf("%16s", buffer);

            printf("Do you want to delete element definitively \n");
            printf("1- yes (Physical Delete)\n");
            printf("2- no (Logical Delete) \n");
            
            scanf("%hu", &answer);
            if(answer == 1){
                DeleteElementPhysique(&file);
            }
            else if(answer == 2)
            {
                DeleteElementLogique(&file);
            }
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
            fBlock *PTR=file.head;
            int s=0;
            while(PTR!=NULL){
               
                printf("\nBlock %d: \n et usedSpace:%d\n\n",s,PTR->data->header.usedSpace);
                for(int i=0;i<PTR->data->header.usedSpace;i++){
                    printf("%c",PTR->data->tab[i]);
                    if(PTR->data->tab[i]=='#'){printf("\n");}
                }
                PTR=PTR->next;
                s++;
            }
            
            
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