/*
 * ReDMCSB COORD.C F0798_COMMAND_IsPointInZone, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0798_COMMAND_IS_POINT_IN_ZONE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0798_COMMAND_IS_POINT_IN_ZONE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * PC 3.4 M704..M709 stores a zone as { left_x, top_y, width, height }.
 * Width and height are counts, so the right and bottom source edges are
 * left_x + width - 1 and top_y + height - 1 respectively.
 */
int redmcsb_f0798_command_is_point_in_zone_pc34_compat(
    const int16_t zone[4],
    int16_t x,
    int16_t y);

const char *redmcsb_f0798_command_is_point_in_zone_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
