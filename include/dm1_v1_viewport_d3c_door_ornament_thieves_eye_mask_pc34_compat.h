#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D3C_DOOR_ORNAMENT_THIEVES_EYE_MASK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D3C_DOOR_ORNAMENT_THIEVES_EYE_MASK_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int zone_index;
    int door_state;
    int ornament_index_expected;
    bool thieves_eye_active_expected;
    const char *redmcsbAnchor;
} DM1_V1_D3CDoorOrnamentThievesEyeMaskPc34Contract;

int dm1_v1_viewport_d3c_door_ornament_thieves_eye_mask_pc34_compat_test_entry(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D3C_DOOR_ORNAMENT_THIEVES_EYE_MASK_PC34_COMPAT_H */
