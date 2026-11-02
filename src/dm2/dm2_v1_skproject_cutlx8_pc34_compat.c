/* DM2 byte truncation helper — skproject SKWINDOS/types.cpp and
 * SKWINSPX/src/v5/types.cpp: CUTLX8. */

#include "dm2_v1_skproject_core.h"

int16_t dm2_v1_skproject_cutlx8(int32_t value)
{
    uint8_t low_byte = (uint8_t)((uint32_t)value & 0xffu);
    return low_byte < 0x80u
        ? (int16_t)low_byte
        : (int16_t)low_byte - 0x100;
}
