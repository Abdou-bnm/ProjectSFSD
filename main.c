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
        if(Index.tab[startIndex].isDeletedLogically)
            return false;
    
    unsigned short median = (startIndex + endIndex) / 2;
    int strcmpResult = strncmp(key, Index.tab[median].key, KEY_MAX_SIZE);

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
    if(Index.IndexSize == 0)                  return false;
    return __recuSearch(0, Index.IndexSize - 1, buffer);
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
    if(j>=Index.IndexSize){
        PtrF=Index.tab[j-1].blockAddress;
   
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
        }
    }
    else{
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
    }
}
return(head);
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
            int i=0;
            char c=buffer[0],TabKey[KEY_MAX_SIZE],TabRest[BUFFER_MAX_SIZE];
            while(( c != 0) && (i < KEY_MAX_SIZE)){
                TabKey[i]=buffer[i];
                i++;
                c=buffer[i];
            }
            TabKey[i]=0; // "\0"
            buffer[i]='\0';
            int SizeTabKey=strlen(TabKey);
           
           //insertion of the rest of the struct
            printf("enter the rest of the structs");
            fgets(buffer+SizeTabKey+1,BUFFER_MAX_SIZE-SizeTabKey-3,stdin); 
            i=SizeTabKey+1;
            c=buffer[i];
            buffer[i]='\0';
            buffer[i+1]='\0';
            int SizeBuffer=strlen(buffer);
            for(int i=SizeTabKey+1;i<SizeBuffer;i++){
                TabRest[i]=buffer[i];
                i++;
            }

            int SizeTabRest = strlen(TabRest);
            file.head=insertion(file.head,TabKey,SizeTabKey,SizeTabRest);
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