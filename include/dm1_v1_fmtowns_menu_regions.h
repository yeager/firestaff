#ifndef DM1_V1_FMTOWNS_MENU_REGIONS_H
#define DM1_V1_FMTOWNS_MENU_REGIONS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked region coordinates for the FM Towns DM1 menu draw
 * chain. Every constant here is a byte-verified value from the
 * hash-verified HMA-240 English EDM.EXP, recovered via capstone
 * disassembly of GET_SCL_COORD (0x1942c) and the region registry at
 * [0x28f08] (head pointer = 0x28e78, 23 linked blocks).
 *
 * Full evidence:
 *   parity-evidence/dm1_fmtowns_region_table.md
 *
 * Record schema (8 bytes each): { u16 type, u16 parent, i16 a, i16 b }.
 *  - type == 9    -> (a, b) is (width, height); this is a SIZE node.
 *  - other types  -> (a, b) is an anchor pair whose semantics come
 *                    from the GET_COORD jump tables at 0x18f14/0x18fe6.
 *
 * GET_SCL_COORD returns the parent's size scaled by
 *   size * scale / 10000
 * where scale defaults to 10000 (1:1) via GET_RGN_COORD.
 *
 * Region 10 backs SPC_BLOT's dynamic menu panel; region 11 is the
 * clear-area anchor that inherits region 10's size through its
 * parent pointer.
 *
 * These constants do NOT ship pixels; they encode the source-lifted
 * geometry that a firestaff-side EGB shim must use to draw the
 * authentic FM Towns menu panel. Do not modify without matching
 * evidence in parity-evidence/dm1_fmtowns_region_table.md.
 */

typedef struct {
    uint16_t type;
    uint16_t parent;
    int16_t  a;
    int16_t  b;
} DM1_V1_FmtownsRegionRecord;

/* Region 10 — SPC_BLOT dynamic-menu panel size.
 * EDM.EXP block-1 record at offset (10 - 1) * 8 + 8. */
#define DM1_V1_FMTOWNS_REGION_ID_MENU_PANEL_SIZE    10
#define DM1_V1_FMTOWNS_REGION_MENU_PANEL_TYPE       9      /* SIZE node */
#define DM1_V1_FMTOWNS_REGION_MENU_PANEL_PARENT     2
#define DM1_V1_FMTOWNS_REGION_MENU_PANEL_WIDTH_PX   87
#define DM1_V1_FMTOWNS_REGION_MENU_PANEL_HEIGHT_PX  45

/* Region 11 — Dynamic menu clear-area anchor.
 * Parent points at region 10, so this record inherits the 87x45 size. */
#define DM1_V1_FMTOWNS_REGION_ID_MENU_CLEAR_AREA    11
#define DM1_V1_FMTOWNS_REGION_MENU_CLEAR_AREA_TYPE   2
#define DM1_V1_FMTOWNS_REGION_MENU_CLEAR_AREA_PARENT DM1_V1_FMTOWNS_REGION_ID_MENU_PANEL_SIZE
#define DM1_V1_FMTOWNS_REGION_MENU_CLEAR_AREA_ANCHOR_X 319
#define DM1_V1_FMTOWNS_REGION_MENU_CLEAR_AREA_ANCHOR_Y 77

/* Look up region 10 (menu panel size). Populates *out with the exact
 * source record. Returns 1 on success, 0 if out is NULL. This
 * function is source-lockable: it does not read any external state
 * and always returns the byte-verified EDM.EXP record. */
int dm1_v1_fmtowns_region_menu_panel_pc34(DM1_V1_FmtownsRegionRecord *out);

/* Look up region 11 (menu clear-area anchor). Same contract. */
int dm1_v1_fmtowns_region_menu_clear_area_pc34(DM1_V1_FmtownsRegionRecord *out);

/* Compute GET_SCL_COORD's exact scale conversion:
 *     result = (int)((int32_t)source * scale / 10000)
 * with the same signed 32-bit intermediate the original EDM.EXP
 * uses (imul then idiv). scale == 10000 returns source unchanged,
 * matching the GET_RGN_COORD 1:1 default. Returns 0 on overflow-
 * safe bounds; callers should treat any non-positive result as a
 * "region not visible" gate, matching EDM.EXP's post-conversion
 * `test <= 0 -> return 0` at 0x194ce / 0x194d5. */
int dm1_v1_fmtowns_region_scale_coord_pc34(int source, int scale);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_MENU_REGIONS_H */
