#include "structs.h"

int main(int argc, char const *argv[]) {
    unsigned short answer;                                  // Used to get user's answers
    file file;
    memset(&file, 0, sizeof(file));
    
    printf("Do you want to create a new file or open an existing file?\n");
    printf("1- Create a new file\n");
    printf("2- Open an existing file\n");
    printf("Your Answer? : ");
    scanf("%hu", &answer);                                  // "%hu" format specifier for unsigned short
    switch(answer){
        case 1:
            createfile(&file);
            break;
        
        case 2:
            printf("Enter the name of the file, (CASE SENSITIVE, NO spaces, 35 characters MAX): ");
            scanf("%36s", file.header.name);
            printf("Opening file \"%s\"... ", file.header.name);
            fileOpen(&file);
            printf("file opened successfully!\n");
        default:
            printf("Invalide choice.");
            goto end;
    }

    while(true){
        memset(&buffer, sizeof(buffer), 0);                 // Clear the buffer for next use.

        printf("Enter what you want to do in numbers\n");
        printf("1- Insert an element\n");
        printf("2- Delete an element\n");
        printf("3- Search for an element\n");
        printf("4- Display contents of file\n");
        printf("5- Leave\n");
        printf("Your Answer? : ");
        scanf("%hu", &answer);
        switch(answer){
            case 1:
                // Insert function
                //entrer la cle 
                printf("enter the key (%d characters MAX, NO spaces): ", KEY_MAX_SIZE - 1);
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
                printf("enter the rest of the structs: ");
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

                
                // From now on this bullshit I wrote, bidma chahinez tkamel l khadma, bach n tester la fonction ta3i, and it worked successfully.
                // Bach temchi hadi, commentit m ligne 47 (int i=0;) 7ata 71 (file.head=insertion(file.head,TabKey,SizeTabKey,SizeTabRest);).
                
                // int keySize = strlen(buffer);
                
                // printf("enter the rest of the structs: ");
                // fgets(buffer + keySize + 1, BUFFER_MAX_SIZE - keySize, stdin);
                
                // int dataSize = strlen(buffer + keySize + 1);
                // buffer[keySize + dataSize] = '\0';
                // dataSize = strlen(buffer + keySize + 1);

                // char *tmp = (char*)malloc(BUFFER_MAX_SIZE);
                // memcpy(tmp, buffer, BUFFER_MAX_SIZE);

                // RFile_insert(file.RFile, tmp, keySize + 1 + dataSize + 1);
                // free(tmp);
                
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
                printf("Deleting Element...");
                printf("\n-------------------------------------------\n");
                
                switch(answer){
                    case 1:
                        DeleteElementPhysique(&file);
                        break;

                    case 2:
                        DeleteElementLogique(&file);
                        break;

                    default:
                        printf("Invalide choice.\nExiting...\n");
                        break;
                }
                break;
                
            case 3:
                printf("Searching in the file...");
                printf("\n-------------------------------------------\n");
                printf("Enter the key to search (Keys does NOT contain spaces) and a maximum size of %d: ", KEY_MAX_SIZE - 1);
                scanf("%16s", buffer);
                if(searchElement() != -1)
                    printf("Element exists.\n");
                else
                    printf("Element doesn't exist.\n");
                break;

            case 4:
                printf("Printing the file...");
                printf("\n-------------------------------------------\n");
                printFile(file);
                break;

            case 5:
                printf("Leaving...\n");
                goto end;

            default:
                fprintf(stderr, "Invalid choice.\nExiting...\n");
                exit(EXIT_FAILURE);
                break;
        }
    }

    end:
        return 0;
}