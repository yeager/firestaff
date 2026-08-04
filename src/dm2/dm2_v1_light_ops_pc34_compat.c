/* DM2 V1 light operations — skproject c_light.cpp. */

#include "dm2_v1_light_ops_pc34_compat.h"
#include <stddef.h>

/* ---- DM2_RECALC_LIGHT_LEVEL (c_light.cpp:16-198) ---- */

void dm2_v1_recalc_light_level_pc34(
    const DM2_V1_RecalcLightLevelCallbacks *cb, void *ctx)
{
    if (!cb)
        return;

    /* c_light.cpp:24 — check if current map tile has light flag (byte 0xD, mask 0xF0) */
    uint8_t tile_byte = cb->get_map_tile_byte(ctx, cb->map_index, 0x0D);
    int16_t tile_light_flag = (int16_t)((tile_byte >> 4) & 0x0F);

    /* c_light.cpp:30-90 — collect light-emitting items from party hands.
     * Items with GDAT DBSPEC flag 0x10 emit light.
     * Collect their charges, sort descending, accumulate via table. */
    int16_t charges[8]; /* max 4 heroes * 2 hands */
    int charge_count = 0;

    int16_t hero_count = cb->get_heros_in_party(ctx);
    for (int16_t hero = 0; hero < hero_count; hero++) {
        for (int hand = 0; hand < 2; hand++) {
            int16_t item = cb->get_hero_item(ctx, hero, hand);
            if (item < 0)
                continue;
            uint16_t dbspec = cb->query_gdat_dbspec_word(ctx, item, 0x10);
            if ((dbspec & 0x10) == 0)
                continue;
            int16_t charge = cb->add_item_charge(ctx, item, 0);
            if (charge > 0 && charge_count < 8)
                charges[charge_count++] = charge;
        }
    }

    /* c_light.cpp:92-100 — sort charges descending (simple insertion sort) */
    for (int i = 1; i < charge_count; i++) {
        int16_t key = charges[i];
        int j = i - 1;
        while (j >= 0 && charges[j] < key) {
            charges[j + 1] = charges[j];
            j--;
        }
        charges[j + 1] = key;
    }

    /* c_light.cpp:102-120 — accumulate light via table1d6702 with shifting.
     * Each successive item's contribution is shifted right by 1. */
    int16_t accumulated = 0;
    for (int i = 0; i < charge_count; i++) {
        int16_t charge_val = charges[i];
        if (charge_val >= cb->table1d6702_size)
            charge_val = (int16_t)(cb->table1d6702_size - 1);
        if (charge_val < 0)
            charge_val = 0;
        int16_t contrib = cb->table1d6702[charge_val];
        if (i > 0)
            contrib = (int16_t)(contrib >> i);
        accumulated = (int16_t)(accumulated + contrib);
    }

    /* c_light.cpp:125 — add tile light flag contribution */
    accumulated = (int16_t)(accumulated + tile_light_flag);

    /* c_light.cpp:130 — add ambient light (v1e0974) */
    accumulated = (int16_t)(accumulated + cb->v1e0974);

    /* c_light.cpp:135 — add savegame light contribution */
    accumulated = (int16_t)(accumulated + cb->savegame_light);

    /* c_light.cpp:140-155 — query GDAT entry 8/v1d6c02/11/0x67 for map-specific adjustment */
    int16_t gdat_adj = cb->query_gdat_entry_data_index(
        ctx, 8, cb->v1d6c02, 11, 0x67);
    if (gdat_adj != 0)
        accumulated = (int16_t)(accumulated + gdat_adj);

    /* c_light.cpp:160-180 — weather light adjustment */
    if (cb->v1e147f != 0 && cb->v1e1476 != 0) {
        int16_t weather_idx = cb->v1e1480;
        if (weather_idx >= 0 && weather_idx < cb->table1d6712_size) {
            int16_t weather_adj = cb->table1d6712[weather_idx];
            accumulated = (int16_t)(accumulated + weather_adj);
        }
    }

    /* c_light.cpp:190 — clamp to 0-5 */
    accumulated = dm2_v1_between_value(0, 5, accumulated);

    /* c_light.cpp:195 — store result in ddat.v1e0286 */
    cb->set_light_level(ctx, accumulated);
}

void dm2_v1_proceed_light(
    uint16_t light_type, int16_t intensity,
    const DM2_V1_ProceedLightCallbacks *cb, void *ctx)
{
    if (!cb)
        return;

    int dir_mult = 1;
    intensity = (int16_t)(intensity + 1);
    intensity = dm2_v1_between_value(32, 256, intensity);
    int16_t step = (int16_t)(intensity / 8);
    if (step < 8) step = 8;

    int16_t r2 = (int16_t)(step - 8);
    uint16_t delay;

    if (light_type == 0x06) {
        /* Darkness spell */
        delay = (uint16_t)(16 * r2 + 16);
        dir_mult = -2;
    } else if (light_type == 0x26) {
        /* Torch-class */
        delay = (uint16_t)(((step - 3) << 7) + 2000);
        step = (int16_t)(step >> 2);
        step = (int16_t)(step + 1);
    } else if (light_type == 0x27) {
        /* Bright light */
        delay = (uint16_t)((r2 << 9) + 10000);
    } else {
        return;
    }

    if (light_type != 0x06) {
        /* Non-darkness: halve and decrement */
        step = (int16_t)(step >> 1);
        step--;
    }

    /* Queue light timer (type 0x46) */
    int16_t timer_val;
    if (light_type != 0x06)
        timer_val = (int16_t)-step;
    else
        timer_val = step;

    cb->queue_light_timer(ctx, timer_val,
                          (uint32_t)delay + cb->game_tick);

    /* Apply initial light delta */
    if (step >= 0 && step < cb->light_table_size) {
        int16_t light_delta = (int16_t)(cb->light_table[step] * dir_mult);
        *cb->global_light = (int16_t)(*cb->global_light + light_delta);
    }

    cb->recalc_light(ctx);
}
