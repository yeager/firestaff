#include "csb_v1_atari_f0115_click_bounds.h"

#include <assert.h>

int main(void)
{
    CSB_V1_AtariF0115ClickBounds bounds[4];
    csb_v1_atari_f0115_click_bounds_clear(bounds);
    assert(!csb_v1_atari_f0115_click_bounds_contains(bounds, 0, 12, 18));
    assert(csb_v1_atari_f0115_click_bounds_add(bounds, 0, 10, 20, 5, 4));
    assert(csb_v1_atari_f0115_click_bounds_add(bounds, 0, 16, 18, 3, 5));
    assert(bounds[0].left == 10 && bounds[0].right == 18 &&
           bounds[0].top == 18 && bounds[0].bottom == 23);
    assert(csb_v1_atari_f0115_click_bounds_contains(bounds, 0, 10, 18));
    assert(csb_v1_atari_f0115_click_bounds_contains(bounds, 0, 18, 23));
    assert(!csb_v1_atari_f0115_click_bounds_contains(bounds, 0, 19, 23));
    assert(!csb_v1_atari_f0115_click_bounds_contains(bounds, 1, 12, 18));
    assert(!csb_v1_atari_f0115_click_bounds_add(bounds, 4, 0, 0, 1, 1));
    assert(!csb_v1_atari_f0115_click_bounds_add(bounds, 0, 0, 0, 0, 1));
    csb_v1_atari_f0115_click_bounds_clear(bounds);
    assert(!csb_v1_atari_f0115_click_bounds_contains(bounds, 0, 12, 18));
    return 0;
}
