#include "redmcsb_f0091_strchr_pc34_compat.h"

#include <stddef.h>

char *redmcsb_f0091_strchr_pc34_compat(char *string, char character)
{
    char current;

    if (!string) return NULL;
    /* STRING.C:70-73 stops at NUL before checking it against the requested
     * character, so the terminator itself is never returned as a match. */
    while ((current = *string) != '\0' && current != character) {
        ++string;
    }
    return current != '\0' ? string : NULL;
}
