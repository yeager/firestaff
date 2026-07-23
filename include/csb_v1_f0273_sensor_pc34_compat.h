#ifndef FIRESTAFF_CSB_V1_F0273_SENSOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0273_SENSOR_PC34_COMPAT_H

#include "csb_v1_runtime_pc34_compat.h"

typedef struct {
    int valid;
    int map_index;
    int map_x;
    int map_y;
    int requested_cell;
    int object_type;
    uint16_t matched_thing;
    int matched_cell;
    const char *source_evidence;
} CSB_V1_F0273ReceiptPc34;

/* ReDMCSB SENSOR.C F0273.  Searches a loaded raw-PC34 square chain in source
 * order and returns a receipt even when no matching object exists. */
int csb_v1_f0273_get_object_of_type_in_cell_pc34(
    const CSB_V1_RuntimeProfile *profile, int map_x, int map_y, int cell,
    int object_type, CSB_V1_F0273ReceiptPc34 *out_receipt);

#endif
