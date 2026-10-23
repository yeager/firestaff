/* Bounded traversal of source-owned DB14 missile records in viewport order. */
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_skproject_core.h"
#include <string.h>

int dm2_v1_g1_snapshot_visible_missile_chains(
    const DM2_V1_DungeonData *dungeon,
    const DM2_V1_RecordPoolSet *record_pools,
    int map,
    const DM2_V1_G1VisibleMapCell *visible_cells,
    size_t visible_cell_count,
    uint8_t view_direction,
    DM2_V1_G1MissileChainSnapshot *out_snapshots,
    size_t snapshot_capacity,
    size_t *out_count)
{
    static const uint8_t missile_position_5x5[4] = { 6u, 8u, 18u, 16u };
    uint64_t record_budget = 0u;
    uint64_t traversed_records = 0u;
    size_t missile_count = 0u;

    if (out_count) *out_count = 0u;
    if (!dungeon || !record_pools || !out_count ||
        (snapshot_capacity != 0u && !out_snapshots) ||
        (visible_cell_count != 0u && !visible_cells) ||
        visible_cell_count > 20u ||
        snapshot_capacity > SIZE_MAX / sizeof(*out_snapshots) ||
        !dungeon->raw_data || dungeon->square_bytes != 1 ||
        !dungeon->record_graph_complete || !record_pools->valid ||
        !record_pools->record_graph_complete || map < 0 ||
        map >= dungeon->level_count || dungeon->level_widths[map] <= 0 ||
        dungeon->level_heights[map] <= 0 ||
        dungeon->level_widths[map] > DM2_V1_MAX_MAP_SIZE ||
        dungeon->level_heights[map] > DM2_V1_MAX_MAP_SIZE ||
        (record_pools->source_words_big_endian !=
         (dungeon->words_big_endian != 0))) {
        return 0;
    }

    for (int pool = 0; pool < DM2_V1_RECORD_POOL_COUNT; ++pool) {
        const DM2_V1_RecordPool *owner = &record_pools->pools[pool];
        if (owner->record_count < 0 || owner->extension_count < 0 ||
            (owner->record_count > 0 && owner->record_size < 2) ||
            (owner->extension_count > 0 && owner->record_size < 2)) {
            return 0;
        }
        record_budget += (uint64_t)owner->record_count +
                         (uint64_t)owner->extension_count;
        if (record_budget > UINT32_MAX) return 0;
    }

    for (size_t cell_index = 0; cell_index < visible_cell_count; ++cell_index) {
        const int x = visible_cells[cell_index].x;
        const int y = visible_cells[cell_index].y;
        {
            int tile = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
            int root;
            int16_t link;
            uint32_t ordinal = 0u;

            if (x < 0 || y < 0 || x >= dungeon->level_widths[map] ||
                y >= dungeon->level_heights[map] || tile < 0) goto malformed;
            for (size_t prior = 0; prior < cell_index; ++prior) {
                if (visible_cells[prior].x == x && visible_cells[prior].y == y)
                    goto malformed;
            }
            if (((unsigned)tile & 0x10u) == 0u) continue;
            root = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
            if (root < 0 || root > 0xffff) goto malformed;
            link = (int16_t)(uint16_t)root;
            if (link == DM2_V1_RECORD_HANDLE_NULL ||
                link == DM2_V1_RECORD_HANDLE_END) {
                continue;
            }

            while (link != DM2_V1_RECORD_HANDLE_END) {
                const uint8_t *record;
                int type;
                int16_t next;

                /* A valid finite graph cannot visit more records than the
                 * authenticated pools own. This also terminates all cycles. */
                if (traversed_records >= record_budget ||
                    ordinal >= UINT32_MAX) goto malformed;
                type = dm2_v1_record_handle_pool(link);
                if (type < 0 || type >= DM2_V1_RECORD_POOL_COUNT ||
                    (!record_pools->pools[type].bytes &&
                     !record_pools->pools[type].extension_bytes)) {
                    goto malformed;
                }
                record = dm2_v1_record_pool_address(record_pools, link);
                if (!record || record_pools->pools[type].record_size < 2) {
                    goto malformed;
                }
                ++ordinal;
                ++traversed_records;

                if (type == 14) {
                    DM2_V1_G1MissileChainSnapshot *snapshot;
                    DM2_V1_SkprojectQueryObject5x5PosReceipt position_receipt;
                    uint8_t position_5x5 = 0xffu;
                    uint32_t hash = 2166136261u;

                    if (record_pools->pools[14].record_size < 8 ||
                        !dm2_v1_skproject_query_object_5x5_pos(
                            (uint16_t)link, (uint8_t)(view_direction & 3u),
                            record_pools, missile_position_5x5,
                            &position_5x5, &position_receipt) ||
                        !position_receipt.valid ||
                        position_receipt.object_type != 14u ||
                        position_receipt.subtype !=
                            (((uint16_t)link >> 14) & 3u) ||
                        position_5x5 > 24u || missile_count >= snapshot_capacity ||
                        !out_snapshots) {
                        goto malformed;
                    }
                    snapshot = &out_snapshots[missile_count++];
                    memset(snapshot, 0, sizeof(*snapshot));
                    snapshot->object_id = (uint16_t)link;
                    snapshot->map = (int16_t)map;
                    snapshot->x = (int16_t)x;
                    snapshot->y = (int16_t)y;
                    snapshot->chain_ordinal = ordinal;
                    snapshot->missile_object =
                        dm2_v1_dungeon_read_record_u16(dungeon, record + 2);
                    snapshot->energy_remaining = record[4];
                    snapshot->energy_remaining2 = record[5];
                    snapshot->timer_index =
                        dm2_v1_dungeon_read_record_u16(dungeon, record + 6);
                    snapshot->view_position_5x5 = position_5x5;
                    snapshot->object_direction_subtype =
                        (uint8_t)(((uint16_t)link >> 14) & 3u);
                    for (int byte = 0; byte < 8; ++byte) {
                        hash ^= record[byte];
                        hash *= 16777619u;
                    }
                    snapshot->raw_record_hash = hash ? hash : 1u;
                }

                /* PC G1 has no persisted GenericRecord::w0 chains. Its
                 * authenticated root is a single record; reading record[0:2]
                 * as a next-link would reinterpret opaque DB14 bytes. */
                if (dungeon->g1_w0_chains_disabled) {
                    next = DM2_V1_RECORD_HANDLE_END;
                } else if (!dm2_v1_record_pool_next_link(
                               record_pools, link, &next)) {
                    goto malformed;
                }
                if (next == DM2_V1_RECORD_HANDLE_NULL) goto malformed;
                link = next;
            }
        }
    }

    *out_count = missile_count;
    return 1;

malformed:
    if (out_snapshots && missile_count != 0u)
        memset(out_snapshots, 0, missile_count * sizeof(*out_snapshots));
    *out_count = 0u;
    return 0;
}
