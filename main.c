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
            break;

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
                printf("Enter the key to the element (%hu max characters, NO SPACES and CASE SENSITIVE): ", KEY_MAX_SIZE - 1);
                scanf("%16s", buffer);
                getchar();
                unsigned short keySize = strlen(buffer) + 1, element_Size;

                printf("enter the rest of the structs (max size of %hu due to lack of overlap, spaces allowed): ", BUFFER_MAX_SIZE - keySize);
                fgets(buffer + keySize, BUFFER_MAX_SIZE - keySize, stdin);

                element_Size = keySize + strlen(buffer + keySize);
                buffer[element_Size - 1] = '\0';

                //system("clear");
                insert(&file, element_Size);
                
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
                        DeleteElementPhysique(&file, file.RFile);
                        break;

                    case 2:
                        DeleteElementLogique(file.RFile);
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
        StockHeaderecFile(file.RFile, &file);
        if(fclose(file.RFile)){
            fprintf(stderr, "ERROR! [fclose in main]: returned a non-zero value.\nExiting...\n");
            exit(EXIT_FAILURE);
        }
        return 0;
}