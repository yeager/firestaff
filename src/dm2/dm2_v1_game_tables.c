#include "dm2_v1_game_tables.h"
#include <string.h>

const int16_t dm2_v1_direction_dx[4] = { 0, 1, 0, -1 };
const int16_t dm2_v1_direction_dy[4] = { -1, 0, 1, 0 };

const int8_t dm2_v1_light_attenuation[21] = {
    99, 75, 50, 25, 1, 0, 4, 5,
     6,  7,  8,  9, 10, 11, 12, 13,
    14, 15, 16, 17, 18
};

const int8_t dm2_v1_movement_speed[24] = {
    5, 5, 4, 3, 2, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 2, 3, 4, 5, 5
};

const int8_t dm2_v1_creature_scatter[16][2] = {
    {0x08, 0x04}, {0x04, 0x04}, {0x0c, 0x04}, {0x08, 0x08},
    {0x04, 0x08}, {0x0c, 0x08}, {0x08, 0x0c}, {0x04, 0x0c},
    {0x0c, 0x0c}, {0x00, 0x0c}, {0x10, 0x0c}, {0x08, 0x10},
    {0x04, 0x10}, {0x0c, 0x10}, {0x00, 0x10}, {0x10, 0x10}
};

const int8_t dm2_v1_creature_occupy[16] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1
};

const int8_t dm2_v1_skill_map[6] = { 5, 5, 4, 6, 3, 1 };
const int8_t dm2_v1_skill_class[6] = { 0, 3, 2, 1, 0, 0 };

const int8_t dm2_v1_action_hand_map[32] = {
    5,  4,  9,  8,  7, 12,  6, 11,
    3, 13, 14, 15, 16, 17, 18, 19,
   20, 21, 22, 23, 24, 25, 26, 27,
   28, 29, 10,  2,  0,  1,  0,  0
};

int dm2_v1_direction_delta(int direction, DM2_V1_DirectionDeltaReceipt *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    if (direction < 0 || direction > 3) return 0;

    out->valid = 1;
    out->dx = dm2_v1_direction_dx[direction];
    out->dy = dm2_v1_direction_dy[direction];
    return 1;
}

const char *dm2_v1_game_tables_source_evidence(void)
{
    return "skproject SKULLWIN/dm2data.cpp table1d27fc:172 table1d2804:177 "
           "table1d6712:66 table1d70f0:182 table1d6de3:851 table1d6dd3:859 "
           "table1d69aa:864 table1d69a2:865 table1d69b0:876; "
           "DM2 core game constant tables.";
}
