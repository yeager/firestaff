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

/* F0093 stores palette-change values in platform-native colour-index
 * encodings.  The indexed M11 renderer uses the decoded 0..15 result, so
 * select the original table family rather than applying PC values to Atari
 * CSB media. */
typedef enum {
    CSB_V1_F0093_PALETTE_PROFILE_PC34 = 0,
    CSB_V1_F0093_PALETTE_PROFILE_ATARI_ST = 1,
    /* ReDMCSB DUNVIEW.C:G2025/F0695 version-3 table used by the Amiga
     * and FM Towns ports.  It shares the decoded indexed values with the
     * Atari table, but remains a separate source-family selection. */
    CSB_V1_F0093_PALETTE_PROFILE_VERSION3_F0695 = 2
} CSB_V1_F0093PaletteProfilePc34;

/* Returns 1 only when the loaded map and Graphic 558 receipt are bounded and
 * every nonzero selector resolves to a loaded replacement set. The map's
 * allowed-creature order is preserved, so its final writer owns each slot.
 */
int csb_v1_f0093_build_replacement_palette_receipt_pc34(
    const struct DungeonMapDesc_Compat *loaded_map,
    const CSB_V1_F0093Graphics558ReceiptPc34 *loaded_graphics,
    CSB_V1_F0093ReplacementPaletteReceiptPc34 *out_receipt);

/* Apply F0093's final map-ordered owners to a derived D2/D3 creature
 * palette.  It first restores slot 9 from replacement set 8 and slot 10
 * from set 12, exactly as DUNVIEW.C does, so an unowned slot cannot inherit
 * the preceding creature's local remap. `depth_index >= 2` selects G0221
 * (D3); lower depths select G0222 (D2). This is a renderer adapter: it
 * changes only slots 9 and 10 and consumes the real current-map
 * allowed-creature order. */
int csb_v1_f0093_apply_replacement_palette_pc34(
    const struct DungeonMapDesc_Compat *loaded_map,
    int depth_index,
    uint8_t palette[16]);

/* Variant-aware form used by the native renderer.  The legacy spelling
 * above retains PC 3.4 semantics for focused compatibility callers. */
int csb_v1_f0093_apply_replacement_palette_for_profile_pc34(
    const struct DungeonMapDesc_Compat *loaded_map,
    int depth_index,
    CSB_V1_F0093PaletteProfilePc34 profile,
    uint8_t palette[16]);

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
