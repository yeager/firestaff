
#ifndef DM1_V2_SETTINGS_IMPL_H
#define DM1_V2_SETTINGS_IMPL_H

#include <stdint.h>

/* V2.2 enhanced settings — actual implementation hooks.
 * Each setting has: apply(), tick(), render() where applicable. */

typedef struct {
    /* Footstep audio */
    int footstep_surface;     /* 0=stone, 1=wood, 2=metal, 3=water, 4=dirt */
    float footstep_timer;
    float footstep_interval;  /* seconds between steps */

    /* Camera shake */
    float shake_x, shake_y;
    float shake_intensity;
    float shake_decay;

    /* Camera bob */
    float bob_phase;
    float bob_amplitude;

    /* Smooth movement interpolation */
    float move_lerp;          /* 0.0 = at start, 1.0 = at dest */
    int move_from_x, move_from_y;
    int move_to_x, move_to_y;

    /* Damage numbers */
    struct {
        int value;
        float x, y;
        float life;
        int is_heal;
    } damage_nums[8];
    int damage_num_count;

    /* Screen transitions */
    float transition_alpha;   /* 0=fully visible, 1=fully black */
    int transition_active;
    int transition_direction; /* 0=fade out, 1=fade in */

    /* Torch flicker */
    float torch_phase;
    float torch_intensity;    /* 0.7 - 1.0 range */
} DM1_V2_SettingsState;

void dm1_v2_settings_init(DM1_V2_SettingsState *s);
void dm1_v2_settings_tick(DM1_V2_SettingsState *s, float dt);

/* Individual effects */
void dm1_v2_camera_shake_trigger(DM1_V2_SettingsState *s, float intensity);
void dm1_v2_damage_number_add(DM1_V2_SettingsState *s, int value, float x, float y, int is_heal);
void dm1_v2_transition_start(DM1_V2_SettingsState *s, int fade_out);
void dm1_v2_footstep_set_surface(DM1_V2_SettingsState *s, int surface);

/* Render effects onto RGBA buffer */
void dm1_v2_apply_camera_shake(uint32_t *rgba, int w, int h, const DM1_V2_SettingsState *s);
void dm1_v2_render_damage_numbers(uint32_t *rgba, int w, int h, const DM1_V2_SettingsState *s);
void dm1_v2_apply_transition(uint32_t *rgba, int w, int h, float alpha);
void dm1_v2_apply_torch_flicker(uint32_t *rgba, int w, int h, float intensity);

#endif

