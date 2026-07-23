#ifndef FIRESTAFF_CSB_V1_F0276_SENSOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0276_SENSOR_PC34_COMPAT_H

#include "csb_v1_runtime_pc34_compat.h"

typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    uint16_t moving_thing;
    int moving_thing_type;
    int moving_object_type;
    int party_square;
    int add_thing;
    int triggered_cell;
    int square_contains_object;
    int square_contains_group;
    int square_contains_same_type;
    int square_contains_different_type;
    uint16_t sensor_thing;
    int sensor_type;
    int sensor_data;
    int sensor_cell;
    int effect;
    int would_trigger;
    const char *source_evidence;
} CSB_V1_F0276ReceiptPc34;

/* Read-only ReDMCSB MOVESENS.C F0276 admission. It reports the first
 * source-qualified C03 candidate but never links Things, publishes events,
 * changes inventory, applies rotation, or advances the timeline. */
int csb_v1_f0276_sensor_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile, int map_x, int map_y,
    uint16_t moving_thing, int party_square, int add_thing,
    CSB_V1_F0276ReceiptPc34 *out_receipt);

#endif
