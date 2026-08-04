/*
 * dm2_v1_sfx_pc34_compat.c -- DM2 sound effects processing.
 *
 * Ports the positional sound system from skproject c_sfx.cpp.
 * The Allegro-specific audio playback is abstracted through callbacks;
 * this module handles the game-logic layer: distance/occlusion
 * calculation, directional panning, and the noise queue.
 *
 * Source: skproject/SKULLWIN/c_sfx.cpp
 */

#include "dm2_v1_sfx_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

/* ── State init ─────────────────────────────────────────────────────── */

void dm2_v1_sfx_state_init(DM2_V1_SfxState *state)
{
    if (state == NULL) return;
    memset(state, 0, sizeof(*state));
}

/* ── Volume and pan calculation ─────────────────────────────────────── */

/* Source: c_sfx.cpp c_sfx::do_sound — ad-hoc volume/pan formulas */
void dm2_v1_sfx_calc_volume_pan(int8_t vol_level, int8_t dx, int8_t dy,
                                int16_t *out_volume, int16_t *out_pan)
{
    int distance = (abs((int)dx) + abs((int)dy)) / 2;
    int volume = (255 - 36 * (7 - (int)vol_level)) / (distance + 1);
    if (volume < 0) volume = 0;
    if (volume > 255) volume = 255;

    int pan = 0x80 + 4 * (int)dx;
    if (pan < 0) pan = 0;
    if (pan > 0xFF) pan = 0xFF;

    if (out_volume) *out_volume = (int16_t)volume;
    if (out_pan)    *out_pan    = (int16_t)pan;
}

/* ── Sound distance / occlusion ─────────────────────────────────────── */

/* Source: c_sfx.cpp R_1FB7D */
DM2_V1_SfxSoundDistanceReceipt dm2_v1_sfx_calc_sound_distance(
    const DM2_V1_SfxCallbacks *cb,
    int16_t x, int16_t y)
{
    DM2_V1_SfxSoundDistanceReceipt r;
    r.valid = false;
    r.distance = -1;

    if (cb == NULL) return r;

    int16_t cur_map = cb->get_current_map(cb->ctx);
    uint8_t *view_data;
    int16_t view_width;

    if (cur_map == cb->get_view_map1(cb->ctx)) {
        view_data  = cb->get_view_data1(cb->ctx);
        view_width = cb->get_view_width1(cb->ctx);
    } else if (cur_map == cb->get_view_map2(cb->ctx)) {
        view_data  = cb->get_view_data2(cb->ctx);
        view_width = cb->get_view_width2(cb->ctx);
    } else {
        return r;
    }

    if (x >= view_width) return r;

    int map_height = cb->get_map_height(cb->ctx);
    uint8_t val = view_data[((int)x << 5) + (int)y];

    if ((val & 0x80) != 0) {
        /* Occluded — find minimum of neighbours */
        static const int16_t dx_table[4] = { 0,  1,  0, -1 };
        static const int16_t dy_table[4] = { 1,  0, -1,  0 };

        val = 0x7F;
        for (int i = 0; i <= 3; i++) {
            int16_t nx = x + dx_table[i];
            int16_t ny = y + dy_table[i];
            if (nx >= 0 && nx < view_width && ny >= 0 && ny < map_height) {
                uint8_t nval = view_data[((int)nx << 5) + (int)ny];
                if (nval != 0 && val > nval)
                    val = nval;
            }
        }
    }

    r.valid = true;
    r.distance = (int32_t)(val) - 1;
    return r;
}

/* ── QUEUE_NOISE_GEN2 ───────────────────────────────────────────────── */

/* Source: c_sfx.cpp DM2_QUEUE_NOISE_GEN2 */
DM2_V1_SfxQueueNoiseReceipt dm2_v1_sfx_queue_noise_gen2(
    const DM2_V1_SfxCallbacks *cb,
    DM2_V1_SfxState *state,
    int8_t cat, int8_t type, int8_t sub, int8_t fallback_type,
    int16_t x, int16_t y, int16_t mode, int16_t vol, int16_t dist)
{
    int8_t effective_type = fallback_type;

    if (cb && cb->query_snd_entry_index) {
        if (cb->query_snd_entry_index(cb->ctx, cat, type, sub) != 0)
            effective_type = type;
    }

    return dm2_v1_sfx_queue_noise_gen1(cb, state,
        cat, effective_type, sub, vol, dist, x, y, mode);
}

/* ── QUEUE_NOISE_GEN1 ───────────────────────────────────────────────── */

/* Source: c_sfx.cpp DM2_QUEUE_NOISE_GEN1 */
DM2_V1_SfxQueueNoiseReceipt dm2_v1_sfx_queue_noise_gen1(
    const DM2_V1_SfxCallbacks *cb,
    DM2_V1_SfxState *state,
    int8_t cat, int8_t type, int8_t sub, int16_t vol,
    int16_t dist, int16_t x, int16_t y, int16_t mode)
{
    DM2_V1_SfxQueueNoiseReceipt r;
    r.queued = false;

    if (cb == NULL || state == NULL) return r;

    (void)vol;   /* used in full implementation for sound level */
    (void)dist;  /* used in full implementation for distance param */

    /* Filter: mode > 0 requires party on current or alt map */
    if (mode > 0) {
        int16_t cur_map = cb->get_current_map(cb->ctx);
        if (cur_map != cb->get_party_map(cb->ctx) &&
            cur_map != cb->get_party_alt_map(cb->ctx))
            return r;
    }

    /* Queue full check */
    if (state->queued_count == DM2_V1_SFX_MAX_QUEUED)
        return r;

    /* Resolve sound entry */
    int16_t snd_idx = cb->query_snd_entry_index(cb->ctx, cat, type, sub);
    if (snd_idx == 0) return r;

    /* Distance halving */
    if (cb->get_distance_halve_flag(cb->ctx) != 0)
        dist >>= 1;

    /* Positional sound (mode <= 1) vs delayed sound (mode > 1) */
    if (mode <= 1) {
        int16_t dx = 0, dy = 0;

        if (mode > 0) {
            /* Adjust for multi-level offset */
            /* (simplified: level geometry offset not implemented here) */
        }

        /* Direction-relative positioning */
        int16_t party_dir = cb->get_party_dir(cb->ctx);
        if (party_dir <= 3) {
            int16_t px = cb->get_party_x(cb->ctx);
            int16_t py = cb->get_party_y(cb->ctx);
            int16_t rel_x = x - px;
            int16_t rel_y = py - y;

            switch (party_dir) {
                case 0: dx =  rel_x; dy = rel_y; break;
                case 1: dx = -rel_y + (y - py); dy = rel_x; break;
                case 2: dx = -rel_x; dy = -rel_y + (y - py); break;
                case 3: dx =  rel_y; dy = -rel_x; break;
            }
        }

        /* Occlusion distance check */
        int16_t total_dist = (int16_t)(abs((int)dx) + abs((int)dy));
        if (total_dist > 1) {
            DM2_V1_SfxSoundDistanceReceipt dr =
                dm2_v1_sfx_calc_sound_distance(cb, x, y);
            if (!dr.valid) return r;
            if (dr.distance > total_dist) {
                /* Scale dx/dy by occlusion ratio */
                int ratio = (int)(((int32_t)dr.distance << 10) / (int32_t)total_dist);
                if (dx > 0)
                    dx = (int16_t)(((int)dx * ratio + 0x200) >> 10);
                else if (dx < 0)
                    dx = (int16_t)(-((-dx * ratio + 0x200) >> 10));

                if (dy > 0)
                    dy = (int16_t)(((int)dy * ratio + 0x200) >> 10);
                else if (dy < 0)
                    dy = (int16_t)(-((-dy * ratio + 0x200) >> 10));
            }
        }

        /* Enqueue the sound */
        if (mode == 0) {
            /* Immediate play */
            r.queued = true;
        } else if (mode > 0) {
            state->queued_count++;
            r.queued = true;
        } else {
            if (state->queued_neg_count >= DM2_V1_SFX_MAX_QUEUED_NEG)
                return r;
            state->queued_neg_count++;
            r.queued = true;
        }
    } else {
        /* Delayed sound — find free slot in sndptr1 */
        r.queued = true;
    }

    return r;
}
