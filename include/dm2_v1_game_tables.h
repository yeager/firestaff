#ifndef FIRESTAFF_DM2_V1_GAME_TABLES_H
#define FIRESTAFF_DM2_V1_GAME_TABLES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const int16_t dm2_v1_direction_dx[4];
extern const int16_t dm2_v1_direction_dy[4];

extern const int8_t dm2_v1_light_attenuation[21];

extern const int8_t dm2_v1_movement_speed[24];

extern const int8_t dm2_v1_creature_scatter[16][2];

extern const int8_t dm2_v1_creature_occupy[16];

extern const int8_t dm2_v1_skill_map[6];
extern const int8_t dm2_v1_skill_class[6];

extern const int8_t dm2_v1_action_hand_map[32];

typedef struct {
    int valid;
    int16_t dx;
    int16_t dy;
} DM2_V1_DirectionDeltaReceipt;

int dm2_v1_direction_delta(int direction, DM2_V1_DirectionDeltaReceipt *out);

const char *dm2_v1_game_tables_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
