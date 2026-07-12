/*
 * Source-locked F0093 palette-ownership receipt for CSB V1 PC 3.4.
 *
 * This module is intentionally a receipt/mapping boundary. It does not
 * invent a palette, modify a renderer palette, or draw a creature.
 */
#ifndef FIRESTAFF_CSB_V1_F0093_REPLACEMENT_PALETTE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0093_REPLACEMENT_PALETTE_PC34_COMPAT_H

#include <stdint.h>

#include "memory_dungeon_dat_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_F0093_CREATURE_COUNT_PC34 27u
#define CSB_V1_F0093_REPLACEMENT_SET_COUNT_PC34 13u
#define CSB_V1_F0093_DUNGEON_PALETTE_ROW_COUNT_PC34 6u

typedef struct {
    /* ReDMCSB DEFS.H:2013-2019: low nibble is colour 9, high is colour 10. */
    uint8_t replacement_color_set_indices;
} CSB_V1_F0093CreatureAspectPc34;

typedef struct {
    uint16_t dungeon_view_rgb[CSB_V1_F0093_DUNGEON_PALETTE_ROW_COUNT_PC34];
    uint8_t d2_replacement_color;
    uint8_t d3_replacement_color;
} CSB_V1_F0093ReplacementColorSetPc34;

typedef struct {
    const CSB_V1_F0093CreatureAspectPc34 *creature_aspects;
    uint32_t creature_aspect_count;
    const CSB_V1_F0093ReplacementColorSetPc34 *replacement_sets;
    uint32_t replacement_set_count;
} CSB_V1_F0093Graphics558ReceiptPc34;

typedef struct {
    int assigned;
    uint8_t source_creature_type;
    uint8_t replacement_set_index;
    CSB_V1_F0093ReplacementColorSetPc34 values;
} CSB_V1_F0093PaletteOwnerPc34;

typedef struct {
    /* F0093 restores the base palette before this ordered ownership pass.
     * `assigned == 0` means that base palette remains authoritative. */
    CSB_V1_F0093PaletteOwnerPc34 palette_9;
    CSB_V1_F0093PaletteOwnerPc34 palette_10;
} CSB_V1_F0093ReplacementPaletteReceiptPc34;

/* Returns 1 only when the loaded map and Graphic 558 receipt are bounded and
 * every nonzero selector resolves to a loaded replacement set. The map's
 * allowed-creature order is preserved, so its final writer owns each slot.
 */
int csb_v1_f0093_build_replacement_palette_receipt_pc34(
    const struct DungeonMapDesc_Compat *loaded_map,
    const CSB_V1_F0093Graphics558ReceiptPc34 *loaded_graphics,
    CSB_V1_F0093ReplacementPaletteReceiptPc34 *out_receipt);

/* ReDMCSB DUNVIEW.C G0219:1625-1651 and G0220:1705-1720. Supplies the
 * complete PC CSB Graphic 558 aspect/replacement-set table used by F0093.
 * The caller still must provide the loaded current-map order. */
int csb_v1_f0093_pc34_graphics558_receipt(
    CSB_V1_F0093Graphics558ReceiptPc34 *out_graphics);

const char *csb_v1_f0093_replacement_palette_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0093_REPLACEMENT_PALETTE_PC34_COMPAT_H */
