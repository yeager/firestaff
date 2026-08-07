#include "dm1_v2_screen_transition_pc34.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int copied_unchanged(const uint8_t *source, const uint8_t *actual) {
    return memcmp(source, actual, 6) == 0;
}

int main(void) {
    const uint8_t source[6] = { 1, 3, 5, 7, 9, 11 };
    uint8_t output[6] = { 0, 0, 0, 0, 0, 0 };
    int ok = 1;

    v2_transition_init();
    v2_transition_start(FADE_BLACK, 12.0f, 3, 2);
    v2_transition_update(4.0f);
    v2_transition_apply(source, output, 3, 2);
    ok &= copied_unchanged(source, output);
    ok &= !v2_transition_is_active();
    v2_transition_skip();

    memset(output, 0, sizeof(output));
    v2_screen_transition_start(WIPE_LEFT, 24.0f);
    v2_screen_transition_update(24.0f);
    v2_screen_transition_apply(source, output, 3, 2);
    ok &= copied_unchanged(source, output);
    ok &= v2_screen_transition_progress() == 0.0f;
    ok &= v2_screen_transition_is_done() == 1;

    v22_screen_fade_start(0);
    v22_screen_fade_update(24.0f);
    ok &= v22_screen_fade_alpha() == 0.0f;
    ok &= v22_screen_fade_is_done() == 1;

    puts(ok ? "PASS dm1_v2_screen_transition_pc34" :
              "FAIL dm1_v2_screen_transition_pc34");
    return ok ? 0 : 1;
}
