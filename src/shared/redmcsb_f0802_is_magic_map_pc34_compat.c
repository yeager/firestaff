#include "redmcsb_f0802_is_magic_map_pc34_compat.h"

#include <string.h>

int redmcsb_f0802_is_magic_map_pc34_compat(
    uint16_t thing,
    const uint16_t *scroll_text_string_thing_indices,
    const char *magic_map_scroll_text,
    redmcsb_f0802_decode_text_pc34_compat_fn decode_text)
{
    char scroll_text[200];

    if (((thing & UINT16_C(0x3c00)) >> 10) ==
        REDMCSB_F0802_THING_TYPE_SCROLL) {
        decode_text(scroll_text,
                    scroll_text_string_thing_indices[thing & UINT16_C(0x03ff)],
                    REDMCSB_F0802_TEXT_TYPE_SCROLL |
                        REDMCSB_F0802_DECODE_EVEN_IF_INVISIBLE);
        if (strcmp(scroll_text, magic_map_scroll_text) == 0) {
            return 1;
        }
    }
    return 0;
}

const char *redmcsb_f0802_is_magic_map_source_evidence_pc34(void)
{
    return "ReDMCSB PANEL.C:519-531 (PC 3.4 I34E/I34M route): "
           "F0802 accepts only a C07 scroll whose F0168-decoded text, "
           "using C2_TEXT_TYPE_SCROLL | MASK0x8000_DECODE_EVEN_IF_INVISIBLE, "
           "matches G2212_apc_MagicMapScrollText exactly.";
}
