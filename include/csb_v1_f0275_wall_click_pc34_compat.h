#ifndef FIRESTAFF_CSB_V1_F0275_WALL_CLICK_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0275_WALL_CLICK_PC34_COMPAT_H

#include "csb_v1_runtime_pc34_compat.h"

typedef struct {
    int valid, map_index, map_x, map_y, cell;
    uint16_t sensor_thing;
    int sensor_type, sensor_data, effect, click_accepted, requires_mutation;
    const char *source_evidence;
} CSB_V1_F0275WallClickReceiptPc34;

/* Read-only ReDMCSB SENSOR.C F0275 admission.  No event, inventory, sensor,
 * generator, audio, or timeline mutation is performed by this helper. */
int csb_v1_f0275_wall_click_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile, int map_x, int map_y, int cell,
    CSB_V1_F0275WallClickReceiptPc34 *out_receipt);

#endif
