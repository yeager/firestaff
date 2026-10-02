/* DM2 V1 light operations — skproject c_light.cpp. */

#include "dm2_v1_light_ops_pc34_compat.h"
#include <stddef.h>

int dm2_v1_mode7_light_prepare(uint16_t source_radius,
                              DM2_V1_Mode7LightPreparation *out)
{
    if (!out) return 0;
    out->v1e0974 = 0;
    out->v1e0978 = 0;
    out->traversal_required = source_radius != 0u;
    out->radius = (uint8_t)(source_radius > 8u ? 8u : source_radius);
    return 1;
}

int dm2_v1_mode7_action23_samples_tile(uint16_t cached_tile_state)
{
    return (cached_tile_state & 0x10u) != 0u;
}

int dm2_v1_mode7_go_there_tile_admission(uint8_t raw_tile,
                                          int first_record_link)
{
    unsigned type = raw_tile >> 5;
    /* SK1C9A.cpp:3190-3340 maps GO_THERE tile classes to capability
     * bits. Action 23 sets v1e0576=0x227 at :6873-6882. Empty class-0
     * squares have bit 1 and no record branch. Classes 3, 6 and 7
     * have no overlapping capability bit (or are rejected outright).
     * Other branches need movement and record evidence before admission. */
    if (type == 0u && first_record_link == -1) return 1;
    if (type == 3u || type == 6u || type == 7u) return 0;
    return -1;
}

int dm2_v1_mode7_go_there_class1_raw30_admission(
    uint8_t raw_tile, int no_creature_proven, int party_square)
{
    /* SK1C9A.cpp:3160-3190 gives class 1 capability 0x2, admitted by
     * action 23's 0x227 mask. :3380-3445 then tests the party-square
     * blocker (0x800) and a destination creature (0x1000). The latter is
     * absent only after a complete, bounded record-chain proof. */
    if (raw_tile != 0x30u || !no_creature_proven) return -1;
    return party_square ? 0 : 1;
}

int dm2_v1_mode7_action23_visit_tile(
    uint16_t cached_tile_state, uint8_t radius,
    int16_t map, int16_t x, int16_t y,
    DM2_V1_Mode7AddBackgroundLight add_background_light, void *ctx)
{
    if (!dm2_v1_mode7_action23_samples_tile(cached_tile_state)) return 0;
    if (!add_background_light || radius == 0u || radius > 8u) return -1;
    /* SK1C9A.cpp:8983-8995 passes vl_58, vw_f8, vo_f4, vw_12c and
     * source flags 4, in that order, after testing v1e08ae bit 0x10. */
    return add_background_light(ctx, radius, map, x, y, 4u) ? 1 : -1;
}

int dm2_v1_mode7_light_accumulate_tile(
    uint8_t distance, int16_t tile_light, int16_t darkness,
    int16_t weather_light, int16_t *v1e0974, int16_t *v1e0978)
{
    static const int16_t weather_falloff[9] = {
        0, 6, 14, 30, 42, 54, 76, 88, 96
    };
    static const int16_t source_falloff[6] = {
        0, 10, 22, 45, 70, 90
    };
    int32_t base = tile_light;
    int32_t dark = darkness;
    int32_t weather = weather_light;
    uint8_t source_distance = distance > 5u ? 5u : distance;

    if (!v1e0974 || !v1e0978) return 0;

    /* SKProject sklight.cpp:435-481 and dm2data.cpp:87-96. The weather
     * term vanishes beyond eight; nonzero source terms retain a floor of
     * two after their distance subtraction. */
    if (distance > 8u)
        weather = 0;
    else if (weather != 0) {
        weather -= weather_falloff[distance];
        if (weather < 3) weather = 3;
    }
    if (base != 0) {
        base -= source_falloff[source_distance];
        if (base < 2) base = 2;
    }
    if (dark != 0) {
        dark -= source_falloff[source_distance];
        if (dark < 2) dark = 2;
    }
    *v1e0974 = (int16_t)(uint16_t)((uint32_t)(uint16_t)*v1e0974 +
                                    (uint32_t)(uint16_t)(base + weather));
    *v1e0978 = (int16_t)(uint16_t)((uint32_t)(uint16_t)*v1e0978 +
                                    (uint32_t)(uint16_t)dark);
    return 1;
}

int dm2_v1_mode7_flags4_floor_terms(
    const DM2_V1_CLightFlags4FloorReceipt *floor,
    uint8_t weather_index, uint8_t weather_delta,
    int16_t *out_tile_light, int16_t *out_weather_light)
{
    static const int16_t weather_scale[6] = {99, 75, 50, 25, 1, 0};
    uint16_t light;
    unsigned index;

    if (!floor || !floor->valid || floor->source_flags != 4u ||
        !out_tile_light || !out_weather_light) return 0;
    *out_tile_light = 0;
    *out_weather_light = 0;
    if ((floor->floor_ornament_word & 0xffu) == 0xffu ||
        floor->floor_light_word == 0u) return 1;

    light = floor->floor_light_word & 0x7fffu;
    if (floor->weather_light_word != 0u) {
        /* sklight.cpp:255-278: the weather-scaled ornament is accumulated
         * separately before the terminal distance falloff. */
        index = (unsigned)weather_index + (unsigned)weather_delta;
        if (index > 5u) index = 5u;
        *out_weather_light = (int16_t)((uint32_t)light *
            (uint32_t)weather_scale[index] / 100u);
    } else if ((floor->floor_light_word & 0x8000u) == 0u ||
               (floor->floor_ornament_word & 0xff00u) != 0u) {
        /* The high-bit source gate requires a nonzero animation/frame byte
         * when no weather branch is present. */
        *out_tile_light = (int16_t)light;
    }
    return 1;
}

int dm2_v1_mode7_flags4_class2_terms(
    const DM2_V1_CLightStoneRoomReceipt *room,
    int16_t *out_tile_light, int16_t *out_darkness,
    int16_t *out_weather_light)
{
    if (!room || !room->valid ||
        (room->raw_tile >> 5) != 2u ||
        room->first_record_link != DM2_THING_NULL_MARKER ||
        (room->source_tile_type != 1u && room->source_tile_type != 2u) ||
        !out_tile_light || !out_darkness || !out_weather_light) return 0;
    /* sklight.cpp:231-435: class 1/2 bypasses the class-0 flags-4 branch.
     * The remaining floor and creature probes require flags 1 and 2,
     * respectively; action 0x17 passes only 4. */
    *out_tile_light = 0;
    *out_darkness = 0;
    *out_weather_light = 0;
    return 1;
}

int dm2_v1_mode7_flags4_class5_terms(
    const DM2_V1_CLightStoneRoomReceipt *room,
    int16_t *out_tile_light, int16_t *out_darkness,
    int16_t *out_weather_light)
{
    if (!room || !room->valid || (room->raw_tile >> 5) != 5u ||
        ((room->first_record_link >> 10) & 0x0fu) != 1u ||
        (room->source_tile_type != 1u && room->source_tile_type != 2u &&
         room->source_tile_type != 5u) || !out_tile_light ||
        !out_darkness || !out_weather_light) return 0;
    /* sklight.cpp:304-433: ceiling, teleporter and DBE/DBF scan are all
     * nested under flags & 1; creature light requires flags & 2. Action
     * 23's edge passes only 4, leaving every source term zero. */
    *out_tile_light = 0;
    *out_darkness = 0;
    *out_weather_light = 0;
    return 1;
}

static int dm2_v1_mode7_flags4_class1_terms(
    const DM2_V1_CLightStoneRoomReceipt *room,
    int16_t *out_tile_light, int16_t *out_darkness,
    int16_t *out_weather_light)
{
    if (!room || !room->valid || room->raw_tile != 0x30u ||
        ((room->first_record_link >> 10) & 0x0fu) != 3u ||
        room->source_tile_type != 1u || !out_tile_light ||
        !out_darkness || !out_weather_light) return 0;
    /* sklight.cpp:231-435: flags 4 bypasses class-0 floor light,
     * flags-1 ceiling and record light, and flags-2 creature light. */
    *out_tile_light = 0;
    *out_darkness = 0;
    *out_weather_light = 0;
    return 1;
}

static int dm2_v1_mode7_tile_cache_refresh(
    DM2_V1_Mode7TileCache *cache, int map, int x, int y,
    DM2_V1_Mode7ReadTile read_tile, void *ctx)
{
    uint8_t tile;
    if (!cache || !read_tile || map < 0 || map >= 64 ||
        x < 0 || x >= 32 || y < 0 || y >= 32) return 0;
    if (cache->valid && cache->map == map && cache->x == x &&
        cache->y == y) return 1;
    if (!read_tile(ctx, map, x, y, &tile)) return 0;
    cache->map = (int16_t)map;
    cache->x = (int16_t)x;
    cache->y = (int16_t)y;
    cache->tile = tile;
    cache->valid = 1;
    return 1;
}

int dm2_v1_mode7_tile_cache_start(
    DM2_V1_Mode7TileCache *cache, int map, int x, int y,
    DM2_V1_Mode7ReadTile read_tile, void *ctx)
{
    if (!cache) return 0;
    /* SK1C9A.cpp:6699-6703 calls 19f0_045a for the start square. */
    return dm2_v1_mode7_tile_cache_refresh(
        cache, map, x, y, read_tile, ctx);
}

int dm2_v1_mode7_tile_cache_node(
    DM2_V1_Mode7TileCache *cache, uint8_t effective_flags,
    int map, int x, int y, DM2_V1_Mode7ReadTile read_tile, void *ctx)
{
    if (!cache || !cache->valid) return 0;
    /* SK1C9A.cpp:8622-8638 refreshes the live cache only if vb_140 has
     * both bits 2 and 8. Otherwise action 0x17 observes the prior tile. */
    if ((effective_flags & 0x0au) != 0x0au) return 1;
    return dm2_v1_mode7_tile_cache_refresh(
        cache, map, x, y, read_tile, ctx);
}

int dm2_v1_mode7_tile_cache_action23_gate(
    const DM2_V1_Mode7TileCache *cache)
{
    if (!cache || !cache->valid) return 0;
    return dm2_v1_mode7_action23_samples_tile(cache->tile);
}

static int dm2_v1_mode7_flags3_class2_terms(
    const DM2_V1_CLightStoneRoomReceipt *room,
    const DM2_V1_Mode7Flags3Evidence *evidence,
    uint8_t weather_index, uint8_t weather_delta,
    int16_t *tile_light, int16_t *weather_light)
{
    static const int16_t weather_scale[6] = {99, 75, 50, 25, 1, 0};
    uint16_t light;
    unsigned index;
    if (!room || !room->valid ||
        !(((room->raw_tile >> 5) == 2u &&
           room->first_record_link == DM2_THING_NULL_MARKER) ||
          ((room->raw_tile >> 5) == 5u &&
           ((room->first_record_link >> 10) & 0x0fu) == 1u &&
           evidence && evidence->record_chain_known_no_darkness)) ||
        (room->source_tile_type != 1u && room->source_tile_type != 2u) ||
        !evidence || !evidence->valid ||
        !evidence->teleporter_detail_known ||
        !evidence->creature_query_known || !evidence->ceiling_gdat_known ||
        !tile_light || !weather_light) return 0;
    *tile_light = 0;
    *weather_light = 0;
    /* sklight.cpp:292-350: flags 1 reads the class-1/2 ceiling ornament
     * before flags 2 reads the live creature F8 term. The no-record room
     * proves the DBE/DBF darkness chain empty for this narrow branch. */
    if ((room->ceiling_ornament_word & 0xffu) != 0xffu &&
        evidence->ceiling_gdat_light_word != 0u) {
        light = evidence->ceiling_gdat_light_word & 0x7fffu;
        if (evidence->teleporter_present) {
            index = (unsigned)weather_index + (unsigned)weather_delta;
            if (index > 5u) index = 5u;
            *weather_light = (int16_t)((uint32_t)light *
                (uint32_t)weather_scale[index] / 100u);
        } else if ((evidence->ceiling_gdat_light_word & 0x8000u) == 0u ||
                   (room->ceiling_ornament_word & 0xff00u) != 0u) {
            *tile_light = (int16_t)light;
        }
    }
    if (evidence->creature_present)
        *tile_light = (int16_t)((uint16_t)*tile_light +
            (evidence->creature_f8_word & 0x7fffu));
    return 1;
}

int dm2_v1_mode7_on_node(
    const DM2_V1_Mode7Action23Node *node,
    int16_t *v1e0974, int16_t *v1e0978)
{
    int16_t tile_light = 0, darkness = 0, weather_light = 0;
    if (!node || !v1e0974 || !v1e0978) return -1;
    if (node->source_flags != 3u && node->source_flags != 4u) return -1;
    /* Both SK1C9A action-23 call sites test v1e08ae bit 0x10 before
     * entering ADD_BACKGROUND_LIGHT_FROM_TILE. */
    if (!dm2_v1_mode7_action23_samples_tile(node->cached_tile)) return 0;
    if (node->source_flags == 3u) {
        /* sklight.cpp:316-323 exits before teleporter, record and creature
         * probes when the ceiling F8 query returns zero. */
        if (node->stone_room && node->stone_room->valid &&
            (node->stone_room->raw_tile >> 5) == 1u &&
            node->stone_room->source_tile_type == 1u &&
            node->prepass && node->prepass->valid &&
            node->prepass->ceiling_gdat_known &&
            node->prepass->ceiling_gdat_light_word == 0u)
            return 1;
        if (node->distance > 8u || node->floor ||
            !dm2_v1_mode7_flags3_class2_terms(
                node->stone_room, node->prepass,
                node->weather_index, node->weather_delta,
                &tile_light, &weather_light)) return -1;
        return dm2_v1_mode7_light_accumulate_tile(
            node->distance, tile_light, 0, weather_light,
            v1e0974, v1e0978) ? 1 : -1;
    }
    if ((node->effective_flags & 2u) == 0u) return 0;
    if (node->distance > 8u ||
        (!!node->floor == !!node->stone_room)) return -1;
    if (node->floor) {
        if (!dm2_v1_mode7_flags4_floor_terms(
                node->floor, node->weather_index, node->weather_delta,
                &tile_light, &weather_light)) return -1;
    } else if (!dm2_v1_mode7_flags4_class1_terms(
                   node->stone_room, &tile_light, &darkness,
                   &weather_light) &&
               !dm2_v1_mode7_flags4_class2_terms(
                   node->stone_room, &tile_light, &darkness,
                   &weather_light) &&
               !dm2_v1_mode7_flags4_class5_terms(
                   node->stone_room, &tile_light, &darkness,
                   &weather_light)) return -1;
    if (!dm2_v1_mode7_light_accumulate_tile(
            node->distance, tile_light, darkness, weather_light,
            v1e0974, v1e0978)) return -1;
    return 1;
}

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
         * still passes through the source boolean v1e0978 gate and clamp. */
        light_level = (int16_t)(1 - (cb->v1e0978 > 0x0c ? 1 : 0));
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
    /* sklight.cpp:35-42 passes the record as an unsigned 16-bit handle.
     * A set sign bit is part of a valid record id, not an absence marker. */
    if ((cb->query_gdat_dbspec_word(ctx, item, 0) & 0x10u) != 0u) {
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
            if ((cb->query_gdat_dbspec_word(ctx, item, 0) & 0x10u) == 0u)
                continue;
            int16_t charge = cb->add_item_charge(ctx, item, 0);
            if (charge >= 0 && charge_count < (int)(sizeof(charges) / sizeof(charges[0])))
                charges[charge_count++] = charge;
        }
    }

    /* sklight.cpp:87-112 makes exactly one adjacent pass, swapping only
     * when the left charge is greater. Later charges are not fully sorted. */
    for (int i = 0; i + 1 < charge_count; ++i) {
        if (charges[i] > charges[i + 1]) {
            int16_t tmp = charges[i];
            charges[i] = charges[i + 1];
            charges[i + 1] = tmp;
        }
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

    /* sklight.cpp:190-194 assigns the comparison result to RG1Blo, the low
     * byte of RG1L, before masking RG1L to a byte.  The subtraction is thus
     * the boolean (v1e0978 > 0x0c), not the accumulator itself. */
    source_light_modifier = cb->v1e0978 > 0x0c ? 1 : 0;
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
