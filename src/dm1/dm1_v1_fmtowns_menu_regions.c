#include "dm1_v1_fmtowns_menu_regions.h"

/* Source-locked region-record lookups for the FM Towns DM1 menu.
 * Every field below is a byte-verified value from the hash-verified
 * HMA-240 English EDM.EXP; do not modify without matching evidence
 * in parity-evidence/dm1_fmtowns_region_table.md. */

/* Block 1 (head pointer 0x28e78) — 17 records at offsets
 * 0x28e80..0x28f00. Byte-exact transcription from the disassembly
 * evidence's table (regions 1..17, ID = index + 1). */
const DM1_V1_FmtownsRegionRecord
dm1_v1_fmtowns_region_block_1[DM1_V1_FMTOWNS_REGION_BLOCK_1_COUNT] = {
    /* ID  1 @ 0x28e80 */ { 9,  0, 320, 200 },
    /* ID  2 @ 0x28e88 */ { 1,  1,   0,   0 },
    /* ID  3 @ 0x28e90 */ { 9,  0, 224, 136 },
    /* ID  4 @ 0x28e98 */ { 1,  3,   0,   0 },
    /* ID  5 @ 0x28ea0 */ { 10, 2,   0,   0 },
    /* ID  6 @ 0x28ea8 */ { 10, 4,   0,   0 },
    /* ID  7 @ 0x28eb0 */ { 1,  3,   0,  33 },
    /* ID  8 @ 0x28eb8 */ { 9,  2,  87,  45 },
    /* ID  9 @ 0x28ec0 */ { 3,  8, 319, 168 },
    /* ID 10 @ 0x28ec8 */ { 9,  2,  87,  45 },
    /* ID 11 @ 0x28ed0 */ { 2, 10, 319,  77 },
    /* ID 12 @ 0x28ed8 */ { 9,  2,  87,  33 },
    /* ID 13 @ 0x28ee0 */ { 3, 12, 319,  74 },
    /* ID 14 @ 0x28ee8 */ { 9,  2, 320,  31 },
    /* ID 15 @ 0x28ef0 */ { 4, 14,   0, 199 },
    /* ID 16 @ 0x28ef8 */ { 9,  2,  87,   6 },
    /* ID 17 @ 0x28f00 */ { 1, 16, 233,  33 },
};

int dm1_v1_fmtowns_region_lookup_pc34(
    uint16_t region_id, DM1_V1_FmtownsRegionRecord *out) {
    if (!out) return 0;
    if (region_id < DM1_V1_FMTOWNS_REGION_BLOCK_1_ID_MIN ||
        region_id > DM1_V1_FMTOWNS_REGION_BLOCK_1_ID_MAX) {
        return 0;
    }
    *out = dm1_v1_fmtowns_region_block_1[region_id -
                                         DM1_V1_FMTOWNS_REGION_BLOCK_1_ID_MIN];
    return 1;
}

int dm1_v1_fmtowns_region_menu_panel_pc34(DM1_V1_FmtownsRegionRecord *out) {
    if (!out) return 0;
    out->type   = DM1_V1_FMTOWNS_REGION_MENU_PANEL_TYPE;
    out->parent = DM1_V1_FMTOWNS_REGION_MENU_PANEL_PARENT;
    out->a      = DM1_V1_FMTOWNS_REGION_MENU_PANEL_WIDTH_PX;
    out->b      = DM1_V1_FMTOWNS_REGION_MENU_PANEL_HEIGHT_PX;
    return 1;
}

int dm1_v1_fmtowns_region_menu_clear_area_pc34(
        DM1_V1_FmtownsRegionRecord *out) {
    if (!out) return 0;
    out->type   = DM1_V1_FMTOWNS_REGION_MENU_CLEAR_AREA_TYPE;
    out->parent = DM1_V1_FMTOWNS_REGION_MENU_CLEAR_AREA_PARENT;
    out->a      = DM1_V1_FMTOWNS_REGION_MENU_CLEAR_AREA_ANCHOR_X;
    out->b      = DM1_V1_FMTOWNS_REGION_MENU_CLEAR_AREA_ANCHOR_Y;
    return 1;
}

int dm1_v1_fmtowns_region_scale_coord_pc34(int source, int scale) {
    /* EDM.EXP GET_SCL_COORD (0x1942c):
     *   movsx ecx, word ptr [ebx+4]    ; source (parent size)
     *   movzx esi, si                  ; scale (zero-extended arg)
     *   mov   eax, esi
     *   imul  eax, ecx                 ; source * scale
     *   cdq
     *   mov   ecx, 0x2710              ; 10000
     *   idiv  ecx                      ; eax = source * scale / 10000
     * The fast path when scale == 10000 skips the divide entirely
     * (jne branch at 0x19489), returning the parent's size verbatim.
     * A post-conversion `<= 0` test then rejects the region. */
    long long product;
    int result;
    if (scale == 10000) return source;
    product = (long long)source * (long long)scale;
    result = (int)(product / 10000);
    return result;
}
