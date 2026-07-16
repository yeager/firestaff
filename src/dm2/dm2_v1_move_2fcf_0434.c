#include "dm2_v1_move_2fcf_0434.h"

#include <string.h>

enum {
    DM2_V1_MOVE_2FCF_DB_TELEPORTER = 1,
    DM2_V1_MOVE_2FCF_SOURCE_TELEPORTER_TYPE = 5,
    DM2_V1_MOVE_2FCF_SOURCE_ENABLE_BIT = 0x08,
    DM2_V1_MOVE_2FCF_PARTY_SCOPE_BIT = 0x02
};

static uint32_t dm2_move_2fcf_0434_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

static int dm2_move_2fcf_0434_destination_bounded(
    int destination_map_count,
    int destination_width,
    int destination_height,
    int destination_map,
    int destination_x,
    int destination_y)
{
    if (destination_map_count <= 0 || destination_width <= 0 ||
        destination_height <= 0) {
        return 0;
    }
    if (destination_map < 0 || destination_map >= destination_map_count) {
        return 0;
    }
    if (destination_x < 0 || destination_x >= destination_width) {
        return 0;
    }
    if (destination_y < 0 || destination_y >= destination_height) {
        return 0;
    }
    return 1;
}

static void dm2_move_2fcf_0434_block(
    DM2_V1_Move2fcf0434Receipt *out,
    DM2_V1_Move2fcf0434BlockReason reason)
{
    out->blocked = 1;
    out->block_reason = reason;
}

int dm2_v1_DM2_move_2fcf_0434_teleporter_gate(
    const DM2_V1_DungeonData *dungeon,
    int source_level,
    int source_x,
    int source_y,
    int record_db_type,
    int record_index,
    int record_graph_complete,
    int teleporter_scope,
    int destination_map,
    int destination_x,
    int destination_y,
    DM2_V1_Move2fcf0434Receipt *out)
{
    int source_raw;
    int source_square_type;
    int destination_width = -1;
    int destination_height = -1;

    if (!dungeon || !dungeon->raw_data) {
        return dm2_v1_DM2_move_2fcf_0434_teleporter_gate_from_square(
            source_level, source_x, source_y, 0, -1, -1, 0, -1, -1,
            record_db_type, record_index, record_graph_complete,
            teleporter_scope, destination_map, destination_x, destination_y,
            out);
    }
    source_raw = dm2_v1_dungeon_get_tile_raw(
        dungeon, source_level, source_x, source_y);
    source_square_type = dm2_v1_dungeon_get_square_type(
        dungeon, source_level, source_x, source_y);
    if (destination_map >= 0 && destination_map < dungeon->level_count) {
        destination_width = dungeon->level_widths[destination_map];
        destination_height = dungeon->level_heights[destination_map];
    }
    return dm2_v1_DM2_move_2fcf_0434_teleporter_gate_from_square(
        source_level, source_x, source_y,
        (source_raw >= 0 && source_square_type >= 0), source_raw,
        source_square_type, dungeon->level_count, destination_width,
        destination_height, record_db_type, record_index,
        record_graph_complete, teleporter_scope, destination_map,
        destination_x, destination_y, out);
}

int dm2_v1_DM2_move_2fcf_0434_teleporter_gate_from_square(
    int source_level,
    int source_x,
    int source_y,
    int source_raw_valid,
    int source_raw,
    int source_square_type,
    int destination_map_count,
    int destination_width,
    int destination_height,
    int record_db_type,
    int record_index,
    int record_graph_complete,
    int teleporter_scope,
    int destination_map,
    int destination_x,
    int destination_y,
    DM2_V1_Move2fcf0434Receipt *out)
{
    uint32_t hash = 2166136261u;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->source_level = source_level;
    out->source_x = source_x;
    out->source_y = source_y;
    out->record_db_type = record_db_type;
    out->record_index = record_index;
    out->record_graph_complete = record_graph_complete ? 1 : 0;
    out->teleporter_scope = teleporter_scope;
    out->destination_map = destination_map;
    out->destination_x = destination_x;
    out->destination_y = destination_y;

    out->valid = 1;
    out->party_scope_allowed =
        (teleporter_scope & DM2_V1_MOVE_2FCF_PARTY_SCOPE_BIT) ? 1 : 0;
    out->destination_bounded = dm2_move_2fcf_0434_destination_bounded(
        destination_map_count, destination_width, destination_height,
        destination_map, destination_x, destination_y);

    if (!source_raw_valid) {
        dm2_move_2fcf_0434_block(
            out, DM2_V1_MOVE_2FCF_0434_BLOCK_NO_DUNGEON);
    } else {
        out->source_raw = source_raw;
        out->source_square_type = source_square_type;
        if (source_raw < 0 || source_square_type < 0) {
            dm2_move_2fcf_0434_block(
                out, DM2_V1_MOVE_2FCF_0434_BLOCK_NO_SOURCE_TILE);
        } else {
            out->source_tile_enabled =
                ((out->source_raw & DM2_V1_MOVE_2FCF_SOURCE_ENABLE_BIT) != 0 &&
                 out->source_square_type ==
                     DM2_V1_MOVE_2FCF_SOURCE_TELEPORTER_TYPE)
                    ? 1
                    : 0;
            if (record_db_type != DM2_V1_MOVE_2FCF_DB_TELEPORTER) {
                dm2_move_2fcf_0434_block(
                    out, DM2_V1_MOVE_2FCF_0434_BLOCK_NOT_DB1_TELEPORTER);
            } else if (!out->source_tile_enabled) {
                dm2_move_2fcf_0434_block(
                    out,
                    DM2_V1_MOVE_2FCF_0434_BLOCK_SOURCE_TILE_NOT_ENABLED_TELEPORTER);
            } else if (!out->record_graph_complete) {
                dm2_move_2fcf_0434_block(
                    out,
                    DM2_V1_MOVE_2FCF_0434_BLOCK_INCOMPLETE_RECORD_GRAPH);
            } else if (!out->party_scope_allowed) {
                dm2_move_2fcf_0434_block(
                    out, DM2_V1_MOVE_2FCF_0434_BLOCK_PARTY_SCOPE);
            } else if (!out->destination_bounded) {
                dm2_move_2fcf_0434_block(
                    out, DM2_V1_MOVE_2FCF_0434_BLOCK_DESTINATION_BOUNDS);
            } else {
                out->admitted = 1;
                out->block_reason = DM2_V1_MOVE_2FCF_0434_BLOCK_NONE;
            }
        }
    }

    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->source_level);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->source_x);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->source_y);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->source_raw);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->source_square_type);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->source_tile_enabled);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->record_db_type);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->record_index);
    hash = dm2_move_2fcf_0434_hash_step(
        hash, (uint32_t)out->record_graph_complete);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->teleporter_scope);
    hash = dm2_move_2fcf_0434_hash_step(
        hash, (uint32_t)out->party_scope_allowed);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->destination_map);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->destination_x);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->destination_y);
    hash = dm2_move_2fcf_0434_hash_step(
        hash, (uint32_t)out->destination_bounded);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->block_reason);
    hash = dm2_move_2fcf_0434_hash_step(hash, (uint32_t)out->admitted);
    if (hash == 0u) return 0;
    out->transition_hash = hash;
    return 1;
}

const char *dm2_v1_DM2_move_2fcf_0434_source_evidence(void)
{
    return "skproject SKULLWIN/c_move.cpp:2152 DM2_move_2fcf_0434, "
           "matching the SkWinCore.cpp::_2fcf_0434 teleporter transition "
           "prerequisite: dispatch DB1 Teleporter only from an enabled source "
           "ttTeleporter byte-square, require party scope, bounded "
           "destination, and a complete record graph, and perform no "
           "GenericRecord::w0 traversal or map mutation in this receipt.";
}
