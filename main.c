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


fBlock *insertion(file File,char TabKey[KEY_MAX_SIZE],int SizeTabKey,int SizeTabRest){

char TabKeyIndex[KEY_MAX_SIZE];
File.head->data=allocBlock(); //creer le premier block
fBlock *PtrF=File.head;//initialiser un ptr sur le block courrent 
PtrF->next=NULL;

//COMMENCER L'INSERTION
if (Index.IndexSize==0){
    //mettre le header a jour
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
    Index.tab[0].endAddress=&(PtrF->data->tab[SizeTabKey]);
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
       if(BUFFER_MAX_SIZE - PtrF->data->header.usedSpace >= SizeTabKey+SizeTabRest+3){
   
           //inserer dans l'indexe
            Index.tab[j].key=(Index.tab[j-1].endAddress + SizeTabRest + 4); //supposant que EndAderess se trouve au niveau du dernier caractere de la cle(+3 pour les 3 "\0" +1 pour se positionner sur la nouvelle case)
            Index.tab[j].endAddress=(Index.tab[j].key + SizeTabKey ); 
            Index.tab[j].blockAddress=PtrF;
            Index.tab[j].isDeletedLogically=0;
   
            //inserer dans le bloc
            //placer la chaine(stockée en haut dans la chaine buffer) dans le bloc
            PtrStart=Index.tab[j].key;
            PtrEnd=Index.tab[j].endAddress;
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
            PtrF=PtrF->next; //deplacer le ptr
            PtrF->next=NULL;
            
           //inserer dans l'indexe
            Index.tab[j].key=&(PtrF->data->tab[0]);
            Index.tab[j].endAddress=(Index.tab[j].key + SizeTabKey ); 
            Index.tab[j].blockAddress=PtrF;
            Index.tab[j].isDeletedLogically=0;
   
            //inserer dans le bloc
            //placer la chaine(stockée en haut dans la chaine buffer) dans le bloc
            //placer la chaine dans le bloc
            //a revoir
            PtrStart=Index.tab[j].key;
            PtrEnd=Index.tab[j].endAddress;
            i=0;
            while(PtrStart!=PtrEnd){
                *PtrStart=buffer[i];
                i++;
                PtrStart++;
            }  
        }
    }
    else{
        //traitement a l'interieur du block
        fBlock *Q =Index.tab[j].blockAddress ;//pointeur de parcour sur les blocks(il commence par le block du 1er element superieur)
        if(BUFFER_MAX_SIZE - PtrF->data->header.usedSpace < SizeTabRest +SizeTabKey +3){
            PtrF->next->data=allocBlock();
            PtrF->next->next=NULL;
            PtrF=PtrF->next;
            PtrF->data->header.NbStructs=1;//je ne suis pas sure que ca soit 1??
            PtrF->data->header.usedSpace=SizeTabRest +SizeTabKey +3; //pas sure aussi
        }
        fBlock *P=PtrF,*R=Q;
        char *Qtab=Index.tab[j].key ,*Ptab=Qtab + Q->data->header.usedSpace;
        //traitement decalage 
        while(Q!=P){
            while(R->next!=P){R=R->next;}
            unsigned short N=P->data->header.usedSpace;//limite de la boucle
            for(int k =0;k<(P->data->header.usedSpace);k++){
                P->data->tab[N]=P->data->tab[N-1];
                N--;
            }
            P=R;
            R=Q; 
        }   
        while(Ptab!=Qtab){
            *Ptab=*(Ptab-1);
            Ptab=Ptab-1;
        }
        //insertion dans le bloc
        for(int i=0;i< SizeTabRest +SizeTabKey +3;i++){
            *(Qtab+i)=buffer[i];
        }
    
        //traitement de l'indexe
         // decaler les elements de l'indexe
       int i=Index.IndexSize;
        while(i>=j){//pour l'egalite pas sure (a revoir)
            Index.tab[i]=Index.tab[i-1];
            i--;
        }
        Index.tab[j].key=Qtab;
        Index.tab[j].endAddress=Qtab + SizeTabKey;
        Index.tab[j].blockAddress=Q;
        Index.tab[j].isDeletedLogically=0;
    }
}
return(File.head);
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
            file.head=insertion(file,TabKey,SizeTabKey,SizeTabRest);
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