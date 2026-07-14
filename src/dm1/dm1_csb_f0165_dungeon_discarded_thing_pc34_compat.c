#include "dm1_csb_f0165_dungeon_discarded_thing_pc34_compat.h"

static int dm1_csb_f0165_is_party_visible_square(uint16_t map_index,
                                                   uint16_t party_map_index,
                                                   uint16_t map_x,
                                                   uint16_t map_y,
                                                   int16_t party_map_x,
                                                   int16_t party_map_y)
{
    return map_index == party_map_index &&
           (uint16_t)(map_x - (uint16_t)party_map_x + 5u) <= 10u &&
           (uint16_t)(map_y - (uint16_t)party_map_y + 5u) <= 10u;
}

static int dm1_csb_f0165_discard_selected(
    const DM1_CSB_F0165_DungeonOps *ops,
    void *context,
    uint16_t thing,
    uint16_t thing_type,
    uint16_t map_index,
    uint16_t map_x,
    uint16_t map_y)
{
    enum DM1_CSB_F0165_DiscardAction action;

    switch (thing_type) {
    case DM1_CSB_F0165_THING_TYPE_GROUP:
        if (ops->thing_is_protected(context, thing)) return 0;
        action = DM1_CSB_F0165_DISCARD_GROUP;
        break;
    case DM1_CSB_F0165_THING_TYPE_PROJECTILE:
        action = DM1_CSB_F0165_DISCARD_PROJECTILE;
        break;
    case DM1_CSB_F0165_THING_TYPE_ARMOUR:
    case DM1_CSB_F0165_THING_TYPE_WEAPON:
    case DM1_CSB_F0165_THING_TYPE_JUNK:
    case DM1_CSB_F0165_THING_TYPE_POTION:
        if (ops->thing_is_protected(context, thing)) return 0;
        action = DM1_CSB_F0165_DISCARD_OBJECT;
        break;
    default:
        /* ReDMCSB's switch has no default: F0165 returns this matching
         * record unchanged. F0166 only calls it for reclaimable pools. */
        return 1;
    }
    return ops->discard_thing(context, action, thing, map_index, map_x, map_y);
}

uint16_t F0165_DUNGEON_GetDiscardedThing_Compat(
    DM1_CSB_F0165_DiscardState *state,
    const DM1_CSB_F0165_DungeonOps *ops,
    void *context,
    uint16_t thing_type,
    uint16_t map_count,
    uint16_t party_map_index,
    int16_t party_map_x,
    int16_t party_map_y)
{
    uint16_t map_index;
    uint16_t discard_start_map;

    if (!state || !ops || !ops->get_map_bounds || !ops->get_first_thing ||
        !ops->get_next_thing || !ops->get_thing_type ||
        !ops->sensor_is_enabled || !ops->thing_is_protected ||
        !ops->discard_thing || thing_type > 15 || map_count == 0 ||
        map_count > 255 || party_map_index >= map_count ||
        thing_type == DM1_CSB_F0165_THING_TYPE_EXPLOSION) {
        return DM1_CSB_F0165_THING_NONE;
    }

    map_index = state->last_discarded_map[thing_type];
    if (map_index >= map_count) map_index = 0;
    if (map_index == party_map_index && ++map_index >= map_count) map_index = 0;
    discard_start_map = map_index;

    for (;;) {
        uint16_t max_x;
        uint16_t max_y;
        uint16_t map_x;

        if (ops->get_map_bounds(context, map_index, &max_x, &max_y)) {
            for (map_x = 0;; ++map_x) {
                uint16_t map_y;
                for (map_y = 0;; ++map_y) {
                    uint16_t thing;
                    if (dm1_csb_f0165_is_party_visible_square(
                            map_index, party_map_index, map_x, map_y,
                            party_map_x, party_map_y)) {
                        continue;
                    }
                    thing = ops->get_first_thing(context, map_index, map_x, map_y);
                    while (thing != DM1_CSB_F0165_THING_ENDOFLIST) {
                        uint8_t current_type = ops->get_thing_type(context, thing);
                        if (current_type == DM1_CSB_F0165_THING_TYPE_SENSOR) {
                            if (ops->sensor_is_enabled(context, thing)) break;
                        } else if (current_type == thing_type &&
                                   dm1_csb_f0165_discard_selected(
                                       ops, context, thing, thing_type,
                                       map_index, map_x, map_y)) {
                            state->last_discarded_map[thing_type] = (uint8_t)map_index;
                            return (uint16_t)(thing & 0x3fffu);
                        }
                        thing = ops->get_next_thing(context, thing);
                    }
                    if (map_y == max_y) break;
                }
                if (map_x == max_x) break;
            }
        }

        if (map_index == party_map_index || map_count <= 1) {
            state->last_discarded_map[thing_type] = (uint8_t)map_index;
            return DM1_CSB_F0165_THING_NONE;
        }
        do {
            if (++map_index >= map_count) map_index = 0;
        } while (map_index == party_map_index);
        if (map_index == discard_start_map) map_index = party_map_index;
    }
}
