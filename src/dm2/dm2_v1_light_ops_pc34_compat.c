/* DM2 V1 light operations — skproject c_light.cpp. */

#include "dm2_v1_light_ops_pc34_compat.h"
#include <stddef.h>

/* ---- DM2_RECALC_LIGHT_LEVEL (c_light.cpp:16-198) ---- */

void dm2_v1_recalc_light_level_pc34(
    const DM2_V1_RecalcLightLevelCallbacks *cb, void *ctx)
{
    int16_t accumulated;
    int16_t light_level;
    int16_t source_light_modifier;
    uint8_t tile_byte;

    if (!cb || !cb->get_map_tile_byte || !cb->set_light_level)
        return;

    /* SKProject src/v5/sklight.cpp:24-198 (the same routine is retained in
     * the older c_light.cpp disassembly): the high nibble at map offset 0x0D
     * selects the item/weather accumulation branch.  It is a branch guard,
     * not a light amount to add to the result. */
    tile_byte = cb->get_map_tile_byte(ctx, cb->map_index, 0x0D);
    if ((tile_byte & 0xf0u) == 0u) {
        /* sklight.cpp:184-198: the no-light-tile branch starts at one and
         * still passes through the source v1e0978 modifier and clamp. */
        light_level = (int16_t)(1 - cb->v1e0978);
        cb->set_light_level(ctx, dm2_v1_between_value(0, 5, light_level));
        return;
    }

    if (!cb->get_leader_item || !cb->get_heros_in_party ||
        !cb->get_hero_item || !cb->query_gdat_dbspec_word ||
        !cb->add_item_charge || !cb->query_gdat_entry_data_index ||
        !cb->table1d6702 || cb->table1d6702_size <= 0 ||
        !cb->table1d6712 || cb->table1d6712_size <= 5) {
        return;
    }

    /* sklight.cpp:39-90 — the leader's savegame hand is examined first,
     * followed by both hands of every active hero.  The source table has nine
     * slots, not eight: one leader hand plus four heroes and two hands each. */
    int16_t charges[9];
    int charge_count = 0;
    int16_t item = cb->get_leader_item(ctx);
    if (item >= 0 &&
        (cb->query_gdat_dbspec_word(ctx, item, 0) & 0x10u) != 0u) {
        int16_t charge = cb->add_item_charge(ctx, item, 0);
        if (charge >= 0 && charge_count < (int)(sizeof(charges) / sizeof(charges[0])))
            charges[charge_count++] = charge;
    }

    int16_t hero_count = cb->get_heros_in_party(ctx);
    if (hero_count < 0) return;
    if (hero_count > 4) hero_count = 4;
    for (int16_t hero = 0; hero < hero_count; hero++) {
        for (int hand = 0; hand < 2; hand++) {
            item = cb->get_hero_item(ctx, hero, hand);
            if (item < 0)
                continue;
            if ((cb->query_gdat_dbspec_word(ctx, item, 0) & 0x10u) == 0u)
                continue;
            int16_t charge = cb->add_item_charge(ctx, item, 0);
            if (charge >= 0 && charge_count < (int)(sizeof(charges) / sizeof(charges[0])))
                charges[charge_count++] = charge;
        }
    }

    /* sklight.cpp:92-113 — source bubble pass, descending by charge. */
    for (int i = 1; i < charge_count; i++) {
        int16_t key = charges[i];
        int j = i - 1;
        while (j >= 0 && charges[j] < key) {
            charges[j + 1] = charges[j];
            j--;
        }
        charges[j + 1] = key;
    }

    /* sklight.cpp:115-157 — table1d6702 contribution starts with a six-bit
     * left shift and is divided by 64; the shift count decreases per item.
     * This is materially different from a per-item right shift. */
    accumulated = 0;
    for (int i = 0; i < charge_count; i++) {
        int16_t charge_val = charges[i];
        if (charge_val >= cb->table1d6702_size)
            charge_val = (int16_t)(cb->table1d6702_size - 1);
        if (charge_val < 0)
            charge_val = 0;
        int shift = 6 - i;
        int32_t contrib = cb->table1d6702[charge_val];
        if (shift > 0)
            contrib = (contrib << shift) >> 6;
        else
            contrib = 0;
        accumulated = (int16_t)(accumulated + contrib);
    }

    /* sklight.cpp:130-157 — source global accumulators and GDAT map delta. */
    accumulated = (int16_t)(accumulated + cb->v1e0974);
    accumulated = (int16_t)(accumulated + cb->savegame_light);
    int16_t gdat_adj = cb->query_gdat_entry_data_index(
        ctx, 8, cb->v1d6c02, 11, 0x67);
    accumulated = (int16_t)(accumulated + gdat_adj);

    /* sklight.cpp:158-177 — weather indexes table1d6712 with
     * v1e1480+v1e1476, clamped to five. */
    if (cb->v1e147f != 0) {
        int16_t weather_idx = dm2_v1_between_value(
            0, 5, (int16_t)(cb->v1e1480 + cb->v1e1476));
        accumulated = (int16_t)(accumulated + cb->table1d6712[weather_idx]);
    }

    /* sklight.cpp:160-183 — convert accumulated light through the inverse
     * table and then apply the map-specific minimum at dtWordValue/0x68. */
    light_level = 0;
    while (light_level < 5 && cb->table1d6712[light_level] >= accumulated)
        ++light_level;
    gdat_adj = cb->query_gdat_entry_data_index(ctx, 8, cb->v1d6c02, 11, 0x68);
    if (gdat_adj > light_level)
        light_level = gdat_adj;
    if (cb->v1e147f != 0 && cb->v1e024c != 0)
        light_level = 0;

    /* sklight.cpp:186-190 — the original narrows v1e0978 to one when it is
     * above 0x0c before subtracting it.  It is not an unrestricted host
     * light delta; preserving the source normalization avoids turning an
     * authenticated high modifier into an artificial black frame. */
    source_light_modifier = cb->v1e0978 > 0x0c ? 1 : cb->v1e0978;
    light_level = (int16_t)(light_level - source_light_modifier);
    cb->set_light_level(ctx, dm2_v1_between_value(0, 5, light_level));
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
