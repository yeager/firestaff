#include "theron_v1_track02_dungeon_loader.h"
#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_thing_data.h"
#include "theron_v1_track02_ground_ref.h"
#include "theron_v1_track02_actuator.h"
#include "theron_v1_track02_door.h"
#include "theron_v1_track02_item_id_map.h"
#include "theron_v1_world.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned int map;
    unsigned int x;
    unsigned int y;
} TilePosition;

static int build_gref_position_table(
    const Theron_DungeonData *dd,
    TilePosition *pos_table,
    unsigned int max_entries)
{
    unsigned int k = 0;
    for (unsigned int m = 0; m < dd->map_count; m++) {
        unsigned int w = dd->maps[m].header.x_dim + 1u;
        unsigned int h = dd->maps[m].header.y_dim + 1u;
        for (unsigned int x = 0; x < w; x++) {
            for (unsigned int y = 0; y < h; y++) {
                if (theron_tile_has_things(dd->maps[m].tiles[x][y])) {
                    if (k >= max_entries) return -1;
                    pos_table[k].map = m;
                    pos_table[k].x = x;
                    pos_table[k].y = y;
                    k++;
                }
            }
        }
    }
    return (int)k;
}

static uint16_t get_item_next_ref(const Theron_ThingData *td,
                                   unsigned int cat, unsigned int id) {
    if (cat >= 16 || id >= td->object_counts[cat]) return THERON_REF_NONE;
    size_t item_size = theron_item_bytes[cat];
    if (item_size < 2) return THERON_REF_NONE;
    const uint8_t *rec = &td->items[cat][id * item_size];
    return (uint16_t)rec[0] | ((uint16_t)rec[1] << 8);
}

static uint8_t tile_type_at(const Theron_DungeonData *dd,
                             unsigned int map, unsigned int x, unsigned int y) {
    if (map >= dd->map_count) return 0;
    return theron_tile_type(dd->maps[map].tiles[x][y]);
}

static int retain_source_object_occurrence(
    Theron_DungeonLoadResult *result,
    uint16_t source_ref,
    unsigned int category,
    unsigned int source_index,
    unsigned int position,
    const uint8_t *raw,
    size_t raw_size,
    unsigned int map,
    unsigned int x,
    unsigned int y,
    const Theron_Track02ItemRecord *decoded)
{
    Theron_Track02SourceObjectOccurrence *occurrence;

    if (!result || !raw || category >= THERON_ITEM_CATEGORY_COUNT ||
        source_index > UINT16_MAX || position > UINT8_MAX ||
        raw_size == 0u || raw_size > sizeof(occurrence->raw) ||
        result->source_object_count >= THERON_TRACK02_SOURCE_OBJECT_MAX)
        return 0;
    occurrence = &result->source_objects[result->source_object_count++];
    memset(occurrence, 0, sizeof(*occurrence));
    occurrence->source_ref = source_ref;
    occurrence->next_ref = (uint16_t)raw[0] | ((uint16_t)raw[1] << 8);
    occurrence->category = (uint8_t)category;
    occurrence->source_index = (uint16_t)source_index;
    occurrence->position = (uint8_t)position;
    occurrence->raw_size = (uint8_t)raw_size;
    memcpy(occurrence->raw, raw, raw_size);
    occurrence->map = (uint16_t)map;
    occurrence->x = (uint16_t)x;
    occurrence->y = (uint16_t)y;
    if (decoded && decoded->category == category) {
        unsigned int type_value = 0;
        int has_type = 1;
        occurrence->decoded_valid = 1;
        occurrence->decoded = *decoded;
        switch (category) {
        case THERON_CAT_MONSTER:
            type_value = decoded->value.monster.type;
            break;
        case THERON_CAT_WEAPON:
            type_value = decoded->value.weapon.type;
            break;
        case THERON_CAT_CLOTHING:
            type_value = decoded->value.clothing.type;
            break;
        case THERON_CAT_SCROLL:
            type_value = decoded->value.scroll.type;
            break;
        case THERON_CAT_POTION:
            type_value = decoded->value.potion.type;
            break;
        case THERON_CAT_MISC:
            type_value = decoded->value.misc.type;
            break;
        default:
            has_type = 0;
            break;
        }
        if (has_type)
            result->source_type_value_mask[category][type_value >> 5] |=
                1u << (type_value & 31u);
    }
    result->source_category_counts[category]++;
    return 1;
}

int theron_v1_track02_load_full_dungeon(
    Theron_V1_World *world,
    int dungeon_id,
    const uint8_t *ud_data,
    size_t ud_size,
    Theron_DungeonLoadResult *result)
{
    return theron_v1_track02_load_full_dungeon_for_variant(
        world, dungeon_id, ud_data, ud_size,
        THERON_TRACK02_VARIANT_US_BIN, result);
}

int theron_v1_track02_load_full_dungeon_for_variant(
    Theron_V1_World *world,
    int dungeon_id,
    const uint8_t *ud_data,
    size_t ud_size,
    Theron_Track02Variant variant,
    Theron_DungeonLoadResult *result)
{
    if (!world || !ud_data || !result) return -1;
    if (dungeon_id < 1 || dungeon_id > 7) return -1;

    unsigned int di = (unsigned int)(dungeon_id - 1);
    memset(result, 0, sizeof(*result));

    Theron_DungeonData dd;
    if (!theron_v1_track02_dungeon_map_load_for_variant(
            ud_data, ud_size, variant, di, &dd))
        return -1;

    result->levels_loaded = theron_v1_world_load_track02_dungeon(world, dungeon_id, &dd);
    if (result->levels_loaded <= 0) return -1;

    unsigned int total_tiles = 0;
    uint8_t flat_tiles[8192];
    unsigned int fp2 = 0;
    for (unsigned int m = 0; m < dd.map_count; m++) {
        unsigned int w = dd.maps[m].header.x_dim + 1u;
        unsigned int h = dd.maps[m].header.y_dim + 1u;
        total_tiles += w * h;
        for (unsigned int x = 0; x < w; x++)
            for (unsigned int y = 0; y < h; y++)
                flat_tiles[fp2++] = dd.maps[m].tiles[x][y];
    }

    unsigned int gref_count =
        theron_v1_track02_compute_ground_ref_count(flat_tiles, total_tiles);

    Theron_ThingData *td = calloc(1, sizeof(Theron_ThingData));
    if (!td) return -1;

    if (!theron_v1_track02_thing_data_load_for_variant(
            ud_data, ud_size, variant, di, dd.object_counts,
            gref_count, td)) {
        free(td);
        return -1;
    }

    TilePosition *pos_table = NULL;
    if (gref_count > 0u) {
        pos_table = calloc(gref_count, sizeof(TilePosition));
        if (!pos_table) { free(td); return -1; }
    }

    int pos_count = build_gref_position_table(&dd, pos_table, gref_count);
    if (pos_count < 0 || (unsigned int)pos_count != gref_count) {
        free(pos_table);
        free(td);
        return -1;
    }

    for (unsigned int g = 0; g < gref_count; g++) {
        uint16_t ref = td->ground_refs[g];
        unsigned int map = pos_table[g].map;
        unsigned int tx = pos_table[g].x;
        unsigned int ty = pos_table[g].y;
        unsigned int depth = 0;

        while (!theron_ref_is_end(ref) && depth < 100) {
            unsigned int cat = theron_ref_category(ref);
            unsigned int id = theron_ref_id(ref);
            unsigned int pos = theron_ref_position(ref);

            if (cat >= 16 || id >= td->object_counts[cat]) break;

            Theron_V1_Object obj = {0};
            obj.x = (int16_t)tx;
            obj.y = (int16_t)ty;
            obj.level = (int)map;
            obj.dungeon_id = dungeon_id;
            obj.flags = (uint32_t)pos;

            int place = 1;

            switch (cat) {
            case THERON_CAT_DOOR: {
                Theron_Door door;
                theron_v1_track02_door_decode(
                    &td->items[cat][id * theron_item_bytes[cat]], &door);
                obj.type = 0x01;
                obj.state = door.type;
                obj.quantity = door.ornate;
                obj.flags = (uint32_t)pos |
                            ((uint32_t)door.opens_up << 8) |
                            ((uint32_t)door.button << 9) |
                            ((uint32_t)door.destroyable << 10) |
                            ((uint32_t)door.bashable << 11);
                world->levels[di][map].squares[ty][tx] = THERON_SQUARE_DOOR;
                result->doors_placed++;
                break;
            }
            case THERON_CAT_TELEPORTER: {
                Theron_Teleporter tp;
                theron_v1_track02_teleporter_decode(
                    &td->items[cat][id * theron_item_bytes[cat]], &tp);
                obj.type = 0x02;
                obj.state = tp.scope;
                obj.quantity = tp.rotation;
                obj.flags = (uint32_t)pos |
                            ((uint32_t)tp.absolute << 8) |
                            ((uint32_t)tp.sound << 9) |
                            THERON_OBJ_F_TRACK02_COORD_LINK;
                obj.linked_id = (int)((tp.level_dest << 10) |
                                       (tp.y_dest << 5) |
                                       tp.x_dest);
                world->levels[di][map].squares[ty][tx] = THERON_SQUARE_TELEPORTER;
                result->teleporters_placed++;
                break;
            }
            case THERON_CAT_TEXT:
                obj.type = 0x03;
                place = 0;
                break;
            case THERON_CAT_ACTUATOR: {
                Theron_Actuator act;
                theron_v1_track02_actuator_decode(
                    &td->items[cat][id * theron_item_bytes[cat]], &act);

                int is_wall = (tile_type_at(&dd, map, tx, ty) == THERON_TILE_WALL);
                if (theron_v1_track02_actuator_needs_value_fix(act.type, is_wall)) {
                    uint8_t translated = theron_v1_track02_translate_item_id(
                        (uint8_t)(act.value & 0xFF));
                    if (translated != (act.value & 0xFF)) {
                        act.value = translated;
                        result->actuator_value_fixes++;
                    }
                }

                if (!is_wall && act.type == TQ_ACT_FLOOR_MONSTER_GEN &&
                    theron_v1_world_bind_track02_generator(
                        world, dungeon_id, (int)map, ref, id, (int)tx, (int)ty,
                        act.type, act.value, act.once, act.effect, act.sound,
                        act.delay, act.inactive, act.graphism,
                        act.target_x, act.target_y, act.target_facing) != 0) {
                    free(pos_table);
                    free(td);
                    return -1;
                }

                obj.type = 0x04;
                obj.state = act.type;
                obj.quantity = act.value;
                obj.flags = ((uint32_t)act.graphism << 16) |
                            ((uint32_t)act.effect << 8) |
                            (uint32_t)pos;
                if (act.once) obj.flags |= 0x100000;
                if (act.sound) obj.flags |= 0x200000;
                if (act.inactive) obj.flags |= 0x400000;
                obj.linked_id = (int)((act.target_y << 11) |
                                       (act.target_x << 6) |
                                       (act.target_facing << 4));
                result->actuators_placed++;
                break;
            }
            case THERON_CAT_MONSTER:
            case THERON_CAT_WEAPON:
            case THERON_CAT_CLOTHING:
            case THERON_CAT_SCROLL:
            case THERON_CAT_POTION:
            case THERON_CAT_CHEST:
            case THERON_CAT_MISC: {
                Theron_Track02ItemRecord record;
                const uint8_t *raw =
                    &td->items[cat][id * theron_item_bytes[cat]];
                if (!theron_v1_track02_item_record_decode(
                        cat, raw, theron_item_bytes[cat], &record)) {
                    free(pos_table);
                    free(td);
                    return -1;
                }
                if (!retain_source_object_occurrence(
                        result, ref, cat, id, pos, raw, theron_item_bytes[cat],
                        map, tx, ty, &record)) {
                    free(pos_table);
                    free(td);
                    return -1;
                }
                if (cat == THERON_CAT_MONSTER &&
                    theron_v1_world_bind_track02_monster(
                        world, dungeon_id, (int)map, ref, id, (int)tx, (int)ty,
                        record.value.monster.type,
                        record.value.monster.position,
                        record.value.monster.health,
                        record.value.monster.number,
                        record.value.monster.direction_flags) != 0) {
                    free(pos_table);
                    free(td);
                    return -1;
                }
                /* Source record and chain are real and consumed. The host
                 * object kind/item-index owner is not yet proven, so keep
                 * the record out of Theron_V1_Object and continue the chain. */
                result->source_records_decoded++;
                result->unbound_item_refs++;
                place = 0;
                break;
            }
            case THERON_CAT_MISSILE:
            case THERON_CAT_CLOUD:
            {
                Theron_Track02ItemRecord record;
                const uint8_t *raw =
                    &td->items[cat][id * theron_item_bytes[cat]];
                if (!theron_v1_track02_item_record_decode(
                        cat, raw, theron_item_bytes[cat], &record)) {
                    free(pos_table);
                    free(td);
                    return -1;
                }
                if (!retain_source_object_occurrence(
                        result, ref, cat, id, pos, raw, theron_item_bytes[cat],
                        map, tx, ty, &record)) {
                    free(pos_table);
                    free(td);
                    return -1;
                }
                /* Missile/cloud source layouts are now decoded as records,
                 * but remain unbound from the host projectile/cloud owners. */
                result->source_records_decoded++;
                result->unbound_item_refs++;
                place = 0;
                break;
            }
            default:
                free(pos_table);
                free(td);
                return -1;
            }

            if (place) {
                if (theron_v1_object_place(world, &obj) != 0) {
                    free(pos_table);
                    free(td);
                    return -1;
                }
                world->levels[di][map].thing_count++;
                result->total_things_placed++;
            }

            result->ground_refs_linked++;
            ref = get_item_next_ref(td, cat, id);
            depth++;
        }
    }

    free(pos_table);
    free(td);
    return 0;
}
