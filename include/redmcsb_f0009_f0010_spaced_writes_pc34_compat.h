#ifndef REDMCSB_F0009_F0010_SPACED_WRITES_PC34_COMPAT_H
#define REDMCSB_F0009_F0010_SPACED_WRITES_PC34_COMPAT_H

#include <stdint.h>

/* ReDMCSB BASE.C F0009/F0010 spaced buffer writers. */
void F0009_MAIN_WriteSpacedBytes(
    char *buffer,
    uint16_t byte_count,
    char byte_value,
    int16_t spacing);

/* spacing is measured in bytes, as in the original F0010 contract. */
void F0010_MAIN_WriteSpacedWords(
    int16_t *buffer,
    uint16_t word_count,
    int16_t word_value,
    int16_t spacing);

#endif
