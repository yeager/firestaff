#ifndef THERON_V1_TRACK02_TEXT_DECODE_H
#define THERON_V1_TRACK02_TEXT_DECODE_H

#include <stddef.h>
#include <stdint.h>

#define THERON_TEXT_MAX_STRINGS  256
#define THERON_TEXT_MAX_LENGTH   256
#define THERON_TEXT_END_MARKER   31

typedef struct {
    unsigned int count;
    /* The raw codec can be inspected while the original HuC6280 text
     * consumer is unresolved.  These fields never authorize production text
     * publication. */
    unsigned int unresolved_control_codes;
    int diagnostic_only;
    char strings[THERON_TEXT_MAX_STRINGS][THERON_TEXT_MAX_LENGTH];
} Theron_TextBlock;

int theron_v1_track02_text_decode(
    const uint16_t *codons,
    unsigned int codon_count,
    Theron_TextBlock *out);

#endif
