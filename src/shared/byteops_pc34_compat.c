#include "byteops_pc34_compat.h"

#include <string.h>

void F0007_MAIN_CopyBytes(char *source, char *destination, long byte_count)
{
    if (source == NULL || destination == NULL || byte_count <= 0) return;

    /* CopyBytes preserves the original overlap-safe byte movement contract. */
    memmove(destination, source, (size_t)byte_count);
}

void F0008_MAIN_ClearBytes(char *buffer, unsigned long byte_count)
{
    if (buffer == NULL || byte_count == 0UL) return;

    memset(buffer, 0, (size_t)byte_count);
}
