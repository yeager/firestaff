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
        for (int k = 0; k < 3; k++) {
            char c = decode_letter(w, k);
            if (c == 0) {
                out->strings[str_idx][char_idx] = '\0';
                str_idx++;
                char_idx = 0;
                break;
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
