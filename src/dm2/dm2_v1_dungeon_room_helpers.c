#include "dm2_v1_dungeon_room_helpers.h"

#include <string.h>

static uint32_t dm2_room_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    return hash * 16777619u;
}

static void dm2_room_receipt_begin(DM2_V1_DungeonRoomReceipt *receipt,
                                   const char *symbol,
                                   const char *source_path)
{
    dm2_v1_dungeon_room_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

void dm2_v1_dungeon_room_receipt_clear(DM2_V1_DungeonRoomReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

int dm2_v1_SET_TILE_ATTRIBUTE_02(
    uint8_t *tile_attributes,
    size_t tile_count,
    size_t tile_index,
    DM2_V1_DungeonRoomReceipt *out_receipt)
{
    uint8_t before;
    uint8_t after;

    dm2_room_receipt_begin(out_receipt,
                           "SET_TILE_ATTRIBUTE_02",
                           "SKWIN/SkWinCore.cpp:3050");
    if (!tile_attributes || tile_index >= tile_count ||
        tile_count > DM2_V1_STONE_ROOM_MAX_TILES) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    before = tile_attributes[tile_index];
    after = (uint8_t)(before | DM2_V1_TILE_ATTRIBUTE_02);
    tile_attributes[tile_index] = after;
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->mutated = before != after;
        out_receipt->tile_index = (uint16_t)tile_index;
        out_receipt->attribute_before = before;
        out_receipt->attribute_after = after;
    }
    return 1;
}

int dm2_v1_SUMMARIZE_STONE_ROOM(
    const DM2_V1_StoneRoomTile *tiles,
    size_t tile_count,
    DM2_V1_StoneRoomSummary *out_summary,
    DM2_V1_DungeonRoomReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (out_summary) {
        memset(out_summary, 0, sizeof(*out_summary));
    }
    dm2_room_receipt_begin(out_receipt,
                           "SUMMARIZE_STONE_ROOM",
                           "SKWIN/SkWinCore.cpp:9680");
    if (!tiles || !out_summary || tile_count == 0u ||
        tile_count > DM2_V1_STONE_ROOM_MAX_TILES) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    out_summary->tile_count = (uint16_t)tile_count;
    for (i = 0u; i < tile_count; ++i) {
        const DM2_V1_StoneRoomTile *tile = &tiles[i];

        if (tile->tile_type == 0u) {
            ++out_summary->wall_count;
        } else {
            ++out_summary->floor_count;
        }
        if (tile->has_door) {
            ++out_summary->door_count;
        }
        if (tile->has_alcove) {
            ++out_summary->alcove_count;
        }
        if ((tile->attributes & DM2_V1_TILE_ATTRIBUTE_02) != 0u) {
            ++out_summary->attribute02_count;
        }
        if (tile->has_wall_ornament) {
            ++out_summary->ornament_count;
        }
        hash = dm2_room_hash_step(hash, tile->tile_type);
        hash = dm2_room_hash_step(hash, tile->attributes);
        hash = dm2_room_hash_step(hash, tile->has_wall_ornament);
        hash = dm2_room_hash_step(hash, tile->has_door);
        hash = dm2_room_hash_step(hash, tile->has_alcove);
    }
    out_summary->decoration_blocked = 1;
    out_summary->summary_hash = hash ? hash : 1u;
    out_summary->valid = 1;
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->blocked = out_summary->decoration_blocked;
        out_receipt->tile_index = out_summary->tile_count;
        out_receipt->attribute_after =
            (uint8_t)(out_summary->attribute02_count > 255u
                          ? 255u
                          : out_summary->attribute02_count);
    }
    return 1;
}

const char *dm2_v1_dungeon_room_helpers_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp SET_TILE_ATTRIBUTE_02:3050 "
           "SUMMARIZE_STONE_ROOM:9680; bounded dungeon-room receipts only, "
           "with random decoration/table population blocked until proven.";
}
