#include "theron_v1_track02_actuator.h"

int theron_v1_track02_actuator_decode(
    const uint8_t *raw8, Theron_Actuator *out)
{
    if (!raw8 || !out) return -1;

    uint16_t w0 = (uint16_t)raw8[0] | ((uint16_t)raw8[1] << 8);
    uint16_t w1 = (uint16_t)raw8[2] | ((uint16_t)raw8[3] << 8);
    uint16_t w2 = (uint16_t)raw8[4] | ((uint16_t)raw8[5] << 8);

    out->type   = (uint8_t)(w0 & 0x7F);
    out->value  = (uint16_t)((w0 >> 7) & 0x1FF);

    out->once     = (w1 >> 2) & 1;
    out->effect   = (w1 >> 3) & 7;
    out->sound    = (w1 >> 6) & 1;
    out->delay    = (w1 >> 7) & 0xF;
    out->inactive = (w1 >> 11) & 1;
    out->graphism = (w1 >> 12) & 0xF;

    out->target_facing = (w2 >> 4) & 3;
    out->target_x      = (w2 >> 6) & 0x1F;
    out->target_y      = (w2 >> 11) & 0x1F;

    return 0;
}

int theron_v1_track02_actuator_needs_value_fix(
    uint8_t type, int is_wall)
{
    if (is_wall) {
        return type == TQ_ACT_WALL_ALCOVE_ITEM ||
               type == TQ_ACT_WALL_ITEM_EATER ||
               type == TQ_ACT_WALL_ITEM ||
               type == TQ_ACT_WALL_ITEM_EATER_TOGGLE;
    }
    return type == TQ_ACT_FLOOR_CARRIED_ITEM;
}
