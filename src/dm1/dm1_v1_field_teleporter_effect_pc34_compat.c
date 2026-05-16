/* DM1 V1 Field/Teleporter Visual Effects — source-locked from ReDMCSB
 * MOVESENS.C F0263: teleporter square processing on party entry
 * PROJEXPL.C: explosion rendering with radial particle scatter
 * TIMELINE.C: event-scheduled visual effects (flash timers) */

#include "dm1_v1_field_teleporter_effect_pc34_compat.h"
#include <string.h>
#include <stdlib.h>

void m11_ft_init(M11_FT_EffectState* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_FT_EffectState));
}

static void spawn_particles(M11_FT_EffectState* state, int16_t cx, int16_t cy,
                             uint8_t count, uint8_t color) {
    for (uint8_t i = 0; i < count && state->particle_count < DM1_FT_MAX_PARTICLES; i++) {
        M11_FT_Particle* p = &state->particles[state->particle_count++];
        p->x = cx;
        p->y = cy;
        /* Simple radial scatter — pseudo-random via index */
        p->dx = (int16_t)((i % 5) - 2);
        p->dy = (int16_t)((i % 3) - 1);
        p->color = color;
        p->life = (uint8_t)(6 + (i % 4));
        p->active = true;
    }
}

/* MOVESENS.C F0263 pattern: teleporter triggers flash + level transition */
void m11_ft_start_teleport(M11_FT_EffectState* state,
                            int16_t sx, int16_t sy, int16_t sl,
                            int16_t dx, int16_t dy, int16_t dl) {
    if (!state) return;
    state->type = M11_FT_EFFECT_TELEPORT;
    state->frame = 0;
    state->total_frames = DM1_FT_TELEPORT_FRAMES;
    state->active = true;
    state->src_x = sx; state->src_y = sy; state->src_level = sl;
    state->dst_x = dx; state->dst_y = dy; state->dst_level = dl;
    state->flash_intensity = 15; /* Full white flash */
    state->particle_count = 0;
    spawn_particles(state, 112, 72, 16, 15); /* White particles at viewport center */
}

void m11_ft_start_pit_fall(M11_FT_EffectState* state,
                            int16_t x, int16_t y, int16_t level) {
    if (!state) return;
    state->type = M11_FT_EFFECT_PIT_FALL;
    state->frame = 0;
    state->total_frames = DM1_FT_PIT_FALL_FRAMES;
    state->active = true;
    state->src_x = x; state->src_y = y; state->src_level = level;
    state->dst_x = x; state->dst_y = y; state->dst_level = level + 1;
    state->flash_intensity = 0;
    state->particle_count = 0;
}

/* PROJEXPL.C pattern: explosion with radial particles */
void m11_ft_start_explosion(M11_FT_EffectState* state,
                             int16_t screen_x, int16_t screen_y,
                             uint8_t radius) {
    if (!state) return;
    state->type = M11_FT_EFFECT_EXPLOSION;
    state->frame = 0;
    state->total_frames = (int)(radius * 2);
    state->active = true;
    state->flash_intensity = 12;
    state->particle_count = 0;
    uint8_t count = (radius > 8) ? DM1_FT_MAX_PARTICLES : (uint8_t)(radius * 3);
    spawn_particles(state, screen_x, screen_y, count, 8); /* Red particles */
}

bool m11_ft_tick(M11_FT_EffectState* state) {
    if (!state || !state->active) return false;

    state->frame++;

    /* Decay flash intensity */
    if (state->flash_intensity > 0) {
        state->flash_intensity--;
    }

    /* Update particles */
    for (uint8_t i = 0; i < state->particle_count; i++) {
        M11_FT_Particle* p = &state->particles[i];
        if (!p->active) continue;
        p->x += p->dx;
        p->y += p->dy;
        if (p->life > 0) {
            p->life--;
        }
        if (p->life == 0) {
            p->active = false;
        }
    }

    /* Check completion */
    if (state->frame >= state->total_frames) {
        state->active = false;
        return false;
    }

    return true;
}

bool m11_ft_is_active(const M11_FT_EffectState* state) {
    return state && state->active;
}

uint8_t m11_ft_get_flash_intensity(const M11_FT_EffectState* state) {
    if (!state) return 0;
    return state->flash_intensity;
}

uint8_t m11_ft_get_particle_count(const M11_FT_EffectState* state) {
    if (!state) return 0;
    uint8_t count = 0;
    for (uint8_t i = 0; i < state->particle_count; i++) {
        if (state->particles[i].active) count++;
    }
    return count;
}
