#ifndef FIRESTAFF_REDMCSB_F0619_SLOT_BOX_BORDER_H
#define FIRESTAFF_REDMCSB_F0619_SLOT_BOX_BORDER_H

#include <stdint.h>

typedef struct {
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
} RedmcsbF0619Rect;

typedef enum {
    REDMCSB_F0619_NO_MARGIN = 0,
    REDMCSB_F0619_MEDIA508_MARGIN = 1,
    REDMCSB_F0619_MEDIA720_MARGIN = 2
} RedmcsbF0619MarginBranch;

/* Adapter for the edition-owned F0635 zone resolver used by F0618.  The
 * resolver receives the source icon dimensions as initial width/height. */
typedef int (*RedmcsbF0619ResolveZoneFn)(
    void *context, int16_t zone_index, int16_t initial_width,
    int16_t initial_height, RedmcsbF0619Rect *out_rect);

typedef struct {
    uint8_t accepted;
    int16_t zone_index;
    int16_t initial_width;
    int16_t initial_height;
    int16_t text_margin;
    RedmcsbF0619MarginBranch margin_branch;
    RedmcsbF0619Rect rect;
} RedmcsbF0619Receipt;

/* ReDMCSB OBJECT.C:365-389 F0618/F0619.  Zone geometry remains owned by the
 * source edition's F0635 data; this adapter applies F0619's exact margin
 * branch to the canonical rectangle returned by that resolver. */
int redmcsb_f0619_get_slot_box_border(
    int16_t zone_index, int16_t object_icon_width,
    int16_t object_icon_height, int16_t text_margin,
    RedmcsbF0619MarginBranch margin_branch,
    RedmcsbF0619ResolveZoneFn resolve_zone, void *context,
    RedmcsbF0619Receipt *out_receipt);

#endif
