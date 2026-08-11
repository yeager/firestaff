#include "theron_v1_track02_text_decode.h"
#include <string.h>

static char decode_letter(uint16_t codon, int pos) {
    int shift;
    switch (pos) {
        case 0: shift = 10; break;
        case 1: shift = 5;  break;
        case 2: shift = 0;  break;
        default: return 0;
    }
    int val = (codon >> shift) & 0x1F;
    if (val == THERON_TEXT_END_MARKER) return 0;
    if (val <= 25) return (char)('a' + val);
    if (val == 26) return ' ';
    if (val == 27) return '{';
    if (val == 28) return '}';
    if (val == 29) return '\n';
    if (val == 30) return '.';
    return '?';
}

static uint8_t decode_glyph_value(uint16_t codon, int pos) {
    int shift;
    switch (pos) {
        case 0: shift = 10; break;
        case 1: shift = 5;  break;
        case 2: shift = 0;  break;
        default: return THERON_TEXT_END_MARKER;
    }
    return (uint8_t)((codon >> shift) & 0x1F);
}

int theron_v1_track02_text_decode(
    const uint16_t *codons,
    unsigned int codon_count,
    Theron_TextBlock *out)
{
    if (!codons || !out) return -1;
    memset(out, 0, sizeof(*out));

    unsigned int str_idx = 0;
    unsigned int char_idx = 0;
    unsigned int i = 0;

    while (i < codon_count && str_idx < THERON_TEXT_MAX_STRINGS) {
        uint16_t w = codons[i++];
        int string_ended = 0;
        for (int k = 0; k < 3; k++) {
            uint8_t glyph = decode_glyph_value(w, k);
            char c = decode_letter(w, k);
            if (out->raw_glyph_count < THERON_TEXT_MAX_RAW_GLYPHS)
                out->raw_glyphs[out->raw_glyph_count++] = glyph;
            /* Keep consuming the rest of this packed word after an end
             * marker.  Those padding/control values are part of the real
             * source stream even though they do not belong to the decoded
             * diagnostic string. */
            if (string_ended) continue;
            if (c == 0) {
                out->strings[str_idx][char_idx] = '\0';
                str_idx++;
                char_idx = 0;
                string_ended = 1;
                continue;
            }
            /* The 5-bit stream's brace values are preserved for reverse-
             * engineering output, but their original text/UI control
             * meaning is not proven by the current Track 02 consumer
             * evidence.  Keep the complete raw decode diagnostic-only. */
            if (c == '{' || c == '}') {
                out->unresolved_control_codes++;
                out->diagnostic_only = 1;
            }
            if (char_idx < THERON_TEXT_MAX_LENGTH - 1)
                out->strings[str_idx][char_idx++] = c;
        }
    }

    out->count = str_idx;
    return 0;
}
