#ifndef FIRESTAFF_DM2_V1_BUTTONS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_BUTTONS_PC34_COMPAT_H

/*
 * dm2_v1_buttons_pc34_compat.h — DM2 button group management.
 *
 * Source: skproject c_buttons.cpp (6 functions).
 * Manages rectangular button regions with containment testing,
 * clipping, and offset computation for the DM2 GUI.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Data structures
 * ======================================================================== */

typedef struct {
    int16_t x, y, w, h;
} DM2_V1_ButtonRect;

/* c_button — single button with DB index and bounding rect.
 * skproject: c_button size 0xc */
typedef struct {
    int16_t         dbidx;      /* @00 — NODATA = -1 */
    DM2_V1_ButtonRect r;        /* @02 — bounding rectangle */
    int16_t         groupsize;  /* @0a — number of active rects */
} DM2_V1_Button;

/* c_buttongroup — button with up to 5 sub-rectangles.
 * skproject: c_buttongroup size 0x34 */
typedef struct {
    DM2_V1_Button     button;   /* @00 */
    DM2_V1_ButtonRect rects[5]; /* @0c */
} DM2_V1_ButtonGroup;

/* ========================================================================
 * Receipts
 * ======================================================================== */

typedef struct {
    DM2_V1_ButtonRect result;
} DM2_V1_OffsetRectReceipt;

typedef struct {
    bool    adjusted;       /* true if a slot was written */
    int16_t slot_used;     /* which slot index was used */
    bool    clipped;        /* true if rect was clipped to bounds */
    bool    removed;        /* true if rect was fully clipped away */
    bool    already_contained; /* true if existing rect already covers input */
    bool    overflow;       /* true if all 5 slots full */
} DM2_V1_AdjustButtonGroupReceipt;

/* ========================================================================
 * Functions
 * ======================================================================== */

void dm2_v1_button_init(DM2_V1_Button *b);
void dm2_v1_buttongroup_init(DM2_V1_ButtonGroup *bg);
void dm2_v1_init_global_buttongroups(DM2_V1_ButtonGroup *bg1,
                                      DM2_V1_ButtonGroup *bg2);

DM2_V1_OffsetRectReceipt dm2_v1_offset_rect(
    const DM2_V1_ButtonGroup *bg,
    const DM2_V1_ButtonRect *src);

DM2_V1_AdjustButtonGroupReceipt dm2_v1_adjust_buttongroup_rects(
    DM2_V1_ButtonGroup *bg,
    const DM2_V1_ButtonRect *rect);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_BUTTONS_PC34_COMPAT_H */
