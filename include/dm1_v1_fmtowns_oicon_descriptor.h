#ifndef DM1_V1_FMTOWNS_OICON_DESCRIPTOR_H
#define DM1_V1_FMTOWNS_OICON_DESCRIPTOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked FM Towns DM1 OICON descriptor kind table.
 *
 * DRAW_ICN_BUTTON at 0x44f0 in EDM.EXP tests byte 0 of each 6-byte
 * OICON descriptor to decide whether an icon dispatches through the
 * generic LOAD_ICON path (kind == 0) or the THING_ICON path (kind
 * != 0). Descriptor base is 0x224db; stride is 6 bytes; the
 * effective address is:
 *
 *   [0x224db + oicon_index * 6]
 *
 * where `oicon_index` is the value returned by OICON_INDEX (0x2a50)
 * after masking bits 0x3c00 of the caller's icon index and mapping
 * through the atlas category threshold table.
 *
 * The 224-byte table shipped below is the exact byte-0-per-record
 * extraction from the shipping HMA-240 English EDM.EXP: 77
 * non-zero entries (THING-classified icons) and 147 zero entries
 * (generic LOAD_ICON icons). Recovered 2026-08-07 by reading the
 * flat data segment; matches DRAW_ICN_BUTTON's dispatch semantics.
 */

#define DM1_V1_FMTOWNS_OICON_KIND_COUNT   224

extern const uint8_t
    dm1_v1_fmtowns_oicon_kind[DM1_V1_FMTOWNS_OICON_KIND_COUNT];

/* Return the descriptor kind byte for `oicon_index`, or 0xff if the
 * index is out of range. The distinction between 0 (generic) and
 * non-zero (THING) is authoritative; specific non-zero values
 * classify the thing category (creatures 1..~40, ornaments 100+,
 * doors 128+, etc.) but consumers must not synthesise mappings —
 * a downstream user is only permitted to check `!= 0` for dispatch
 * or compare against a specific known kind byte recovered from the
 * same disassembly. */
uint8_t dm1_v1_fmtowns_oicon_kind_at_pc34(uint16_t oicon_index);

/* Return non-zero iff the descriptor at `oicon_index` selects the
 * THING_ICON dispatch (kind != 0). Fails closed for out-of-range. */
int dm1_v1_fmtowns_oicon_is_thing_pc34(uint16_t oicon_index);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_OICON_DESCRIPTOR_H */
