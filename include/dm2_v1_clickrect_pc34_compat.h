#ifndef FIRESTAFF_DM2_V1_CLICKRECT_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CLICKRECT_PC34_COMPAT_H

/*
 * dm2_v1_clickrect_pc34_compat.h — DM2 click rectangle zones.
 *
 * Source: skproject c_clickrect.cpp (5 functions).
 * Manages clickable regions for mouse interaction — linked list of
 * click rectangles with priority-sorted insertion and removal.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Data structures
 * ======================================================================== */

typedef struct { int16_t x, y, w, h; } DM2_V1_ClickRect;

typedef struct DM2_V1_ClickRectData {
    struct DM2_V1_ClickRectNode *next;
    DM2_V1_ClickRect r;
} DM2_V1_ClickRectData;

typedef struct DM2_V1_ClickRectNode {
    int16_t  w_00;      /* rect index / flags */
    int8_t   b_02;
    int8_t   b_03;      /* bit 0x80 = linked, bit 0x40 = allocated, low nibble = priority */
    int8_t   buttons;   /* @04 */
    int8_t   b_05;      /* mouse rect flag */
    DM2_V1_ClickRectData *node; /* @06 */
} DM2_V1_ClickRectNode;

#define DM2_V1_CLICKRECT_TABLE_SIZE 18

/* ========================================================================
 * Receipts
 * ======================================================================== */

typedef struct {
    bool    set;
    int16_t rx0, ry0, rx1, ry1;
    int16_t rect_w, rect_h;
} DM2_V1_SetClickRectReceipt;

typedef struct {
    bool refreshed;
    bool was_linked;
} DM2_V1_RefreshClickRectReceipt;

typedef struct {
    bool allocated;
    bool was_already_allocated;
} DM2_V1_AllocClickRectReceipt;

/* ========================================================================
 * Functions
 * ======================================================================== */

void dm2_v1_init_clickrect_table(DM2_V1_ClickRectNode table[DM2_V1_CLICKRECT_TABLE_SIZE]);

DM2_V1_SetClickRectReceipt dm2_v1_set_clickrect_datas(
    int16_t x0, int16_t y0, int16_t x1, int16_t y1);

DM2_V1_RefreshClickRectReceipt dm2_v1_refresh_clickrectlink_1(
    DM2_V1_ClickRectNode *node,
    DM2_V1_ClickRectNode **list_head);

DM2_V1_RefreshClickRectReceipt dm2_v1_refresh_clickrectlink_2(
    DM2_V1_ClickRectNode *node,
    DM2_V1_ClickRectNode **list_head);

DM2_V1_AllocClickRectReceipt dm2_v1_alloc_clickrect_data(
    DM2_V1_ClickRectNode *node);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CLICKRECT_PC34_COMPAT_H */
