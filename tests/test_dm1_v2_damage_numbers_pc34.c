#include "dm1_v2_damage_numbers_pc34.h"
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
    for (int i = 0; i < 4; ++i) {
        if (rgba[i] != expected_rgba[i]) ok = 0;
    }

    puts(ok ? "PASS dm1_v2_damage_numbers_pc34" :
              "FAIL dm1_v2_damage_numbers_pc34");
    return ok ? 0 : 1;
}
