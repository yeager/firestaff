#include "redmcsb_f0798_command_is_point_in_zone_pc34_compat.h"

int redmcsb_f0798_command_is_point_in_zone_pc34_compat(
    const int16_t zone[4],
    int16_t x,
    int16_t y)
{
    return x >= zone[0] && x <= zone[0] + zone[2] - 1 &&
           y >= zone[1] && y <= zone[1] + zone[3] - 1;
}

const char *redmcsb_f0798_command_is_point_in_zone_source_evidence_pc34(void)
{
    return "ReDMCSB COORD.C:1915-1920 (PC 3.4 I34 route): "
           "F0798_COMMAND_IsPointInZone tests x/y against M704_ZONE_LEFT, "
           "M705_ZONE_RIGHT, M706_ZONE_TOP and M707_ZONE_BOTTOM; "
           "DEFS.H:171-176 defines the PC zone layout as "
           "{left, top, width, height} with inclusive right/bottom edges.";
}
