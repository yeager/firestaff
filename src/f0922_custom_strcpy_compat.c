#include "f0922_custom_strcpy_compat.h"

#include <stdint.h>

char *f0922_custom_strcpy_compat(char *destination,
                                 size_t destination_capacity,
                                 const char *source)
{
    size_t source_length = 0u;
    size_t index;

    if (!destination || !source || destination_capacity == 0u) {
        return NULL;
    }

    while (source[source_length] != '\0') {
        if (source_length == SIZE_MAX - 1u) {
            return NULL;
        }
        ++source_length;
    }

    if (source_length >= destination_capacity) {
        return NULL;
    }

    for (index = 0u; index <= source_length; ++index) {
        destination[index] = source[index];
    }

    return destination;
}
