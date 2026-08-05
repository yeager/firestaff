#include "dm1_v2_hud_overlay_pc34.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static int all_equal(const uint8_t* fb, int count, uint8_t value) {
    for (int i = 0; i < count; ++i) {
        if (fb[i] != value) return 0;
    }
    return 1;
}

int main(void) {
    uint8_t fb[320 * 200];
    const int pixel_count = (int)sizeof(fb);

    memset(fb, 0x5a, sizeof(fb));
    v2_hud_init();
    v2_hud_set_direction(3);
    v2_hud_set_level(3, 12);
    v2_hud_set_opacity(200);
    v2_hud_set_champion_overlay_state(0, 50, 75, 25, true, true);
    v2_hud_set_action_overlay_state(1, 2, 2);
    v2_hud_set_rune_overlay_state(0x05u, 2, true, true, true);
    v2_hud_render(fb, 320, 200);
    CHECK(all_equal(fb, pixel_count, 0x5a));

    memset(fb, 0xa5, sizeof(fb));
    v22_hud_render_champion_panel(fb, 320, 200, NULL, NULL, NULL);
    CHECK(all_equal(fb, pixel_count, 0xa5));

    v2_hud_toggle();
    memset(fb, 0x33, sizeof(fb));
    v2_hud_render(fb, 320, 200);
    CHECK(all_equal(fb, pixel_count, 0x33));

    v22_hud_notify_move_complete();
    CHECK(v22_hud_is_move_complete_pending() == 1);
    v22_hud_clear_move_complete();
    CHECK(v22_hud_is_move_complete_pending() == 0);
    v22_hud_notify_turn_complete();
    CHECK(v22_hud_is_turn_complete_pending() == 1);
    v22_hud_clear_turn_complete();
    CHECK(v22_hud_is_turn_complete_pending() == 0);

    v22_hud_start_health_pulse();
    CHECK(v22_hud_health_pulse_alpha() >= 0.0f &&
          v22_hud_health_pulse_alpha() <= 1.0f);
    for (int i = 0; i < 9; ++i) v22_hud_pulse_v1_tick();
    CHECK(v22_hud_health_pulse_alpha() >= 0.0f &&
          v22_hud_health_pulse_alpha() <= 1.0f);

    const char* evidence = v21_hud_panel_source_evidence();
    CHECK(evidence != NULL);
    CHECK(strstr(evidence, "strict no-draw") != NULL);
    CHECK(strstr(evidence, "M653") != NULL);
    CHECK(strstr(evidence, "C009") != NULL);

    if (failures) return 1;
    puts("dm1_v2_hud_overlay_pc34: no-draw contract ok");
    return 0;
}
