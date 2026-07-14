#include <stdint.h>
#include <string.h>

#include "redmcsb_f0787_get_zone_initialized_from_coordinates_pc34_compat.h"

int main(void)
{
    int16_t zone[4] = {(int16_t)99, (int16_t)98, (int16_t)97, (int16_t)96};
    int16_t *const result =
        redmcsb_f0787_get_zone_initialized_from_coordinates_pc34_compat(
            zone, (int16_t)-12, (int16_t)34, (int16_t)56, (int16_t)-78);

    if (result != zone || zone[0] != -12 || zone[1] != 34 || zone[2] != 56 ||
        zone[3] != -78) {
        return 1;
    }
    if (strcmp(
            redmcsb_f0787_get_zone_initialized_from_coordinates_source_evidence_pc34(),
            "ReDMCSB COORD.C:1840-1852; four-word XYZ zone initialization") != 0) {
        return 1;
    }

    return 0;
}
