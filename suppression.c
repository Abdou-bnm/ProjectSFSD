#include "structs.h"
// Suppression Section

void ElementShift(char **NewElementPos, char *StartCurElementPos, char *EndCurElementPos)
{
    // Shift the Element to New Pos
    while (StartCurElementPos != EndCurElementPos)
    {
        **NewElementPos = *StartCurElementPos;
        *StartCurElementPos = 0;
        *NewElementPos += sizeof(char);
        StartCurElementPos += sizeof(char);
    }
    **NewElementPos = *StartCurElementPos;
}

int CalculateSpace(char *StartEspaceAddress, char *EndEspaceAddress)
{
    return EndEspaceAddress - StartEspaceAddress;
}

// Function to update index array (Delete Element)
void UpdateIndexDelete(int IndexElementDeleted)
{
    for (int ElementIndex = IndexElementDeleted; ElementIndex < Index.IndexSize; ElementIndex++)
    {
        Index.tab[ElementIndex] = Index.tab[ElementIndex + 1];
    }

    Index.IndexSize--;
}

// Function to update File
void UpdateFileStruct(file *file)
{
    fBlock *ftmp = (*file).head;
    while ((ftmp)->next != NULL)
    {
        ftmp = ftmp->next;
    }
}

// Function to delete an element from the file (Logical)
void DeleteElementLogique(FILE* RFile)
{
    short indexElement = searchElement();
    if (indexElement == -1)
    {
        printf("\nERROR! [Searching for Element]:already deleted or not Existe ");
        return;
    }
    else
    {
        Index.tab[indexElement].isDeletedLogically = true;
        RFile_Delete(RFile, indexElement);
    }
}

// Delete in RFile
void RFile_Delete(FILE *file, short elementIndexPos)
{
    if (elementIndexPos == -1)
    { // Error handling
        fprintf(stderr, "ERROR! [searchElement in RFile_insert]: Couldn't find element in index.\nreturning...\n");
        return;
    }

    if (fseek(file, 0, SEEK_END))
    { // Error handling
        fprintf(stderr, "ERROR! [fseek 1 in RFile_Delete]: return a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    if (elementIndexPos == Index.IndexSize - 1)
    { // In case the element is the last in index, so no shifting needed
        fseek(file, -(Index.tab->endAddress - Index.tab->key), SEEK_END);
        fputc(-1, file);
        return;
    }

    // In case Not the last element in index, so there is shifting needed

    unsigned short pos = (Index.tab[elementIndexPos].filePos);
    unsigned long taille = (Index.tab[elementIndexPos].endAddress - Index.tab[elementIndexPos].key);
    char c;
    fseek(file, pos + taille, SEEK_SET);
    // Shift the remaining characters to the left
    int size;
    int i = 0;
    while ((c = fgetc(file)) != EOF)
    {
        memset(buffer, 0, sizeof(buffer)); // Clear the current buffer
        i = 1;
        size=1;
        buffer[0] = c;
        
        do  // Copy Record to Buffer
        {   
            c = fgetc(file);
            buffer[i] = c;
            i++;
        } while (c != '\n');
        
        buffer[i] = '\0';

        fseek(file,pos, SEEK_SET);
        fputs(buffer, file); //Shift the record to new pos
        pos = pos + i;
        fseek(file, ftell(file) + taille, SEEK_SET);// Jump to next Record
    }
}

// Function to delete an element from the file (Physique)
int DeleteElementPhysique(file *file,FILE *Rfile){

    // Search for the index of the element to be deleted
    short indexElementDeleted = searchElement();

    // Check if the element is not found or already deleted
    if (indexElementDeleted == -1)
    {
        printf("\nERROR! [Searching for Element]:already deleted or not Existe");
        return -1;
    }
    else
    {
        unsigned short NbElement = -1;
        int FreeSpace = 0;
        char *EndCurElementPos, *StartCurElementPos;
        char *NewElementPos = Index.tab[indexElementDeleted].key;
        block *blockAddressdataRecover = (Index.tab[indexElementDeleted].blockAddress)->data; // Save Address of Block
        for (int i = indexElementDeleted + 1; i < Index.IndexSize; i++)
        {
            // Verify if the Shift Will be in the Same Block or not
            if ((Index.tab[i].blockAddress)->data != (Index.tab[i - 1].blockAddress)->data) // Shift the element into a different block
            {
                FreeSpace = (((Index.tab[i - 1].blockAddress)->data)->header).EndAddress - NewElementPos; // Calculate the remaining free space in the block
                // Verify if FreeSpace is sufficient for the Element
                if (CalculateSpace(Index.tab[i].key, Index.tab[i].endAddress) <= FreeSpace) // FreeSpace is sufficient => Make the Element in the FreeSpace
                {
                    EndCurElementPos = Index.tab[i].endAddress;
                    StartCurElementPos = Index.tab[i].key;
                    Index.tab[i].key = NewElementPos;
                    ElementShift(&NewElementPos, StartCurElementPos, EndCurElementPos);
                    Index.tab[i].endAddress = NewElementPos;
                    *NewElementPos += 2 * sizeof(char);
                    NbElement++;
                }
                else // FreeSpace isn't sufficient => Make the Element in New Block
                {
                    if (NbElement == -1) // element deleted was in base of block and the next element have more space
                    {
                        (((Index.tab[i].blockAddress)->data)->header).NbStructs -= NbElement + 1; // Update Number of Element in New Block
                    }
                    (((Index.tab[i - 1].blockAddress)->data)->header).NbStructs += NbElement; // Update Number of Element in Current Block

                    NbElement = 0;
                    NewElementPos = blockAddressdataRecover->tab;
                    EndCurElementPos = Index.tab[i].endAddress;
                    StartCurElementPos = Index.tab[i].key;
                    if (NewElementPos == StartCurElementPos)
                    {
                        return -1; // Shift Complete (Block no need to do shift on it)
                    }
                    Index.tab[i].key = NewElementPos;
                    ElementShift(&NewElementPos, StartCurElementPos, EndCurElementPos);
                    Index.tab[i].endAddress = NewElementPos;
                    *NewElementPos += 2 * sizeof(char);
                }
            }
            else // Shift the element within the same block
            {
                EndCurElementPos = Index.tab[i].endAddress;
                StartCurElementPos = Index.tab[i].key;
                Index.tab[i].key = NewElementPos;
                ElementShift(&NewElementPos, StartCurElementPos, EndCurElementPos);
                Index.tab[i].endAddress = NewElementPos;
                *NewElementPos += 2 * sizeof(char);
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

        // Delete in the Rfile  
        RFile_Delete(Rfile,indexElementDeleted);

        printf("\nElement Deleted Successfully!\n");
    }
}