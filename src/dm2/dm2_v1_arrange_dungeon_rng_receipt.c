/* Source-order census of DM2_ARRANGE_DUNGEON conditional RANDDIR calls. */
#include "dm2_v1_caii_source_owner.h"

#include <string.h>

#define DM2_ARRANGE_DUNGEON_THING_TYPE_COUNT 16

static uint32_t dm2_arrange_rng_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

int dm2_v1_DM2_ARRANGE_DUNGEON_rng_receipt(
    const DM2_V1_DungeonData *dungeon,
    const DM2_V1_CaiiSourceOwner *caii_source,
    DM2_V1_ArrangeDungeonRngReceipt *out)
{
    DM2_V1_ArrangeDungeonRngReceipt receipt;
    uint32_t hash = 2166136261u;
    uint32_t record_limit = 0u;
    int map;

    memset(&receipt, 0, sizeof(receipt));
    if (out) *out = receipt;
    if (!out || !dungeon || !dungeon->raw_data ||
        dungeon->level_count <= 0 ||
        dungeon->level_count > DM2_V1_MAX_LEVELS ||
        !dungeon->record_graph_complete ||
        dungeon->g1_w0_chains_disabled ||
        !dm2_v1_dungeon_validate_record_graph(dungeon) ||
        !caii_source || !caii_source->valid) {
        return 0;
    }

    for (int type = 0; type < DM2_ARRANGE_DUNGEON_THING_TYPE_COUNT; ++type) {
        if (dungeon->thing_type_counts[type] < 0 ||
            dungeon->thing_type_counts[type] > 1024 ||
            UINT32_MAX - record_limit <
                (uint32_t)dungeon->thing_type_counts[type]) {
            return 0;
        }
        record_limit += (uint32_t)dungeon->thing_type_counts[type];
    }
    for (int type = 0; type < DM2_ARRANGE_DUNGEON_THING_TYPE_COUNT; ++type) {
        if (dungeon->g1_extension_record_counts[type] < 0 ||
            dungeon->g1_extension_record_counts[type] > 1024 ||
            UINT32_MAX - record_limit <
                (uint32_t)dungeon->g1_extension_record_counts[type]) {
            return 0;
        }
        record_limit +=
            (uint32_t)dungeon->g1_extension_record_counts[type];
    }
    if (record_limit == 0u) return 0;

    for (map = 0; map < dungeon->level_count; ++map) {
        int x;
        int width = dungeon->level_widths[map];
        int height = dungeon->level_heights[map];
        if (width <= 0 || width > DM2_V1_MAX_MAP_SIZE ||
            height <= 0 || height > DM2_V1_MAX_MAP_SIZE) {
            receipt.incomplete = 1;
            break;
        }
        for (x = 0; x < width; ++x) {
            int y;
            for (y = 0; y < height; ++y) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                int creature;
                uint32_t traversed = 0u;
                if (raw < 0) {
                    receipt.incomplete = 1;
                    break;
                }
                if (((uint16_t)raw & 0x10u) == 0u) continue;
                creature = dm2_v1_dungeon_get_first_thing(
                    dungeon, map, x, y);
                if (creature == (int)DM2_THING_NULL_MARKER) continue;
                while (creature != (int)DM2_THING_END_MARKER) {
                    const uint8_t *record;
                    int pool = (creature >> 10) & 0x0f;
                    int next;
                    ++traversed;
                    ++receipt.creature_records_seen;
                    if (creature < 0 ||
                        traversed > record_limit ||
                        !(record = dm2_v1_dungeon_get_thing_record(
                              dungeon, (uint16_t)creature,
                              NULL, NULL, NULL))) {
                        receipt.incomplete = 1;
                        break;
                    }
                    if (pool == 4) {
                        const DM2_AIDefinition *ai = NULL;
                        uint16_t flags;
                        uint16_t record_flags;
                        uint8_t type = record[4];
                        if (!dm2_v1_caii_source_owner_ai_spec_def(
                                caii_source, type, &ai) || !ai) {
                            receipt.incomplete = 1;
                            break;
                        }
                        flags = ai->w0AIFlags;
                        record_flags = dm2_v1_dungeon_read_record_u16(
                            dungeon, record + 0x0e);
                        hash = dm2_arrange_rng_hash_step(hash, (uint32_t)map);
                        hash = dm2_arrange_rng_hash_step(hash, (uint32_t)x);
                        hash = dm2_arrange_rng_hash_step(hash, (uint32_t)y);
                        hash = dm2_arrange_rng_hash_step(hash,
                                                     (uint16_t)creature);
                        hash = dm2_arrange_rng_hash_step(hash, type);
                        hash = dm2_arrange_rng_hash_step(hash, flags);
                        hash = dm2_arrange_rng_hash_step(hash, record_flags);
                        if ((flags & DM2_AIFLAG_STATIC) != 0u &&
                            (record_flags & 0x0080u) == 0u) {
                            uint16_t possession =
                                dm2_v1_dungeon_read_record_u16(
                                    dungeon, record + 2);
                            if (possession != DM2_THING_END_MARKER) {
                                int item = (int)possession;
                                ++receipt.eligible_static_creatures;
                                ++receipt.rng_draws;
                                hash = dm2_arrange_rng_hash_step(
                                    hash, (uint32_t)possession);
                                while (item != (int)DM2_THING_END_MARKER) {
                                    const uint8_t *item_record;
                                    int item_next;
                                    if (item < 0 ||
                                        ++traversed > record_limit ||
                                        !(item_record =
                                            dm2_v1_dungeon_get_thing_record(
                                                dungeon, (uint16_t)item,
                                                NULL, NULL, NULL))) {
                                        receipt.incomplete = 1;
                                        break;
                                    }
                                    item_next = (int)
                                        dm2_v1_dungeon_read_record_u16(
                                            dungeon, item_record);
                                    if (item_next !=
                                        (int)DM2_THING_END_MARKER) {
                                        ++receipt.possession_records;
                                        ++receipt.rng_draws;
                                    }
                                    hash = dm2_arrange_rng_hash_step(
                                        hash, (uint16_t)item);
                                    hash = dm2_arrange_rng_hash_step(
                                        hash, (uint16_t)item_next);
                                    if (item_next == item) {
                                        receipt.incomplete = 1;
                                        break;
                                    }
                                    item = item_next;
                                }
                                if (receipt.incomplete) break;
                            }
                        } else if ((flags & DM2_AIFLAG_STATIC) != 0u) {
                            ++receipt.skipped_direction_flag;
                        }
                    }
                    next = dm2_v1_dungeon_get_next_thing(
                        dungeon, (uint16_t)creature);
                    if (next < 0 || next == creature) {
                        receipt.incomplete = 1;
                        break;
                    }
                    creature = next;
                }
                if (receipt.incomplete) break;
            }
            if (receipt.incomplete) break;
        }
        if (receipt.incomplete) break;
    }
    receipt.source_order_hash = hash;
    receipt.valid = !receipt.incomplete && hash != 0u;
    *out = receipt;
    return receipt.valid;
}
