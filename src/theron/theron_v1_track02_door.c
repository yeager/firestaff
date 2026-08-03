#include "theron_v1_track02_door.h"

int theron_v1_track02_door_decode(const uint8_t *raw4, Theron_Door *out) {
    if (!raw4 || !out) return -1;

    /* 4 bytes total: 2 bytes next_ref + 2 bytes door data
     * Door data word (16-bit LE):
     *   bit 0:     type (0=wooden, 1=iron)
     *   bits 4:1:  ornate index
     *   bit 5:     opens up
     *   bit 6:     button (has button)
     *   bit 7:     destroyable
     *   bit 8:     bashable
     *   bits 15:9: unused */
    out->next_ref = (uint16_t)raw4[0] | ((uint16_t)raw4[1] << 8);

    uint16_t w = (uint16_t)raw4[2] | ((uint16_t)raw4[3] << 8);
    out->type        = (uint8_t)(w & 1);
    out->ornate      = (uint8_t)((w >> 1) & 0x0F);
    out->opens_up    = (uint8_t)((w >> 5) & 1);
    out->button      = (uint8_t)((w >> 6) & 1);
    out->destroyable = (uint8_t)((w >> 7) & 1);
    out->bashable    = (uint8_t)((w >> 8) & 1);

    return 0;
}

int theron_v1_track02_teleporter_decode(const uint8_t *raw6, Theron_Teleporter *out) {
    if (!raw6 || !out) return -1;

    /* 6 bytes total: 2 bytes next_ref + 4 bytes teleporter data
     * Word 1 (16-bit LE):
     *   bits 4:0:   x destination
     *   bits 9:5:   y destination
     *   bits 11:10: rotation
     *   bit 12:     absolute (vs relative)
     *   bits 14:13: scope (0=items, 1=party+items, 2=party, 3=all)
     *   bit 15:     sound
     * Word 2 (16-bit LE):
     *   bits 7:0:   unused
     *   bits 13:8:  level destination (DM1 uses bits 11:8 = 4 bits)
     *   bits 15:14: unused */
    out->next_ref = (uint16_t)raw6[0] | ((uint16_t)raw6[1] << 8);

    uint16_t w1 = (uint16_t)raw6[2] | ((uint16_t)raw6[3] << 8);
    uint16_t w2 = (uint16_t)raw6[4] | ((uint16_t)raw6[5] << 8);

    out->x_dest    = (uint8_t)(w1 & 0x1F);
    out->y_dest    = (uint8_t)((w1 >> 5) & 0x1F);
    out->rotation  = (uint8_t)((w1 >> 10) & 0x03);
    out->absolute  = (uint8_t)((w1 >> 12) & 1);
    out->scope     = (uint8_t)((w1 >> 13) & 0x03);
    out->sound     = (uint8_t)((w1 >> 15) & 1);
    out->level_dest = (uint8_t)((w2 >> 8) & 0x0F);

    return 0;
}
