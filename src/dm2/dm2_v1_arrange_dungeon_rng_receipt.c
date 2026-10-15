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

static uint16_t arrange_read_pool_u16(const DM2_V1_RecordPoolSet *pools,
                                      const uint8_t *bytes)
{
    if (pools->words_big_endian) {
        return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
    }
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static void arrange_write_pool_u16(const DM2_V1_RecordPoolSet *pools,
                                   uint8_t *bytes, uint16_t value)
{
    if (pools->words_big_endian) {
        bytes[0] = (uint8_t)(value >> 8);
        bytes[1] = (uint8_t)value;
    } else {
        bytes[0] = (uint8_t)value;
        bytes[1] = (uint8_t)(value >> 8);
    }
}

int dm2_v1_DM2_ARRANGE_DUNGEON_apply_rng_directions(
    const DM2_V1_DungeonData *dungeon,
    const DM2_V1_CaiiSourceOwner *caii_source,
    DM2_V1_RecordPoolSet *record_pools,
    DM2_V1_DropRng *session_rng,
    DM2_V1_ArrangeDungeonRngApplyReceipt *out)
{
    DM2_V1_ArrangeDungeonRngApplyReceipt receipt;
    DM2_V1_ArrangeDungeonRngReceipt census;
    DM2_V1_RecordPoolSet candidate;
    DM2_V1_DropRng candidate_rng;
    uint32_t record_limit = 0u;

    memset(&receipt, 0, sizeof(receipt));
    memset(&census, 0, sizeof(census));
    memset(&candidate, 0, sizeof(candidate));
    if (out) *out = receipt;
    if (!out || !session_rng || !record_pools || !record_pools->valid ||
        !record_pools->record_graph_complete ||
        !dungeon || !dungeon->raw_data ||
        record_pools->words_big_endian != dungeon->words_big_endian ||
        record_pools->source_words_big_endian != dungeon->source_words_big_endian ||
        !dm2_v1_DM2_ARRANGE_DUNGEON_rng_receipt(
            dungeon, caii_source, &census) ||
        !dm2_v1_record_pool_set_clone(&candidate, record_pools)) {
        receipt.incomplete = 1;
        if (out) *out = receipt;
        return 0;
    }

    for (int type = 0; type < DM2_ARRANGE_DUNGEON_THING_TYPE_COUNT; ++type) {
        if (dungeon->thing_type_counts[type] < 0 ||
            dungeon->g1_extension_record_counts[type] < 0 ||
            UINT32_MAX - record_limit <
                (uint32_t)dungeon->thing_type_counts[type] ||
            UINT32_MAX - record_limit <
                (uint32_t)dungeon->g1_extension_record_counts[type]) {
            goto fail;
        }
        record_limit += (uint32_t)dungeon->thing_type_counts[type];
        record_limit += (uint32_t)dungeon->g1_extension_record_counts[type];
    }
    candidate_rng = *session_rng;
    receipt.rng_before = candidate_rng.random;

    /* SKProject skmap.cpp:851-918 traverses maps, columns, rows, then each
     * tile's record chain. Preserve that order for the global c_random LCG. */
    for (int map = 0; map < dungeon->level_count; ++map) {
        for (int x = 0; x < dungeon->level_widths[map]; ++x) {
            for (int y = 0; y < dungeon->level_heights[map]; ++y) {
                int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                int thing;
                uint32_t traversed = 0u;
                if (raw < 0) goto fail;
                if (((uint16_t)raw & 0x10u) == 0u) continue;
                thing = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
                if (thing == (int)DM2_THING_NULL_MARKER) continue;
                while (thing != (int)DM2_THING_END_MARKER) {
                    const uint8_t *source_record;
                    uint8_t *record;
                    int pool = (thing >> 10) & 0x0f;
                    int next;
                    if (thing < 0 || ++traversed > record_limit ||
                        !(source_record = dm2_v1_dungeon_get_thing_record(
                              dungeon, (uint16_t)thing, NULL, NULL, NULL)) ||
                        !(record = dm2_v1_record_pool_address_mut(
                              &candidate, (int16_t)thing))) {
                        goto fail;
                    }
                    if (pool == 4) {
                        const DM2_AIDefinition *ai = NULL;
                        const uint8_t type = source_record[4];
                        const uint16_t source_flags =
                            dm2_v1_dungeon_read_record_u16(
                                dungeon, source_record + 0x0e);
                        const uint16_t candidate_flags =
                            arrange_read_pool_u16(&candidate, record + 0x0e);
                        const uint16_t source_possession =
                            dm2_v1_dungeon_read_record_u16(
                                dungeon, source_record + 2);
                        uint16_t candidate_possession =
                            arrange_read_pool_u16(&candidate, record + 2);
                        if (!dm2_v1_caii_source_owner_ai_spec_def(
                                caii_source, type, &ai) || !ai ||
                            record[4] != type ||
                            (candidate_flags & 0x0080u) !=
                                (source_flags & 0x0080u) ||
                            (candidate_possession & 0x3fffu) !=
                                (source_possession & 0x3fffu)) {
                            goto fail;
                        }
                        if ((ai->w0AIFlags & DM2_AIFLAG_STATIC) != 0u &&
                            (source_flags & 0x0080u) == 0u &&
                            source_possession != DM2_THING_END_MARKER) {
                            uint16_t word;
                            int item = (int)source_possession;
                            word = arrange_read_pool_u16(&candidate,
                                                        record + 2);
                            word = (uint16_t)((word & 0x3fffu) |
                                (dm2_v1_drops_randdir(&candidate_rng) << 14));
                            arrange_write_pool_u16(&candidate, record + 2,
                                                   word);
                            ++receipt.rng_draws;
                            ++receipt.creature_direction_writes;
                            while (item != (int)DM2_THING_END_MARKER) {
                                const uint8_t *source_item_record;
                                uint8_t *item_record;
                                uint16_t next_word;
                                uint16_t source_next_word;
                                uint16_t next_link;
                                int16_t linked;
                                if (item < 0 || ++traversed > record_limit ||
                                    !(source_item_record =
                                        dm2_v1_dungeon_get_thing_record(
                                            dungeon, (uint16_t)item,
                                            NULL, NULL, NULL)) ||
                                    !(item_record =
                                        dm2_v1_record_pool_address_mut(
                                            &candidate, (int16_t)item)) ||
                                    !dm2_v1_record_pool_next_link(
                                        &candidate, (int16_t)item, &linked)) {
                                    goto fail;
                                }
                                next_word = arrange_read_pool_u16(
                                    &candidate, item_record);
                                source_next_word =
                                    dm2_v1_dungeon_read_record_u16(
                                        dungeon, source_item_record);
                                next_link = (uint16_t)linked;
                                if ((source_next_word ==
                                         DM2_THING_END_MARKER) !=
                                        (next_link ==
                                         DM2_THING_END_MARKER) ||
                                    (source_next_word & 0x3fffu) !=
                                        (next_link & 0x3fffu)) {
                                    goto fail;
                                }
                                if (next_link != DM2_THING_END_MARKER) {
                                    const uint16_t new_word =
                                        (uint16_t)((next_word & 0x3fffu) |
                                            (dm2_v1_drops_randdir(
                                                &candidate_rng) << 14));
                                    arrange_write_pool_u16(&candidate,
                                                           item_record,
                                                           new_word);
                                    ++receipt.rng_draws;
                                    ++receipt.possession_direction_writes;
                                }
                                if ((next_link & 0x3fffu) ==
                                    ((uint16_t)item & 0x3fffu) &&
                                    next_link != DM2_THING_END_MARKER) {
                                    goto fail;
                                }
                                item = (int)next_link;
                            }
                        }
                    }
                    next = dm2_v1_dungeon_get_next_thing(
                        dungeon, (uint16_t)thing);
                    if (next < 0 || next == thing) goto fail;
                    thing = next;
                }
            }
        }
    }

    if (receipt.rng_draws != census.rng_draws ||
        receipt.creature_direction_writes !=
            census.eligible_static_creatures ||
        receipt.possession_direction_writes != census.possession_records) {
        goto fail;
    }
    receipt.rng_after = candidate_rng.random;
    receipt.source_order_hash = census.source_order_hash;
    receipt.valid = 1;
    receipt.committed = 1;
    dm2_v1_record_pool_set_free(record_pools);
    *record_pools = candidate;
    *session_rng = candidate_rng;
    *out = receipt;
    return 1;

fail:
    receipt.incomplete = 1;
    dm2_v1_record_pool_set_free(&candidate);
    *out = receipt;
    return 0;
}
