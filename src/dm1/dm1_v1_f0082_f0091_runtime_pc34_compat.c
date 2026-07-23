#include "dm1_v1_f0082_f0091_runtime_pc34_compat.h"

#include <limits.h>
#include <stdint.h>

int dm1_v1_f0082_ldiv_pc34(int32_t dividend, int32_t divisor, int32_t *out_quotient)
{
    if (!out_quotient || divisor == 0 ||
        (dividend == INT32_MIN && divisor == -1)) return 0;
    *out_quotient = dividend / divisor;
    return 1;
}

int32_t dm1_v1_f0083_lmul_pc34(int32_t left, int32_t right)
{
    return (int32_t)((uint32_t)left * (uint32_t)right);
}

int dm1_v1_f0084_blockmv_pc34(void *destination, size_t destination_size,
                               const void *source, size_t source_size,
                               size_t byte_count)
{
    uint8_t *to = (uint8_t *)destination;
    const uint8_t *from = (const uint8_t *)source;
    size_t index;
    if ((!to || !from) && byte_count != 0u) return 0;
    if (byte_count > destination_size || byte_count > source_size) return 0;
    if (to == from || byte_count == 0u) return 1;
    if (to < from || to >= from + byte_count) {
        for (index = 0; index < byte_count; ++index) to[index] = from[index];
    } else {
        for (index = byte_count; index != 0u; --index) to[index - 1u] = from[index - 1u];
    }
    return 1;
}

char *dm1_v1_f0086_strcat_pc34(char *destination, size_t destination_size,
                                const char *source)
{
    size_t used = 0u;
    size_t copied = 0u;
    if (!destination || !source || destination_size == 0u) return NULL;
    while (used < destination_size && destination[used] != '\0') ++used;
    if (used == destination_size) return NULL;
    while (source[copied] != '\0') {
        if (used + copied + 1u >= destination_size) return NULL;
        destination[used + copied] = source[copied];
        ++copied;
    }
    destination[used + copied] = '\0';
    return destination;
}

int16_t dm1_v1_f0087_strcmp_pc34(const char *left, const char *right)
{
    const unsigned char *a = (const unsigned char *)left;
    const unsigned char *b = (const unsigned char *)right;
    if (!a || !b) return (int16_t)(a == b ? 0 : (a ? 1 : -1));
    while (*a != '\0' && *a == *b) { ++a; ++b; }
    return (int16_t)(*a - *b);
}

char *dm1_v1_f0088_strcpy_pc34(char *destination, size_t destination_size,
                                const char *source)
{
    size_t index = 0u;
    if (!destination || !source || destination_size == 0u) return NULL;
    do {
        if (index >= destination_size) return NULL;
        destination[index] = source[index];
    } while (source[index++] != '\0');
    return destination;
}

int dm1_v1_f0090_strlen_pc34(const char *string, size_t maximum_length,
                              int16_t *out_length)
{
    size_t length = 0u;
    if (!string || !out_length) return 0;
    while (length < maximum_length && string[length] != '\0') ++length;
    if (length == maximum_length || length > (size_t)INT16_MAX) return 0;
    *out_length = (int16_t)length;
    return 1;
}

char *dm1_v1_f0091_strchr_pc34(char *string, char character)
{
    if (!string || character == '\0') return NULL;
    while (*string != '\0' && *string != character) ++string;
    return *string == character ? string : NULL;
}

const char *dm1_v1_f0082_f0091_runtime_source_evidence_pc34(void)
{
    return "ReDMCSB DEFS.H:6898-6908 identifies F0082-F0085 as Megamax "
           "long/structure helpers and F0086-F0091 as the C runtime string "
           "family. STRING.C:6-70 provides exact F0086-F0091 PC34 game "
           "semantics. F0085/F0089 retain their established shared owners.";
}
