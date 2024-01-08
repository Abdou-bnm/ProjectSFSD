#include "structs.h"

int main(int argc, char const *argv[]) {
    unsigned short answer;                                  // Used to get user's answers
    file file;
    memset(&file, 0, sizeof(file));
    
    printf("Do you want to create a new file?\n");
    printf("1- yes\n");
    printf("2- no\n");
    printf("Your Answer? : ");
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
        printf("Your Answer? : ");
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
            char TabKey[KEY_MAX_SIZE],TabRest[BUFFER_MAX_SIZE];
            int N=strlen(buffer);
            for(int i=0;i<N;i++){
                TabKey[i]=buffer[i];
            }
            TabKey[i]='\0'; // "\0"
            buffer[i]='a';
            int k=i;
            int SizeTabKey=strlen(TabKey);

           //insertion of the rest of the struct
            printf("enter the rest of the structs");
            fgets(buffer+SizeTabKey+1,BUFFER_MAX_SIZE-SizeTabKey-3,stdin); 
            buffer[i]='\0';
            buffer[i+1]='\0';
            int SizeBuffer=strlen(buffer);
            buffer[k]='\0';

            for(int i=SizeTabKey+1;i<SizeBuffer;i++){
                TabRest[i]=buffer[i];
            }

            int SizeTabRest = strlen(TabRest);
            file.head=insertion(file.head,TabKey,SizeTabKey,SizeTabRest);
        break;
        
        case 2:
            // Delete function
            printf("Enter the key to Delete Element (Keys does NOT contain spaces) and a maximum size of %d: ", KEY_MAX_SIZE - 1);
            scanf("%16s", buffer);
            getchar();

            printf("Do you want to delete element definitively \n");
            printf("1- yes (Physical Delete)\n");
            printf("2- no (Logical Delete) \n");
            printf("Your Answer? : ");
            scanf("%hu", &answer);
            printf("Deletion Element...");
            printf("\n-------------------------------------------\n");
            if(answer == 1){
                DeleteElementPhysique(&file);
            }
            else if(answer == 2)
            {
                DeleteElementLogique(&file);
            }
            break;

        case 3:
            printf("Searching in the file...");
            printf("\n-------------------------------------------\n");
            printf("Enter the key to search (Keys does NOT contain spaces) and a maximum size of %d: ", KEY_MAX_SIZE - 1);
            scanf("%16s", buffer);
            if(searchElement())
                printf("Element exists.\n");
            else
                printf("Element doesn't exist.\n");
            
            break;

        case 4:
            printf("Printing the file...");
            printf("\n-------------------------------------------\n");
            printFile(file);
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
