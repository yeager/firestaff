#include "redmcsb_f0089_strncpy.h"

char *redmcsb_f0089_strncpy(char *destination,
                             const char *source,
                             int16_t count)
{
    char *result = destination;

    while (count > 0) {
        --count;
        if (!(*destination++ = *source++)) {
            break;
        }
    }

    return result;
}
