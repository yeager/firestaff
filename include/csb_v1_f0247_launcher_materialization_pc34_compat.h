#ifndef CSB_V1_F0247_LAUNCHER_MATERIALIZATION_PC34_COMPAT_H
#define CSB_V1_F0247_LAUNCHER_MATERIALIZATION_PC34_COMPAT_H

#include <stdint.h>

typedef struct {
    int valid;
    uint16_t thing;
    int map_x;
    int map_y;
    int cell;
} CSB_V1_F0247LauncherMaterializationReceipt_PC34;

/* ReDMCSB PROJEXPL.C F0212, CSB21 CHANGE8_00_FIX: when no C14 projectile
 * record is available, only a non-explosion associated Thing is sent through
 * F0267 at the launch square. */
int csb_v1_f0247_launcher_create_failure_materialization_pc34_compat(
    uint16_t associated_thing,
    int map_x,
    int map_y,
    int cell,
    CSB_V1_F0247LauncherMaterializationReceipt_PC34 *out_receipt);

#endif
