
#include "dm1_v2_settings_impl.h"
#include <string.h>

void dm1_v2_settings_init(DM1_V2_SettingsState *s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->footstep_interval = 0.4f;
    s->torch_intensity = 1.0f;
}

void dm1_v2_settings_tick(DM1_V2_SettingsState *s, float dt) {
    int i;
    if (!s) return;

    /* PC34 has no authenticated V2 camera-shake, bob or global-flicker
     * presentation path. Its source-owned torch light remains in F0337. */
    s->shake_x = 0.0f;
    s->shake_y = 0.0f;
    s->shake_intensity = 0.0f;
    s->bob_phase = 0.0f;
    s->torch_phase = 0.0f;
    s->torch_intensity = 1.0f;

    /* Footstep timer */
    s->footstep_timer += dt;

    /* Damage numbers float up and fade */
    for (i = 0; i < s->damage_num_count; i++) {
        s->damage_nums[i].y -= 30.0f * dt;
        s->damage_nums[i].life -= dt;
        if (s->damage_nums[i].life <= 0) {
            /* Remove by shifting */
            memmove(&s->damage_nums[i], &s->damage_nums[i+1],
                (s->damage_num_count - i - 1) * sizeof(s->damage_nums[0]));
            s->damage_num_count--;
            i--;
        }
    }

    /* Screen transition */
    if (s->transition_active) {
        if (s->transition_direction == 0) { /* fade out */
            s->transition_alpha += dt * 2.0f;
            if (s->transition_alpha >= 1.0f) {
                s->transition_alpha = 1.0f;
                s->transition_active = 0;
            }
        } else { /* fade in */
            s->transition_alpha -= dt * 2.0f;
            if (s->transition_alpha <= 0.0f) {
                s->transition_alpha = 0.0f;
                s->transition_active = 0;
            }
        }
    }

}

void dm1_v2_camera_shake_trigger(DM1_V2_SettingsState *s, float intensity) {
    (void)s;
    (void)intensity;
}

void dm1_v2_damage_number_add(DM1_V2_SettingsState *s, int value, float x, float y, int is_heal) {
    (void)s;
    (void)value;
    (void)x;
    (void)y;
    (void)is_heal;
}

void dm1_v2_transition_start(DM1_V2_SettingsState *s, int fade_out) {
    if (!s) return;
    s->transition_active = 1;
    s->transition_direction = fade_out ? 0 : 1;
    if (fade_out) s->transition_alpha = 0.0f;
    else s->transition_alpha = 1.0f;
}

void dm1_v2_footstep_set_surface(DM1_V2_SettingsState *s, int surface) {
    if (s) s->footstep_surface = surface;
}

void dm1_v2_apply_camera_shake(uint32_t *rgba, int w, int h, const DM1_V2_SettingsState *s) {
    (void)rgba;
    (void)w;
    (void)h;
    (void)s;
}

/* Render floating damage numbers */
void dm1_v2_render_damage_numbers(uint32_t *rgba, int w, int h, const DM1_V2_SettingsState *s) {
    (void)rgba;
    (void)w;
    (void)h;
    (void)s;
    /* Do not draw modern damage indicators over source-owned combat pixels. */
}

void dm1_v2_apply_transition(uint32_t *rgba, int w, int h, float alpha) {
    int i, total;
    if (!rgba || alpha <= 0.001f) return;
    total = w * h;
    for (i = 0; i < total; i++) {
        uint32_t c = rgba[i];
        int r = (int)(((c>>16)&0xFF) * (1.0f - alpha));
        int g = (int)(((c>>8)&0xFF) * (1.0f - alpha));
        int b = (int)((c&0xFF) * (1.0f - alpha));
        rgba[i] = 0xFF000000 | (r<<16) | (g<<8) | b;
    }
}

void dm1_v2_apply_torch_flicker(uint32_t *rgba, int w, int h, float intensity) {
    (void)rgba;
    (void)w;
    (void)h;
    (void)intensity;
}
