#include "csb_v1_atari_f0115_click_bounds.h"

#include <limits.h>
#include <string.h>

void csb_v1_atari_f0115_click_bounds_clear(
    CSB_V1_AtariF0115ClickBounds bounds[4])
{
    if (bounds) memset(bounds, 0, sizeof(*bounds) * 4u);
}

int csb_v1_atari_f0115_click_bounds_add(
    CSB_V1_AtariF0115ClickBounds bounds[4], int view_cell,
    int left, int top, int width, int height)
{
    CSB_V1_AtariF0115ClickBounds *box;
    int right;
    int bottom;
    if (!bounds || view_cell < 0 || view_cell >= 4 ||
        width <= 0 || height <= 0 || left > INT_MAX - width + 1 ||
        top > INT_MAX - height + 1) return 0;
    right = left + width - 1;
    bottom = top + height - 1;
    box = &bounds[view_cell];
    if (!box->valid) {
        box->valid = 1;
        box->left = left;
        box->right = right;
        box->top = top;
        box->bottom = bottom;
        return 1;
    }
    if (left < box->left) box->left = left;
    if (right > box->right) box->right = right;
    if (top < box->top) box->top = top;
    if (bottom > box->bottom) box->bottom = bottom;
    return 1;
}

int csb_v1_atari_f0115_click_bounds_contains(
    const CSB_V1_AtariF0115ClickBounds bounds[4], int view_cell,
    int x, int y)
{
    const CSB_V1_AtariF0115ClickBounds *box;
    if (!bounds || view_cell < 0 || view_cell >= 4) return 0;
    box = &bounds[view_cell];
    return box->valid && x >= box->left && x <= box->right &&
           y >= box->top && y <= box->bottom;
}
