#ifndef FIRESTAFF_DM1_V1_GROUP_STATE_BUNDLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GROUP_STATE_BUNDLE_PC34_COMPAT_H

#include <stdint.h>

#include "memory_tick_orchestrator_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Source-shaped ACTIVE_GROUP row used at the C04/F0435 boundary. */
typedef struct DM1_V1_SourceActiveGroupPc34Compat {
    int groupThing;
    int cells;
    int directions;
    int lastMoveTime;
    int delayFleeingFromTarget;
    int targetMapX;
    int targetMapY;
    int priorMapX;
    int priorMapY;
    int homeMapX;
    int homeMapY;
    uint8_t aspect[4];
} DM1_V1_SourceActiveGroupPc34Compat;

typedef struct DM1_V1_GroupStateBundleReceiptPc34Compat {
    int valid;
    int activeGroupCount;
    int sourceCapacity;
    int groupIndex;
    int mapIndex;
    unsigned int cells;
    unsigned int directions;
    uint32_t fingerprint;
    const char *sourceSymbol;
} DM1_V1_GroupStateBundleReceiptPc34Compat;

/* GROUP.C F0182, F0226 and MOVESENS.C F0273 share the live C04/SFT world.
 * Keep that ownership at one boundary: callers receive a raw-source receipt
 * rather than being allowed to construct a group or square-chain surrogate. */
typedef struct DM1_V1_GroupSensorReceiptPc34Compat {
    int valid;
    int mapIndex;
    int mapX;
    int mapY;
    int activeGroupIndex;
    int groupIndex;
    int squareDistance;
    uint16_t requestedThingType;
    int requestedCell;
    uint16_t matchedThing;
    int matchedThingType;
    int matchedThingIndex;
    int matchedCell;
    int removedReactionCount;
    const char *sourceSymbol;
} DM1_V1_GroupSensorReceiptPc34Compat;

/* GROUP.C F0196 for PC 3.4.  The exact sixty source owners are staged before
 * publishing, preserving any host-only storage beyond that range. */
int dm1_v1_group_state_initialize_f0196_pc34(
    struct GameWorld_Compat *world,
    DM1_V1_GroupStateBundleReceiptPc34Compat *outReceipt);

/* DUNGEON.C F0146/F0148 as one group-state transaction.  On the party map
 * only the matching ACTIVE_GROUP row changes; elsewhere the raw C04 row is
 * updated.  A bad owner or raw row leaves both fields unchanged. */
int dm1_v1_group_state_write_f0146_f0148_pc34(
    struct GameWorld_Compat *world,
    int mapIndex,
    int groupIndex,
    unsigned int cells,
    unsigned int directions,
    DM1_V1_GroupStateBundleReceiptPc34Compat *outReceipt);

/* LOADSAVE.C F0435 C04 handoff.  All rows are admitted into a candidate
 * runtime first, so malformed cells/directions/group owners cannot partially
 * replace the live active-group state. */
int dm1_v1_group_state_apply_save_handoff_pc34(
    struct GameWorld_Compat *world,
    const DM1_V1_SourceActiveGroupPc34Compat *rows,
    int currentCount,
    int sourceCapacity,
    DM1_V1_GroupStateBundleReceiptPc34Compat *outReceipt);

const char *dm1_v1_group_state_bundle_source_evidence_pc34(void);

/* GROUP.C F0182: validates the ACTIVE_GROUP -> raw C04 owner first, then
 * commits the four Aspect updates and exact-square C29..C41 deletion as one
 * transaction.  Invalid C04, map, or timeline state leaves both unchanged. */
int dm1_v1_group_stop_attacking_f0182_pc34(
    struct GameWorld_Compat *world,
    int activeGroupIndex,
    int mapIndex,
    int mapX,
    int mapY,
    DM1_V1_GroupSensorReceiptPc34Compat *outReceipt);

/* GROUP.C F0226: source-owned Manhattan distance.  Coordinates must name
 * squares in the currently loaded original map; no host coordinates are
 * admitted. */
int dm1_v1_group_square_distance_f0226_pc34(
    const struct GameWorld_Compat *world,
    int mapIndex,
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY,
    DM1_V1_GroupSensorReceiptPc34Compat *outReceipt);

/* MOVESENS.C F0273: walks the selected raw SquareFirstThing chain in source
 * order and returns the first matching F0032 object type/cell.  A valid
 * no-match has matchedThing == 0xffff; malformed ownership fails closed. */
int dm1_v1_sensor_get_object_of_type_f0273_pc34(
    const struct GameWorld_Compat *world,
    int mapIndex,
    int mapX,
    int mapY,
    int cell,
    uint16_t objectType,
    DM1_V1_GroupSensorReceiptPc34Compat *outReceipt);

#ifdef __cplusplus
}
#endif

#endif
