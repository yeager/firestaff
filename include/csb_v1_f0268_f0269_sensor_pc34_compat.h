#ifndef FIRESTAFF_CSB_V1_F0268_F0269_SENSOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0268_F0269_SENSOR_PC34_COMPAT_H

#include "csb_v1_runtime_pc34_compat.h"

typedef struct {
    int valid;
    int event_index;
    int map_index;
    int map_x;
    int map_y;
    int cell;
    int effect;
    uint8_t event_type;
    uint32_t event_time;
    int recipient_count;
    int experience_per_recipient;
    int leader_only;
    const char *source_evidence;
} CSB_V1_F0268F0269ReceiptPc34;

int csb_v1_f0268_add_event_pc34(
    CSB_V1_RuntimeProfile *profile, uint8_t event_type, int map_x, int map_y,
    int cell, int effect, uint32_t event_time,
    CSB_V1_F0268F0269ReceiptPc34 *out_receipt);

int csb_v1_f0269_skill_experience_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile, int skill_index, int experience,
    int leader_only, CSB_V1_F0268F0269ReceiptPc34 *out_receipt);

#endif
