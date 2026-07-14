#include "redmcsb_f0787_get_zone_initialized_from_coordinates_pc34_compat.h"

int16_t *redmcsb_f0787_get_zone_initialized_from_coordinates_pc34_compat(
    int16_t zone[4],
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height)
{
    zone[0] = x;
    zone[1] = y;
    zone[2] = width;
    zone[3] = height;
    return zone;
}

const char *redmcsb_f0787_get_zone_initialized_from_coordinates_source_evidence_pc34(void)
{
    return "ReDMCSB COORD.C:1840-1852; four-word XYZ zone initialization";
}
