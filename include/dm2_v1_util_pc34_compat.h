#ifndef FIRESTAFF_DM2_V1_UTIL_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_UTIL_PC34_COMPAT_H

/*
 * dm2_v1_util_pc34_compat.h — DM2 utility functions.
 *
 * Ports pure computation helpers from skproject util.cpp.
 * These are used across combat, movement, timers, and AI.
 *
 * Source: skproject/SKWINSPX/src/v5/util.cpp
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int16_t dm2_v1_abs_i16(int16_t n)
{
    return (n >= 0) ? n : (int16_t)-n;
}

static inline int16_t dm2_v1_min_i16(int16_t a, int16_t b)
{
    return (a < b) ? a : b;
}

static inline int16_t dm2_v1_max_i16(int16_t a, int16_t b)
{
    return (a > b) ? a : b;
}

static inline int16_t dm2_v1_clamp_i16(int16_t lo, int16_t hi, int16_t val)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

/* Manhattan distance. Source: util.cpp DM2_CALC_SQUARE_DISTANCE */
int16_t dm2_v1_calc_square_distance(int16_t x1, int16_t y1,
                                     int16_t x2, int16_t y2);

/* Direction from (a,d) to (b,c). Returns 0-3.
 * If dx == dy, uses rand_bit to break tie.
 * Source: util.cpp DM2_CALC_VECTOR_DIR */
int16_t dm2_v1_calc_vector_dir(int16_t a, int16_t d,
                                int16_t b, int16_t c,
                                int rand_bit);

/* Offset a point by (front, side) relative to direction a.
 * Uses table1d27fc/table1d2804 directional deltas.
 * Source: util.cpp DM2_CALC_VECTOR_W_DIR */
void dm2_v1_calc_vector_w_dir(int16_t dir, int16_t front, int16_t side,
                               const int16_t dir_dx[4],
                               const int16_t dir_dy[4],
                               int16_t *out_x, int16_t *out_y);

/* Find the d-th set bit in a, returning its power-of-2 mask.
 * Source: util.cpp DM2_COMPUTE_POWER_4_WITHIN */
int32_t dm2_v1_compute_power_4_within(int16_t a, int16_t d);

/* (a * b) >> c. Source: util.cpp DM2_ATIMESB_RSHIFTC */
static inline int32_t dm2_v1_atimesb_rshiftc(int16_t a, int8_t c, int16_t b)
{
    return (int32_t)((uint32_t)(uint16_t)a * (uint32_t)(uint16_t)b) >> c;
}

/* Fill a table of int16_t with a value. Source: util.cpp DM2_FILL_I16TABLE */
void dm2_v1_fill_i16_table(int16_t *table, int16_t value, uint16_t entries);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_UTIL_PC34_COMPAT_H */
