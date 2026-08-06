#include "dm1_v1_fmtowns_menu_regions.h"

/* Source-locked region-record lookups for the FM Towns DM1 menu.
 * Every field below is a byte-verified value from the hash-verified
 * HMA-240 English EDM.EXP; do not modify without matching evidence
 * in parity-evidence/dm1_fmtowns_region_table.md. */

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
