#include "structs.h"
/// Search Section
short __recuSearch(unsigned short startIndex, unsigned short endIndex, char *key)
{
    if (startIndex == endIndex)
        if (Index.tab[startIndex].isDeletedLogically)
            return -1;

    unsigned short median = (startIndex + endIndex) / 2;
    int strcmpResult = strncmp(key, Index.tab[median].key, KEY_MAX_SIZE);

    if (startIndex == endIndex && strcmpResult)
        return -1;

    if (!strcmpResult)
        return median;
    if (strcmpResult < 1)
        return __recuSearch(startIndex, median, key);
    else
        return __recuSearch(median + 1, endIndex, key);
}

short searchElement()
{
    if (Index.IndexSize == 0)
        return false;
    return __recuSearch(0, Index.IndexSize - 1, buffer);
}
