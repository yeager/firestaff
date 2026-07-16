#ifndef FIRESTAFF_DM2_V1_DUNGEON_ROOM_HELPERS_H
#define FIRESTAFF_DM2_V1_DUNGEON_ROOM_HELPERS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_TILE_ATTRIBUTE_02 0x02u
#define DM2_V1_STONE_ROOM_MAX_TILES 64u

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    int mutated;
    uint16_t tile_index;
    uint8_t attribute_before;
    uint8_t attribute_after;
    const char *symbol;
    const char *source_path;
} DM2_V1_DungeonRoomReceipt;

typedef struct {
    uint8_t tile_type;
    uint8_t attributes;
    uint8_t has_wall_ornament;
    uint8_t has_door;
    uint8_t has_alcove;
} DM2_V1_StoneRoomTile;

typedef struct {
    int valid;
    int decoration_blocked;
    uint16_t tile_count;
    uint16_t wall_count;
    uint16_t floor_count;
    uint16_t door_count;
    uint16_t alcove_count;
    uint16_t attribute02_count;
    uint16_t ornament_count;
    uint32_t summary_hash;
} DM2_V1_StoneRoomSummary;

void dm2_v1_dungeon_room_receipt_clear(DM2_V1_DungeonRoomReceipt *receipt);

int dm2_v1_SET_TILE_ATTRIBUTE_02(
    uint8_t *tile_attributes,
    size_t tile_count,
    size_t tile_index,
    DM2_V1_DungeonRoomReceipt *out_receipt);

int dm2_v1_SUMMARIZE_STONE_ROOM(
    const DM2_V1_StoneRoomTile *tiles,
    size_t tile_count,
    DM2_V1_StoneRoomSummary *out_summary,
    DM2_V1_DungeonRoomReceipt *out_receipt);

const char *dm2_v1_dungeon_room_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_DUNGEON_ROOM_HELPERS_H */
