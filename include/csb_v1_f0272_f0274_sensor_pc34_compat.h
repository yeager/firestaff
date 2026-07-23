#ifndef FIRESTAFF_CSB_V1_F0272_F0274_SENSOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0272_F0274_SENSOR_PC34_COMPAT_H

#include "csb_v1_f0268_f0269_sensor_pc34_compat.h"

typedef struct {
    int valid;
    int sensor_thing;
    int source_x;
    int source_y;
    int target_x;
    int target_y;
    int target_cell;
    int effect;
    int once_only;
    int possession_found;
    uint8_t event_type;
    uint32_t event_time;
    CSB_V1_F0268F0269ReceiptPc34 event_receipt;
    const char *source_evidence;
} CSB_V1_F0272F0274ReceiptPc34;

/* ReDMCSB SENSOR.C F0272 remote-effect branch.  This is deliberately not a
 * local-effect adapter: F0270/F0271 retain that ownership. */
int csb_v1_f0272_trigger_remote_effect_pc34(
    CSB_V1_RuntimeProfile *profile, uint16_t sensor_thing, int effect,
    int source_x, int source_y, CSB_V1_F0272F0274ReceiptPc34 *out_receipt);

/* Receipt wrapper for the source-owned F0274 runtime possession scan. */
int csb_v1_f0274_party_possession_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile, int object_type,
    CSB_V1_F0272F0274ReceiptPc34 *out_receipt);

#endif
