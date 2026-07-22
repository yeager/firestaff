#include "redmcsb_f0009_f0010_spaced_writes_pc34_compat.h"

#include <stddef.h>

static ptrdiff_t f0010_word_stride(int16_t spacing)
{
    const ptrdiff_t byte_stride = (ptrdiff_t)spacing;

    /* BASE.C applies an arithmetic right shift before indexing int16_t words. */
    return byte_stride >= 0 ? byte_stride / 2 : -(((-byte_stride) + 1) / 2);
}

void F0009_MAIN_WriteSpacedBytes(
    char *buffer,
    uint16_t byte_count,
    char byte_value,
    int16_t spacing)
{
    ptrdiff_t byte_index = 0;

    for (uint16_t counter = 0; counter < byte_count; ++counter) {
        buffer[byte_index] = byte_value;
        byte_index += (ptrdiff_t)spacing;
    }
}

void F0010_MAIN_WriteSpacedWords(
    int16_t *buffer,
    uint16_t word_count,
    int16_t word_value,
    int16_t spacing)
{
    const ptrdiff_t word_stride = f0010_word_stride(spacing);
    ptrdiff_t word_index = 0;

    for (uint16_t counter = 0; counter < word_count; ++counter) {
        buffer[word_index] = word_value;
        word_index += word_stride;
    }
}
