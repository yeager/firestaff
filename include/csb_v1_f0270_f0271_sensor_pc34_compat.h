#ifndef FIRESTAFF_CSB_V1_F0270_F0271_SENSOR_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0270_F0271_SENSOR_PC34_COMPAT_H
#include "csb_v1_runtime_pc34_compat.h"
typedef struct { int valid; int map_index,map_x,map_y,cell,effect; int first_sensor,last_sensor,matching_sensor_count; int add_steal_xp; const char *source_evidence; } CSB_V1_F0270F0271ReceiptPc34;
int csb_v1_f0270_f0271_sensor_receipt_pc34(const CSB_V1_RuntimeProfile *profile, int effect, int map_x, int map_y, int cell, CSB_V1_F0270F0271ReceiptPc34 *out);
#endif
