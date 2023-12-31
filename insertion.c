#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "structs.h"
file File;
IndexType Index;

block* allocBlock(){
    for(int i = 0; i < 16; i++)
        if(MS[i].isUsed == 0){
            MS[i].isUsed = 1;
            return &MS[i];
        }
    
    return NULL; 
}


fBlock *insertion(char TabKey[KEY_MAX_SIZE],int SizeTabKey,int SizeTabRest){

int SizeTabRest=strlen(buffer);//pour avoir la taille des reste des champs
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
    Index.tab[0].blockAddress=PtrF->data;
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

    while(j<Index.IndexSize && strcmpn(TabKeyIndex,TabKey,16)<0){
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
            Index.tab[j].blockAddress=PtrF->data;
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
            PtrF->next=allocBlock();
            PtrF=PtrF->next; //deplacer le ptr
            PtrF->next=NULL;
            
           //inserer dans l'indexe
            Index.tab[j].key=&(PtrF->data->tab[0]);
            Index.tab[j].endAddress=(Index.tab[j].key + SizeTabKey ); 
            Index.tab[j].blockAddress=PtrF->data;
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
            PtrF->next=allocBlock();
            PtrF=PtrF->next;
            PtrF->data->header.NbStructs=1;//je ne suis pas sure que ca soit 1??
            PtrF->data->header.usedSpace=SizeTabRest +SizeTabKey +3; //pas sure aussi
        }
        fBlock *P=PtrF,*R=Q;
        char *Qtab=&(Index.tab[j].key),*Ptab=Qtab + Q->data->header.usedSpace;
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