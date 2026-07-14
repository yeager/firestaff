#include "csb_v1_startup_raster_present_pc34_compat.h"

#include <string.h>

static void deliver_vertical_blank(void *context)
{
    F0693_VerticalBlankCallback_PC34(
        (ReDMCSBF0693WaitVerticalBlankPc34Compat *)context);
}

int main(void)
{
    uint8_t indexed[320 * 200];
    uint8_t packed[320 * 200 / 2];
    csb_v1_startup_real_raster_pc34_compat raster = {
        indexed, 320, 200, 1, 1
    };
    ReDMCSBF0693WaitVerticalBlankPc34Compat gate = {
        false, deliver_vertical_blank, NULL
    };
    size_t index;

    memset(indexed, 0, sizeof(indexed));
    memset(packed, 0xff, sizeof(packed));
    indexed[0] = 9;
    indexed[1] = 8;
    indexed[63999] = 6;
    gate.context = &gate;
    if (!csb_v1_startup_present_real_raster_pc34_compat(
            &raster, packed, sizeof(packed), &gate) || packed[0] != 0x98 ||
        packed[sizeof(packed) - 1U] != 0x06 || gate.waiting_for_vertical_blank) {
        return 1;
    }
    indexed[100] = 16;
    memset(packed, 0xaa, sizeof(packed));
    if (csb_v1_startup_present_real_raster_pc34_compat(
            &raster, packed, sizeof(packed), &gate)) {
        return 1;
    }
    for (index = 0U; index < sizeof(packed); ++index) {
        if (packed[index] != 0xaa) {
            return 1;
        }
    }
    return 0;
}
