#ifndef FIRESTAFF_DM2_V1_MOVE_2FCF_0434_H
#define FIRESTAFF_DM2_V1_MOVE_2FCF_0434_H

#include <stdint.h>

#include "dm2_v1_dungeon_loader.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM2_V1_MOVE_2FCF_0434_BLOCK_NONE = 0,
    DM2_V1_MOVE_2FCF_0434_BLOCK_NO_DUNGEON,
    DM2_V1_MOVE_2FCF_0434_BLOCK_NO_SOURCE_TILE,
    DM2_V1_MOVE_2FCF_0434_BLOCK_NOT_DB1_TELEPORTER,
    DM2_V1_MOVE_2FCF_0434_BLOCK_SOURCE_TILE_NOT_ENABLED_TELEPORTER,
    DM2_V1_MOVE_2FCF_0434_BLOCK_INCOMPLETE_RECORD_GRAPH,
    DM2_V1_MOVE_2FCF_0434_BLOCK_PARTY_SCOPE,
    DM2_V1_MOVE_2FCF_0434_BLOCK_DESTINATION_BOUNDS
} DM2_V1_Move2fcf0434BlockReason;

typedef struct {
    int valid;
    int admitted;
    int blocked;
    DM2_V1_Move2fcf0434BlockReason block_reason;
    int source_level;
    int source_x;
    int source_y;
    int source_raw;
    int source_square_type;
    int source_tile_enabled;
    int record_db_type;
    int record_index;
    int record_graph_complete;
    int teleporter_scope;
    int party_scope_allowed;
    int destination_map;
    int destination_x;
    int destination_y;
    int destination_bounded;
    int generic_record_reads;
    int blocked_record_reads;
    uint32_t transition_hash;
} DM2_V1_Move2fcf0434Receipt;

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
    DM2_V1_Move2fcf0434Receipt *out);

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
    DM2_V1_Move2fcf0434Receipt *out);

const char *dm2_v1_DM2_move_2fcf_0434_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_MOVE_2FCF_0434_H */
