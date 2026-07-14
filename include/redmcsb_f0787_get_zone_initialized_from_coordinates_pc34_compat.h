/*
 * ReDMCSB COORD.C F0787_GetZoneInitializedFromCoordinates, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0787_GET_ZONE_INITIALIZED_FROM_COORDINATES_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0787_GET_ZONE_INITIALIZED_FROM_COORDINATES_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The caller owns a four-word XYZ zone: left, top, width, height. */
int16_t *redmcsb_f0787_get_zone_initialized_from_coordinates_pc34_compat(
    int16_t zone[4],
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height);

const char *redmcsb_f0787_get_zone_initialized_from_coordinates_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
