
#include "dm1_v2_settings_impl.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

void dm1_v2_settings_init(DM1_V2_SettingsState *s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->footstep_interval = 0.4f;
    s->bob_amplitude = 2.0f;
    s->torch_intensity = 1.0f;
}

void dm1_v2_settings_tick(DM1_V2_SettingsState *s, float dt) {
    int i;
    if (!s) return;

    /* Camera shake decay */
    if (s->shake_intensity > 0.01f) {
        s->shake_x = (float)(rand() % 100 - 50) / 50.0f * s->shake_intensity;
        s->shake_y = (float)(rand() % 100 - 50) / 50.0f * s->shake_intensity;
        s->shake_intensity *= (1.0f - s->shake_decay * dt);
    } else {
        s->shake_x = s->shake_y = 0;
        s->shake_intensity = 0;
    }

    /* Camera bob */
    s->bob_phase += dt * 8.0f;

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

    /* Torch flicker */
    s->torch_phase += dt * 10.0f;
    s->torch_intensity = 0.85f + 0.15f * sinf(s->torch_phase) *
        (0.9f + 0.1f * sinf(s->torch_phase * 3.7f));
}

void dm1_v2_camera_shake_trigger(DM1_V2_SettingsState *s, float intensity) {
    if (s) { s->shake_intensity = intensity; s->shake_decay = 5.0f; }
}

void dm1_v2_damage_number_add(DM1_V2_SettingsState *s, int value, float x, float y, int is_heal) {
    if (!s || s->damage_num_count >= 8) return;
    s->damage_nums[s->damage_num_count].value = value;
    s->damage_nums[s->damage_num_count].x = x;
    s->damage_nums[s->damage_num_count].y = y;
    s->damage_nums[s->damage_num_count].life = 1.5f;
    s->damage_nums[s->damage_num_count].is_heal = is_heal;
    s->damage_num_count++;
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

/* Apply camera shake as pixel offset */
void dm1_v2_apply_camera_shake(uint32_t *rgba, int w, int h, const DM1_V2_SettingsState *s) {
    int ox, oy, y;
    uint32_t *tmp;
    if (!rgba || !s || s->shake_intensity < 0.5f) return;
    ox = (int)s->shake_x;
    oy = (int)s->shake_y;
    if (ox == 0 && oy == 0) return;
    tmp = (uint32_t *)malloc(w * h * sizeof(uint32_t));
    if (!tmp) return;
    memcpy(tmp, rgba, w * h * sizeof(uint32_t));
    memset(rgba, 0, w * h * sizeof(uint32_t));
    for (y = 0; y < h; y++) {
        int sy = y - oy;
        if (sy >= 0 && sy < h) {
            int src_start = (ox > 0) ? ox : 0;
            int dst_start = (ox > 0) ? 0 : -ox;
            int copy_w = w - abs(ox);
            if (copy_w > 0)
                memcpy(rgba + y*w + dst_start, tmp + sy*w + src_start, copy_w * sizeof(uint32_t));
        }
    }
    free(tmp);
}

/* Render floating damage numbers */
void dm1_v2_render_damage_numbers(uint32_t *rgba, int w, int h, const DM1_V2_SettingsState *s) {
    int i;
    if (!rgba || !s) return;
    for (i = 0; i < s->damage_num_count; i++) {
        int sx = (int)s->damage_nums[i].x;
        int sy = (int)s->damage_nums[i].y;
        float alpha = s->damage_nums[i].life / 1.5f;
        uint32_t color = s->damage_nums[i].is_heal ? 0xFF00FF00 : 0xFFFF4444;
        /* Draw a small colored dot (actual digit rendering needs font) */
        if (sx >= 0 && sx < w && sy >= 0 && sy < h && alpha > 0.1f) {
            int dx, dy2;
            for (dy2 = -1; dy2 <= 1; dy2++)
                for (dx = -1; dx <= 1; dx++) {
                    int px = sx + dx, py = sy + dy2;
                    if (px >= 0 && px < w && py >= 0 && py < h)
                        rgba[py * w + px] = color;
                }
        }
    }
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
    int i, total;
    if (!rgba || intensity >= 0.99f) return;
    total = w * h;
    for (i = 0; i < total; i++) {
        uint32_t c = rgba[i];
        int r = (int)(((c>>16)&0xFF) * intensity);
        int g = (int)(((c>>8)&0xFF) * intensity);
        int b = (int)((c&0xFF) * intensity);
        rgba[i] = 0xFF000000 | (r<<16) | (g<<8) | b;
    }
}

