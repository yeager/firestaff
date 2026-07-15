#ifndef FIRESTAFF_CSB_V1_F0145_F0148_EFFECTIVE_GROUP_OWNER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0145_F0148_EFFECTIVE_GROUP_OWNER_PC34_COMPAT_H

#include "csb_v1_runtime_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

/* ReDMCSB DUNGEON.C F0145--F0148.  A C04 GROUP record is 16 bytes.  Outside
 * the party map, byte 5 is Cells and byte 15 bits 0..1 are Direction.  On the
 * party map byte 5 is the ActiveGroupIndex selector and effective Cells and
 * Directions belong to the selected ACTIVE_GROUP. */
enum {
    CSB_V1_F0145_F0148_C04_RECORD_SIZE_PC34 = 16,
    CSB_V1_F0145_F0148_C04_ACTIVE_GROUP_INDEX_OFFSET_PC34 = 5,
    CSB_V1_F0145_F0148_C04_DIRECTION_OFFSET_PC34 = 15,
    CSB_V1_F0145_F0148_THING_TYPE_GROUP_PC34 = 4
};

typedef struct {
    uint8_t *group_record;
    size_t record_size;
    uint16_t group_thing;
    uint16_t map_index;
    uint16_t party_map_index;
    CSB_V1_RuntimeActiveGroupState *active_groups;
    size_t active_group_count;
} CsbV1F0145F0148EffectiveGroupOwnerPc34Compat;

typedef struct {
    uint8_t cells;
    uint16_t directions;
} CsbV1F0145F0148EffectiveGroupValuesPc34Compat;

typedef struct {
    int write_cells;
    uint8_t cells;
    int write_directions;
    uint16_t directions;
} CsbV1F0145F0148EffectiveGroupMutationPc34Compat;

/* Reads the effective F0145/F0147 values.  The owner fails closed when a C04
 * record, party-map selector, or active state is invalid. */
int csb_v1_f0145_f0148_effective_group_read_pc34_compat(
    const CsbV1F0145F0148EffectiveGroupOwnerPc34Compat *owner,
    CsbV1F0145F0148EffectiveGroupValuesPc34Compat *out_values);

/* Performs F0146 and/or F0148.  Both requested writes are validated before
 * either raw C04 or ACTIVE_GROUP state is committed. */
int csb_v1_f0145_f0148_effective_group_write_pc34_compat(
    CsbV1F0145F0148EffectiveGroupOwnerPc34Compat *owner,
    const CsbV1F0145F0148EffectiveGroupMutationPc34Compat *mutation);

#endif
