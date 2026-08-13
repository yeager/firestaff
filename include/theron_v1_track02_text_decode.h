#ifndef THERON_V1_TRACK02_TEXT_DECODE_H
#define THERON_V1_TRACK02_TEXT_DECODE_H

#include <stddef.h>
#include <stdint.h>

#define THERON_TEXT_MAX_STRINGS  256
#define THERON_TEXT_MAX_LENGTH   256
#define THERON_TEXT_END_MARKER   31
#define THERON_TEXT_MAX_RAW_GLYPHS (1024u * 3u)

typedef enum {
    THERON_TEXT_TOKEN_RAW = 0,
    THERON_TEXT_TOKEN_CONTROL = 1,
    THERON_TEXT_TOKEN_END = 2
} Theron_TextTokenKind;

typedef struct {
    uint8_t value;
    uint16_t word_index;
    uint8_t packed_slot;
    Theron_TextTokenKind kind;
} Theron_TextToken;

typedef struct {
    unsigned int count;
    /* The raw codec can be inspected while the original HuC6280 text
     * consumer is unresolved.  These fields never authorize production text
     * publication. */
    unsigned int unresolved_control_codes;
    int diagnostic_only;
    /* Lossless 5-bit values from the authenticated codon stream, including
     * end/control values.  The original HuC6280 consumer still owns their
     * meaning; retaining them prevents the diagnostic ASCII view from
     * destroying information needed by that consumer. */
    unsigned int raw_glyph_count;
    uint8_t raw_glyphs[THERON_TEXT_MAX_RAW_GLYPHS];
    /* Positional view of the same stream. It retains source word/slot
     * boundaries for a future HuC6280 consumer join; kind does not assign
     * the original control-code meaning. */
    unsigned int token_count;
    Theron_TextToken tokens[THERON_TEXT_MAX_RAW_GLYPHS];
    char strings[THERON_TEXT_MAX_STRINGS][THERON_TEXT_MAX_LENGTH];
} Theron_TextBlock;

int theron_v1_track02_text_decode(
    const uint16_t *codons,
    unsigned int codon_count,
    Theron_TextBlock *out);

#endif
