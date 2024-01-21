#include "structs.h"
// insertion Section

// A function to insert element "element" into the permanent "RFile" in the correct position and changing the position info in Index
void RFile_insert(FILE *file, char *element, unsigned long elementSize)
{
    memset(buffer, 0, sizeof(buffer));    // Clear the current buffer
    memcpy(buffer, element, elementSize); // Copy the insered element into buffer

    short elementIndexPos = searchElement(), cpt = 0;
    if (elementIndexPos == -1)
    { // Error handling
        fprintf(stderr, "ERROR! [searchElement in RFile_insert]: Couldn't find element in index.\nreturning...\n");
        return;
    }

    if (fseek(file, 0, SEEK_END))
    { // Error handling
        fprintf(stderr, "ERROR! [fseek 1 in RFile_insert]: return a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    long maxPos = ftell(file), space, err; // Last position in file
    if (maxPos == -1)
    { // Error handling
        fprintf(stderr, "ERROR! [ftell in RFile_insert]: returned -1.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    if (elementIndexPos == Index.IndexSize - 1)
    { // In case the element is the last in index, so no shifting needed
        Index.tab[elementIndexPos].filePos = maxPos;
        for (int i = 0; i < elementSize; i++)
        {
            if (buffer[i] == '\0')
            { // Add ':' and '\n' instead of '\0' to organise RFile
                if (cpt % 2)
                    fputc('\n', file);
                else
                    fputc(':', file);
                cpt++;
                continue;
            }
            fputc(buffer[i], file); // Copy contents of buffer to file
        }
        return;
    }

    // In case Not the last element in index, so there is shifting needed
    space = maxPos - Index.tab[elementIndexPos + 1].filePos; // Number of bytes that will be shifted

    char *tmp = (char *)malloc(space); // Temporary buffer to hold the shifted string
    if (tmp == NULL)
    { // Error handling
        fprintf(stderr, "ERROR! [malloc in RFile_insert]: Couldn't allocate memory for \"tmp\" variable.\nExiting...\n");
        exit(EXIT_FAILURE);
    }
    memset(tmp, 0, space); // Setting the entire tmp buffer to 0

    if (fseek(file, Index.tab[elementIndexPos + 1].filePos, SEEK_SET))
    { // Error handling
        fprintf(stderr, "ERROR! [fseek 2 in RFile_insert]: returned a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    err = fread(tmp, sizeof(char), space, file); // Read the shifted elements into tmp buffer
    if (err != space)
    { // Error handling
        fprintf(stderr, "ERROR! [fread in RFile_insert]: expected to read %li but got %li", space, err);
        exit(EXIT_FAILURE);
    }

    Index.tab[elementIndexPos].filePos = Index.tab[elementIndexPos + 1].filePos; // Changing the filePos in the Index
    Index.tab[elementIndexPos + 1].filePos += elementSize;                       // Changing the filePos of the next element in the Index

    if (fseek(file, Index.tab[elementIndexPos].filePos, SEEK_SET))
    { // Error handling
        fprintf(stderr, "ERROR! [fseek 3 in RFile_insert]: returned a non-zero value.\nExiting...\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < elementSize; i++)
    { // Add ':' and '\n' instead of '\0' to organise RFile
        if (buffer[i] == '\0')
        {
            if (cpt % 2)
                fputc('\n', file);
            else
                fputc(':', file);
            cpt++;
            continue;
        }
        fputc(buffer[i], file); // Writing the new element
    }

    err = fwrite(tmp, sizeof(char), space, file); // Writing the rest of the old elements (shifted ones)
    if (err != space)
    { // Error handling
        fprintf(stderr, "ERROR! [fwrite 2 in RFile_insert]: expected to read %li but got %li", space, err);
        exit(EXIT_FAILURE);
    }

    free(tmp); // Free the tmp buffer
}
