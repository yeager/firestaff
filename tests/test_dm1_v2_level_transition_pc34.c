#include "dm1_v2_level_transition_pc34.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint8_t framebuffer[12] = { 1, 3, 5, 7, 9, 11, 13, 15, 2, 4, 6, 8 };
    uint8_t before[sizeof(framebuffer)];
    int ok = 1;

    memcpy(before, framebuffer, sizeof(framebuffer));
    v2_level_trans_init();
    v2_level_trans_start(TRANS_PIT_FALL, 1, 2, 12, 34, 1, 10.0f);
    ok &= !v2_level_trans_is_active();
    ok &= !v2_level_trans_update(99.0f);
    v2_level_trans_render_overlay(framebuffer, 4, 3);
    ok &= memcmp(framebuffer, before, sizeof(framebuffer)) == 0;
    v2_level_trans_cancel();

    ok &= v2_level_transition_tick(99.0f) == 0;
    ok &= v2_level_transition_get_progress() == 0.0f;
    ok &= v2_level_transition_is_active() == 0;
    ok &= v22_transition_duration_for_type(1) == 0.0f;
    ok &= v22_transition_duration_for_type(4) == 0.0f;

    puts(ok ? "PASS dm1_v2_level_transition_pc34" :
              "FAIL dm1_v2_level_transition_pc34");
    return ok ? 0 : 1;
}
