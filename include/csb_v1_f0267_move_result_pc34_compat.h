#ifndef FIRESTAFF_CSB_V1_F0267_MOVE_RESULT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0267_MOVE_RESULT_PC34_COMPAT_H

#include "csb_v1_runtime_pc34_compat.h"

typedef enum {
    CSB_V1_F0267_VARIANT_NONE_PC34 = 0,
    CSB_V1_F0267_VARIANT_IIGS_GET_MOVE_RES_PC34 = 1,
    CSB_V1_F0267_VARIANT_CPSCE_GET_MOVE_RESULT_PC34 = 2
} CSB_V1_F0267VariantPc34;

typedef enum {
    CSB_V1_F0267_SOURCE_ON_SQUARE_PC34 = 0,
    CSB_V1_F0267_SOURCE_NOT_ON_SQUARE_PC34 = 1,
    CSB_V1_F0267_SOURCE_PROJECTILE_ASSOCIATED_PC34 = 2
} CSB_V1_F0267SourceModePc34;

typedef enum {
    CSB_V1_F0267_DESTINATION_ON_SQUARE_PC34 = 0,
    CSB_V1_F0267_DESTINATION_REMOVE_PC34 = 1
} CSB_V1_F0267DestinationModePc34;

/* This is an ownership receipt, not a movement or sensor executor. F0268+
 * remains outside this helper. */
typedef struct {
    int valid;
    CSB_V1_F0267VariantPc34 variant;
    CSB_V1_F0267SourceModePc34 source_mode;
    CSB_V1_F0267DestinationModePc34 destination_mode;
    uint16_t thing;
    int thing_type;
    int thing_record_offset;
    int thing_record_size;
    uint32_t thing_record_fnv1a;
    int source_map_index;
    int source_map_x;
    int source_map_y;
    int destination_map_index;
    int destination_map_x;
    int destination_map_y;
    const char *source_evidence;
} CSB_V1_F0267MoveResultReceiptPc34;

/* Source-lock the two audit variants. The IIGS-only name has no callable
 * body in the available ReDMCSB corpus and is deliberately rejected. CPSCE
 * admits only authenticated PC34 Thing records and coordinates. */
int csb_v1_f0267_move_result_receipt_pc34(
    const CSB_V1_DungeonData *dungeon,
    CSB_V1_F0267VariantPc34 variant,
    uint16_t thing,
    int source_map_index,
    int source_map_x,
    int source_map_y,
    int destination_map_index,
    int destination_map_x,
    int destination_map_y,
    CSB_V1_F0267MoveResultReceiptPc34 *out_receipt);

#endif
