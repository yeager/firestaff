#ifndef CSB_V1_ATARI_F0115_CLICK_BOUNDS_H
#define CSB_V1_ATARI_F0115_CLICK_BOUNDS_H

typedef struct CSB_V1_AtariF0115ClickBounds {
    int valid;
    int left;
    int right;
    int top;
    int bottom;
} CSB_V1_AtariF0115ClickBounds;

/* ReDMCSB DUNVIEW.C F0115 accumulates each rendered floor object's clipped
 * box into G0291[viewCell]; CLIKVIEW.C F0377 hit-tests that union. */
void csb_v1_atari_f0115_click_bounds_clear(
    CSB_V1_AtariF0115ClickBounds bounds[4]);
int csb_v1_atari_f0115_click_bounds_add(
    CSB_V1_AtariF0115ClickBounds bounds[4], int view_cell,
    int left, int top, int width, int height);
int csb_v1_atari_f0115_click_bounds_contains(
    const CSB_V1_AtariF0115ClickBounds bounds[4], int view_cell,
    int x, int y);

#endif
