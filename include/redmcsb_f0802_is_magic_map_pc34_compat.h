/*
 * ReDMCSB PANEL.C F0802_IsMagicMap, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0802_IS_MAGIC_MAP_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0802_IS_MAGIC_MAP_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0802_THING_TYPE_SCROLL = 7,
    REDMCSB_F0802_TEXT_TYPE_SCROLL = 2,
    REDMCSB_F0802_DECODE_EVEN_IF_INVISIBLE = UINT16_C(0x8000)
};

typedef void (*redmcsb_f0802_decode_text_pc34_compat_fn)(
    char *decoded_text,
    uint16_t text_string_thing_index,
    uint16_t text_type_and_flags);

/*
 * Mirrors PANEL.C:519-531. scroll_text_string_thing_indices is the original
 * C07 scroll-data table's TextStringThingIndex field, indexed by M013_INDEX.
 * The supplied decoder is the source F0168_DUNGEON_DecodeText boundary.
 */
int redmcsb_f0802_is_magic_map_pc34_compat(
    uint16_t thing,
    const uint16_t *scroll_text_string_thing_indices,
    const char *magic_map_scroll_text,
    redmcsb_f0802_decode_text_pc34_compat_fn decode_text);

const char *redmcsb_f0802_is_magic_map_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
