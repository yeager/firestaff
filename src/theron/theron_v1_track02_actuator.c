#include "theron_v1_track02_actuator.h"

int theron_v1_track02_actuator_decode(
    const uint8_t *raw8, Theron_Actuator *out)
{
    if (!raw8 || !out) return -1;

    /* Byte layout (8 bytes = 4 × 16-bit LE words):
     *   w0 (bytes 0-1): next reference link (id:10, category:4, position:2)
     *   w1 (bytes 2-3): type(7 bits) + value(9 bits)
     *   w2 (bytes 4-5): effect flags (once, effect, sound, delay, inactive, graphism)
     *   w3 (bytes 6-7): target (facing, x, y) */
    uint16_t w0 = (uint16_t)raw8[0] | ((uint16_t)raw8[1] << 8);
    uint16_t w1 = (uint16_t)raw8[2] | ((uint16_t)raw8[3] << 8);
    uint16_t w2 = (uint16_t)raw8[4] | ((uint16_t)raw8[5] << 8);
    uint16_t w3 = (uint16_t)raw8[6] | ((uint16_t)raw8[7] << 8);

    out->next_ref = w0;

    out->type   = (uint8_t)(w1 & 0x7F);
    out->value  = (uint16_t)((w1 >> 7) & 0x1FF);

    out->once     = (w2 >> 2) & 1;
    out->effect   = (w2 >> 3) & 7;
    out->sound    = (w2 >> 6) & 1;
    out->delay    = (w2 >> 7) & 0xF;
    out->inactive = (w2 >> 11) & 1;
    out->graphism = (w2 >> 12) & 0xF;

    out->target_facing = (w3 >> 4) & 3;
    out->target_x      = (w3 >> 6) & 0x1F;
    out->target_y      = (w3 >> 11) & 0x1F;

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
