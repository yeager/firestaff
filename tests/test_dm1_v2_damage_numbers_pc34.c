#include "dm1_v2_damage_numbers_pc34.h"
#include "dm1_v2_camera_shake_pc34.h"
#include "dm1_v2_settings_impl.h"

#include <stdint.h>
#include <stdio.h>

int main(void) {
    uint8_t framebuffer[16] = {
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16
    };
    const uint8_t expected[16] = {
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16
    };
    uint32_t rgba[4] = { 0x11223344u, 0x55667788u, 0x99aabbccu, 0xddeeff00u };
    const uint32_t expected_rgba[4] = {
        0x11223344u, 0x55667788u, 0x99aabbccu, 0xddeeff00u
    };
    DM1_V2_SettingsState settings;
    int ok = 1;

    v2_damage_init();
    v2_damage_spawn(2.0f, 2.0f, 99, 15);
    v2_damage_update(1.0f);
    v2_damage_render(framebuffer, 4, 4);
    v2_damage_clear();
    for (int i = 0; i < 16; ++i) {
        if (framebuffer[i] != expected[i]) ok = 0;
    }
    dm1_v2_settings_init(&settings);
    dm1_v2_damage_number_add(&settings, 99, 1.0f, 1.0f, 0);
    dm1_v2_render_damage_numbers(rgba, 2, 2, &settings);
    dm1_v2_camera_shake_trigger(&settings, 9.0f);
    settings.shake_x = 2.0f;
    settings.shake_y = -1.0f;
    dm1_v2_apply_camera_shake(rgba, 2, 2, &settings);
    dm1_v2_apply_torch_flicker(rgba, 2, 2, 0.5f);
    dm1_v2_settings_tick(&settings, 1.0f);
    if (settings.shake_intensity != 0.0f || settings.bob_phase != 0.0f ||
        settings.torch_intensity != 1.0f) ok = 0;
    v2_shake_trigger(10.0f, 1.0f);
    v2_shake_update(1.0f);
    if (v2_shake_is_active()) ok = 0;
    v22_shake_add_trauma(1.0f);
    v22_shake_tick(1.0f, &settings.shake_x, &settings.shake_y);
    if (settings.shake_x != 0.0f || settings.shake_y != 0.0f ||
        v22_shake_get_trauma() != 0.0f) ok = 0;
    for (int i = 0; i < 4; ++i) {
        if (rgba[i] != expected_rgba[i]) ok = 0;
    }

    puts(ok ? "PASS dm1_v2_damage_numbers_pc34" :
              "FAIL dm1_v2_damage_numbers_pc34");
    return ok ? 0 : 1;
}
