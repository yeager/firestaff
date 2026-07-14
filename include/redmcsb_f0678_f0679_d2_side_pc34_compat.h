#ifndef FIRESTAFF_REDMCSB_F0678_F0679_D2_SIDE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0678_F0679_D2_SIDE_PC34_COMPAT_H

#include <stdint.h>

enum { REDMCSB_F0678_D2L2 = 0, REDMCSB_F0679_D2R2 = 1,
       REDMCSB_F0678_ELEMENT_WALL = 0, REDMCSB_F0678_ELEMENT_TELEPORTER = 5 };
typedef struct redmcsb_f0678_f0679_runtime_pc34_compat {
    void (*draw_wall)(void *context, int side, int source_wall_index, int zone, int flipped);
    void (*draw_teleporter_field)(void *context, int side, int field_aspect_index, int zone);
    void *context;
} redmcsb_f0678_f0679_runtime_pc34_compat;
int redmcsb_f0678_f0679_draw_d2_side_pc34_compat(int side, int16_t element, int use_flipped_wall, const redmcsb_f0678_f0679_runtime_pc34_compat *runtime);
const char *redmcsb_f0678_f0679_d2_side_source_evidence_pc34(void);
#endif
