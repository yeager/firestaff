/*
 * dm2_v1_querydb_pc34_compat.c — DM2 database query module.
 *
 * Ports query functions from skproject c_querydb.cpp.
 * Pure computation functions are implemented directly; callback-dependent
 * functions call through the DM2_V1_QueryDbCallbacks vtable.
 *
 * Source: skproject/SKWINSPX/src/v4/c_querydb.cpp
 */

#include "dm2_v1_querydb_pc34_compat.h"

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

/* ── Section 1: Pure computation — coordinate and classification ──── */

void dm2_v1_query_098d_000f(int16_t x, int16_t y, int16_t pos,
                            int16_t *out_w1, int16_t *out_w2)
{
    *out_w1 = pos % 5 + 4 * x;
    *out_w2 = pos / 5 + 4 * y;
}

bool dm2_v1_is_cls1_critical_for_load(int8_t cls1)
{
    return (cls1 == 0x1b || cls1 == 0x06 || cls1 == 0x05);
}

int32_t dm2_v1_dir_from_5x5_pos(int32_t pos)
{
    pos &= 0xffff;
    if (pos == 0x06) return 0;
    if (pos == 0x08) return 1;
    if (pos == 0x12) return 2;
    if (pos == 0x10) return 3;
    return (pos == 0x0c) ? 4 : -1;
}

/* ── Section 2: Tile queries ──────────────────────────────────────── */

int32_t dm2_v1_is_tile_blocked(int32_t tile_value)
{
    int type = (tile_value >> 5) & 7;
    if (type < 4) {
        return (type == 0) ? 1 : 0;
    }
    if (type == 4) {
        int subtype = tile_value & 7;
        return (subtype == 0 || subtype == 1 || subtype == 5) ? 0 : 1;
    }
    if (type == 5) {
        return 0;
    }
    if (type == 6) {
        if (tile_value & 4) return 0;
        return (tile_value & 1) ? 0 : 1;
    }
    if (type == 7) {
        return 1;
    }
    return 0;
}

int32_t dm2_v1_query_tile_type(int32_t tile_value,
                               const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:636 */
    (void)cb; (void)ctx;
    return (tile_value >> 5) & 7;
}

int32_t dm2_v1_query_tile_flags(int32_t tile_value,
                                const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:640 */
    (void)cb; (void)ctx;
    return tile_value & 0x1f;
}

int32_t dm2_v1_query_tile_is_pit(int32_t tile_value,
                                 const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:684 */
    (void)cb; (void)ctx;
    return ((tile_value >> 5) & 7) == 3 ? 1 : 0;
}

int32_t dm2_v1_query_tile_is_stairs(int32_t tile_value,
                                    const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:690 */
    (void)cb; (void)ctx;
    return ((tile_value >> 5) & 7) == 5 ? 1 : 0;
}

int32_t dm2_v1_query_tile_is_teleporter(int32_t tile_value,
                                        const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:696 */
    (void)cb; (void)ctx;
    return ((tile_value >> 5) & 7) == 7 ? 1 : 0;
}

/* ── Section 3: Door queries ──────────────────────────────────────── */

int32_t dm2_v1_query_door_damage_resist(int32_t door_type,
                                        const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 14, (int8_t)door_type, 11, 0x0f);
}

int32_t dm2_v1_get_door_stat_0x10(int32_t door_type,
                                  const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 14, (int8_t)door_type, 11, 14);
}

int32_t dm2_v1_get_graphics_for_door(int32_t door_type,
                                     const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 14, (int8_t)door_type, 11, 13);
}

int32_t dm2_v1_query_0cee_3275(int32_t door_type,
                               const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 14, (int8_t)door_type, 11, 16);
}

int32_t dm2_v1_query_door_strength(int32_t door_type,
                                   const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    int16_t v = cb->query_gdat_entry_data_index(ctx, 14, (int8_t)door_type, 11, 17);
    if (v != 0) return (int32_t)v;
    return dm2_v1_query_0cee_3275(door_type, cb, ctx) == 0 ? 6 : 1;
}

int32_t dm2_v1_query_door_is_bashable(int32_t door_type,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:730 */
    (void)cb; (void)ctx; (void)door_type;
    return 0;
}

int32_t dm2_v1_query_door_ornament(int32_t door_type,
                                   const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:738 */
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 14, (int8_t)door_type, 11, 12);
}

int32_t dm2_v1_query_door_is_mirrored(int32_t door_type,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:745 */
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 14, (int8_t)door_type, 11, 18);
}

/* ── Section 4: Wall ornament queries ─────────────────────────────── */

int32_t dm2_v1_is_wall_ornate_alcove(int32_t ornament,
                                     const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* ReDMCSB/skproject c_querydb.cpp:635 DM2_IS_WALL_ORNATE_ALCOVE:
       ornament byte 0xff (-1) is a guard sentinel meaning "no ornament";
       return 0 without querying the database. */
    if ((uint8_t)ornament == 0xff) return 0;
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 9, (int8_t)ornament, 11, 10);
}

int32_t dm2_v1_query_wall_ornament_can_destroy(int32_t ornament,
                                               const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:760 */
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 9, (int8_t)ornament, 11, 11);
}

int32_t dm2_v1_query_wall_ornament_is_exit(int32_t ornament,
                                           const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:768 */
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 9, (int8_t)ornament, 11, 12);
}

int32_t dm2_v1_query_wall_ornament_type(int32_t ornament,
                                        const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:776 */
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 9, (int8_t)ornament, 11, 0x0f);
}

int32_t dm2_v1_query_floor_ornament_is_drainable(int32_t ornament,
                                                 const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:784 */
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 10, (int8_t)ornament, 11, 10);
}

/* ── Section 5: Creature queries ──────────────────────────────────── */

int32_t dm2_v1_get_creature_weight(int32_t creature_type,
                                   const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x1d];
}

int32_t dm2_v1_query_0cee_2e09(int32_t type,
                               const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, type & 0xffff);
    if (!p) return 0;
    return (int32_t)(*(int16_t *)(p + 0x20));
}

int32_t dm2_v1_query_0cee_2df4(int32_t type,
                               const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, type & 0xffff);
    if (!p) return 0;
    return (int32_t)(*(int16_t *)(p + 0x1e));
}

int32_t dm2_v1_query_creature_hp(int32_t creature_type,
                                 const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:998 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(*(int16_t *)(p + 0x04));
}

int32_t dm2_v1_query_creature_speed(int32_t creature_type,
                                    const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1010 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x09];
}

int32_t dm2_v1_query_creature_attack_power(int32_t creature_type,
                                           const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1022 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x0a];
}

int32_t dm2_v1_query_creature_defense(int32_t creature_type,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1034 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x07];
}

int32_t dm2_v1_query_creature_poisonous(int32_t creature_type,
                                        const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1046 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x0e];
}

int32_t dm2_v1_query_creature_can_see_invis(int32_t creature_type,
                                            const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1058 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x11];
}

int32_t dm2_v1_query_creature_absorb(int32_t creature_type,
                                     const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1070 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x10];
}

int32_t dm2_v1_query_creature_anim_count(int32_t creature_type,
                                         const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1082 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x16];
}

int32_t dm2_v1_query_creature_fear_resist(int32_t creature_type,
                                          const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1094 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x0f];
}

int32_t dm2_v1_query_creature_experience(int32_t creature_type,
                                         const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1106 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(*(int16_t *)(p + 0x14));
}

int32_t dm2_v1_query_creature_sz(int32_t creature_type,
                                 const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1118 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x08];
}

int32_t dm2_v1_query_creature_fire_resist(int32_t creature_type,
                                          const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1130 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x0b];
}

int32_t dm2_v1_query_creature_water_resist(int32_t creature_type,
                                           const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1142 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x0c];
}

int32_t dm2_v1_query_creature_air_resist(int32_t creature_type,
                                         const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1154 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x0d];
}

int32_t dm2_v1_query_creature_earth_resist(int32_t creature_type,
                                           const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1166 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x12];
}

int32_t dm2_v1_query_creature_bravery(int32_t creature_type,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1178 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x13];
}

int32_t dm2_v1_query_creature_swallow(int32_t creature_type,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1190 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x17];
}

int32_t dm2_v1_query_creature_items_mask(int32_t creature_type,
                                         const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1202 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(*(int16_t *)(p + 0x18));
}

int16_t dm2_v1_querydb_creature_blit_recti(int16_t n, int16_t rotate, int16_t wb,
                                           const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    if (!cb || !cb->rotate_5x5_pos) return 0;
    int16_t rotated = cb->rotate_5x5_pos(ctx, wb, (uint16_t)rotate);
    return rotated + 25 * n + 5000;
}

int32_t dm2_v1_query_creature_gfx_idx(int32_t creature_type,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:5002 */
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 0x0f, (int8_t)creature_type, 11, 13);
}

/* ── Section 6: Item and record queries ───────────────────────────── */

int32_t dm2_v1_is_miscitem_currency(int32_t record,
                                    const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    if (!cb) return 0;
    int32_t cls = cb->query_cls1_from_record
                      ? cb->query_cls1_from_record(ctx, record)
                      : (int32_t)(((record ^ (record & 0xff)) & 0x3c00) >> 10);
    if (cls != 0x0a) return 0;
    if (!cb->query_gdat_dbspec_word_value) return 0;
    int32_t v = cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 0);
    return (v & 0x4000) ? 1 : 0;
}

int32_t dm2_v1_querydb_food_value_from_record(int32_t record,
                                              const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    return cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 3);
}

int32_t dm2_v1_query_gdat_water_value_from_record(int32_t record,
                                                   const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:835 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    return cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 4);
}

int32_t dm2_v1_query_gdat_potion_spell_type_from_record(int32_t record,
                                                        const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:845 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    return cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 5);
}

int32_t dm2_v1_query_gdat_potion_behaviour_from_record(int32_t record,
                                                       const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:855 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    return cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 6);
}

int32_t dm2_v1_query_gdat_item_weight(int32_t record,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:865 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    return cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 1);
}

int32_t dm2_v1_query_gdat_item_strength(int32_t record,
                                        const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:875 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    return cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 2);
}

int32_t dm2_v1_query_gdat_creature_word_value(int32_t creature_type, int32_t field,
                                              const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:885 */
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 0x0f, (int8_t)creature_type, 11, (int8_t)field);
}

int32_t dm2_v1_querydb_item_name(int32_t record, char *buf, int32_t buf_len,
                                 const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    (void)record; (void)cb; (void)ctx;
    if (buf && buf_len > 0) buf[0] = '\0';
    return 0;
}

/* ── Section 7: Player queries ────────────────────────────────────── */

int32_t dm2_v1_querydb_player_skill_lv(int32_t player_idx, int32_t skill,
                                       const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    (void)player_idx; (void)skill; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_querydb_get_player_at_position(int32_t pos,
                                              const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    (void)pos; (void)cb; (void)ctx;
    return -1;
}

int32_t dm2_v1_find_hand_with_empty_flask(int32_t player_idx,
                                          const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1400 */
    (void)player_idx; (void)cb; (void)ctx;
    return -1;
}

int32_t dm2_v1_calc_player_walk_delay(int32_t player_idx,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1500 */
    (void)player_idx; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_querydb_compute_player_attack_or_throw_strength(int32_t player_idx,
                                                               int32_t hand,
                                                               const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    (void)player_idx; (void)hand; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_player_load(int32_t player_idx,
                                 const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1700 */
    (void)player_idx; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_player_max_load(int32_t player_idx,
                                     const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1720 */
    (void)player_idx; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_player_stamina(int32_t player_idx,
                                    const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1740 */
    (void)player_idx; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_player_max_stamina(int32_t player_idx,
                                        const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1760 */
    (void)player_idx; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_player_mana(int32_t player_idx,
                                 const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1780 */
    (void)player_idx; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_player_max_mana(int32_t player_idx,
                                     const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1800 */
    (void)player_idx; (void)cb; (void)ctx;
    return 0;
}

/* ── Section 8: Rainfall and environmental queries ────────────────── */

void dm2_v1_query_rainfall_param(int8_t *out_a, int16_t *out_d,
                                 const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: needs ddat state access — skproject c_querydb.cpp:3034 */
    (void)cb; (void)ctx;
    *out_a = 0x71;
    *out_d = 0;
}

int32_t dm2_v1_query_sky_color(int32_t map_idx,
                               const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:3070 */
    (void)map_idx; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_ambient_light(int32_t map_idx,
                                   const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:3090 */
    (void)map_idx; (void)cb; (void)ctx;
    return 0;
}

/* ── Section 9: Misc item classification ──────────────────────────── */

int32_t dm2_v1_query_item_is_chest(int32_t record,
                                   const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:900 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    int32_t v = cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 0);
    return (v & 0x0001) ? 1 : 0;
}

int32_t dm2_v1_query_item_is_mirror(int32_t record,
                                    const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:912 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    int32_t v = cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 0);
    return (v & 0x0080) ? 1 : 0;
}

int32_t dm2_v1_query_item_is_torch(int32_t record,
                                   const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:924 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    int32_t v = cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 0);
    return (v & 0x0010) ? 1 : 0;
}

int32_t dm2_v1_query_item_is_container(int32_t record,
                                       const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:936 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    int32_t v = cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 0);
    return (v & 0x0002) ? 1 : 0;
}

int32_t dm2_v1_query_item_attack_class(int32_t record,
                                       const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:948 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    return cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 7);
}

int32_t dm2_v1_query_item_armor_class(int32_t record,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:958 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    return cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 8);
}

int32_t dm2_v1_query_item_slot_mask(int32_t record,
                                    const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:968 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    return cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 9);
}

/* ── Section 10: Viewport and position helpers ────────────────────── */

int32_t dm2_v1_query_facing_delta_x(int32_t dir,
                                    const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1920 */
    (void)cb; (void)ctx;
    static const int32_t dx[4] = { 0, 1, 0, -1 };
    if (dir < 0 || dir > 3) return 0;
    return dx[dir];
}

int32_t dm2_v1_query_facing_delta_y(int32_t dir,
                                    const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1930 */
    (void)cb; (void)ctx;
    static const int32_t dy[4] = { -1, 0, 1, 0 };
    if (dir < 0 || dir > 3) return 0;
    return dy[dir];
}

int32_t dm2_v1_query_opposite_dir(int32_t dir,
                                  const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1940 */
    (void)cb; (void)ctx;
    return (dir + 2) & 3;
}

int32_t dm2_v1_query_turn_right(int32_t dir,
                                const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1948 */
    (void)cb; (void)ctx;
    return (dir + 1) & 3;
}

int32_t dm2_v1_query_turn_left(int32_t dir,
                               const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:1956 */
    (void)cb; (void)ctx;
    return (dir + 3) & 3;
}

/* ── Section 11: Record classification helpers ────────────────────── */

int32_t dm2_v1_query_cls1_from_record(int32_t record,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:2100 */
    (void)cb; (void)ctx;
    return (record >> 10) & 0x0f;
}

int32_t dm2_v1_query_cls2_from_record(int32_t record,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:2110 */
    (void)cb; (void)ctx;
    return record & 0x03ff;
}

int32_t dm2_v1_query_record_from_cls(int32_t cls1, int32_t cls2,
                                     const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:2120 */
    (void)cb; (void)ctx;
    return ((cls1 & 0x0f) << 10) | (cls2 & 0x03ff);
}

/* ── Section 12: Spell and magic queries ──────────────────────────── */

int32_t dm2_v1_query_spell_cost(int32_t spell_type,
                                const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:2500 */
    (void)spell_type; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_spell_difficulty(int32_t spell_type,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:2520 */
    (void)spell_type; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_spell_skill(int32_t spell_type,
                                 const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:2540 */
    (void)spell_type; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_spell_duration(int32_t spell_type,
                                    const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:2560 */
    (void)spell_type; (void)cb; (void)ctx;
    return 0;
}

/* ── Section 13: Map and dungeon queries ──────────────────────────── */

int32_t dm2_v1_query_map_width(int32_t map_idx,
                               const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:2700 */
    (void)map_idx; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_map_height(int32_t map_idx,
                                const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:2720 */
    (void)map_idx; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_map_level(int32_t map_idx,
                               const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:2740 */
    (void)map_idx; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_map_count(const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:2760 */
    (void)cb; (void)ctx;
    return 0;
}

/* ── Section 14: Text and gdat string queries ─────────────────────── */

int32_t dm2_v1_query_gdat_text(int32_t cls1, int32_t cls2, int32_t field,
                               char *buf, int32_t buf_len,
                               const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    char *text;

    if (!buf || buf_len <= 0) return 0;
    buf[0] = '\0';
    if (!cb || !cb->query_gdat_text) return 0;

    /* ReDMCSB/SKProject: skcore.cpp::QUERY_GDAT_TEXT at 2636:02F8.
     * The source routine takes byte GDAT keys, writes the decoded and
     * FORMAT_SKSTR-expanded result to its caller-owned buffer, and returns
     * that buffer. Do not silently truncate signed or wider caller values:
     * a wrapped key would select unrelated original game data. */
    if (cls1 < 0 || cls1 > 0xff ||
        cls2 < 0 || cls2 > 0xff ||
        field < 0 || field > 0xff) {
        return 0;
    }

    text = cb->query_gdat_text(ctx, (int8_t)cls1, (int8_t)cls2,
                               (int8_t)field, buf);
    return text != NULL ? 1 : 0;
}

/* ── Section 15: Projectile and missile queries ───────────────────── */

int32_t dm2_v1_query_missile_type(int32_t record,
                                  const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:3200 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    return cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 10);
}

int32_t dm2_v1_query_missile_damage(int32_t record,
                                    const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:3220 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    return cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 11);
}

/* ── Section 16: Actuator and sensor queries ──────────────────────── */

int32_t dm2_v1_query_actuator_type_from_record(int32_t record,
                                               const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:3400 */
    (void)record; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_sensor_effect(int32_t sensor_type,
                                   const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:3420 */
    (void)sensor_type; (void)cb; (void)ctx;
    return 0;
}

/* ── Section 17: Graphics and rendering queries ───────────────────── */

int32_t dm2_v1_query_gdat_image_width(int32_t cls1, int32_t cls2, int32_t field,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:4200 */
    (void)cls1; (void)cls2; (void)field; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_gdat_image_height(int32_t cls1, int32_t cls2, int32_t field,
                                       const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:4220 */
    (void)cls1; (void)cls2; (void)field; (void)cb; (void)ctx;
    return 0;
}

int32_t dm2_v1_query_ornate_anim_frame_count(int32_t ornament,
                                             const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:4300 */
    if (!cb || !cb->query_gdat_entry_data_index) return 0;
    return (int32_t)cb->query_gdat_entry_data_index(ctx, 9, (int8_t)ornament, 11, 20);
}

int32_t dm2_v1_query_creature_anim_speed(int32_t creature_type,
                                         const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:4500 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x1a];
}

/* ── Section 18: Sound and audio queries ──────────────────────────── */

int32_t dm2_v1_query_creature_sound_idx(int32_t creature_type,
                                        const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:4700 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x1b];
}

int32_t dm2_v1_query_item_sound_idx(int32_t record,
                                    const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:4720 */
    if (!cb || !cb->query_gdat_dbspec_word_value) return 0;
    return cb->query_gdat_dbspec_word_value(ctx, record & 0xffff, 12);
}

/* ── Section 19: Movement and pathfinding queries ─────────────────── */

int32_t dm2_v1_query_creature_can_fly(int32_t creature_type,
                                      const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:4800 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)((uint8_t)p[0x06] & 0x01);
}

int32_t dm2_v1_query_creature_can_open_doors(int32_t creature_type,
                                             const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:4820 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)((uint8_t)p[0x06] & 0x02) >> 1;
}

int32_t dm2_v1_query_creature_non_material(int32_t creature_type,
                                           const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:4840 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)((uint8_t)p[0x06] & 0x04) >> 2;
}

int32_t dm2_v1_query_creature_levitate(int32_t creature_type,
                                       const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:4860 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)((uint8_t)p[0x06] & 0x08) >> 3;
}

int32_t dm2_v1_query_creature_attack_range(int32_t creature_type,
                                           const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:4880 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x1c];
}

int32_t dm2_v1_query_creature_sight_range(int32_t creature_type,
                                          const DM2_V1_QueryDbCallbacks *cb, void *ctx)
{
    /* TODO: port from skproject c_querydb.cpp:4900 */
    if (!cb || !cb->query_creature_ai_spec_from_type) return 0;
    uint8_t *p = cb->query_creature_ai_spec_from_type(ctx, creature_type & 0xffff);
    if (!p) return 0;
    return (int32_t)(uint8_t)p[0x19];
}
