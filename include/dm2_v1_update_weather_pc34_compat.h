/*
 * include/dm2_v1_update_weather_pc34_compat.h
 *
 * PC v1.1 decompile-source-faithful binding for the 0x54 timer dispatch
 * into DM2_UPDATE_WEATHER (src/dm2/c_tim_proc.cpp:4179-4183:
 *
 *     if (RG4UW <= 0x54) {
 *       DM2_UPDATE_WEATHER(1);
 *       continue;
 *     }
 *
 * ) plus the arg == 1 branch of DM2_UPDATE_WEATHER itself
 * (src/dm2/c_weather.cpp:33-90):
 *
 *     v1e147f = table1d6b76[4*v1e1472 + 0x70];   // zone weather flag
 *     retry = ++v1e147b;
 *     if (retry > 0x1f) {
 *       DM2_weather_3df7_0037(0);                // force transition, no requeue
 *       return;
 *     }
 *     v1e146e = v1e1474;                          // previous snapshot
 *     v1e1474 += v1e1484 * (i8)v1d7108[(v1e1478 << 5) + retry];
 *     v1e1474 = DM2_BETWEEN_VALUE(0, 0xff, v1e1474);
 *     DM2_SET_TIMER_WEATHER(RAND16(256) + 50);    // requeue delay 50..305
 *
 * Table bindings:
 *   - table1d6b76[132] is a static const in src/dm2/dm2data.cpp:889-896,
 *     bound verbatim below; the weather flags live at offsets 0x70..0x7f
 *     (4 * zone_index + 0x70): zone0=0x00, zone1=0x01, zone2=0x00,
 *     zone3=0x00 in the source data.
 *   - v1d7108[128] is binary data the source loads at runtime via
 *     DM2_READ_BINARY("v1d7108.dat") (src/dm2/dm2data.cpp:1371,
 *     declared src/dm2/dm2data.h:514). The
 *     128 bytes are bound verbatim from the extracted v1d7108.dat
 *     (reference extraction), indexed as signed i8:
 *     row = (pattern_row << 5) + retry, pattern_row in 0..3, retry 1..31.
 *
 * RNG: RAND16(256) uses the PC RNG (src/dm2/c_random.cpp:13-31,
 * state = state*0xbb40e62d + 11, (state >> 8) % n) — the same generator
 * already bound as DM2_V1_DropRng / dm2_v1_drops_rand16 in
 * dm2_v1_drops.h; this module reuses that binding.
 *
 * Boundaries / fail-closed:
 *   - zone_index outside 0..31 would index table1d6b76 out of bounds;
 *     bounded to 0..31, anything else -> no mutation, receipt invalid.
 *   - pattern_row outside 0..3 would index v1d7108 out of bounds;
 *     -> no mutation, receipt invalid.
 *   - retry > 0x1f forces a transition via DM2_weather_3df7_0037(0);
 *     that transition function (weather selection / map-weather state
 *     machine, c_weather.cpp:67+) is NOT bound here — the slice ends
 *     with a `transition_forced` receipt flag and the host owns the
 *     actual transition. No requeue happens in this branch.
 *   - The arg == 0 branch of DM2_UPDATE_WEATHER (day-rollover weather
 *     update) is a different path and is not bound here.
 *   - Producers v1e1472 (zone), v1e1478 (pattern row), v1e1484 (step)
 *     are map-load-owned; the caller supplies them in the state struct.
 */
#ifndef DM2_V1_UPDATE_WEATHER_PC34_COMPAT_H
#define DM2_V1_UPDATE_WEATHER_PC34_COMPAT_H

#include <stdint.h>

#include "dm2_v1_drops.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum bounded value of v1e147b's post-increment retry that still
 * runs the intensity step; above it the source forces a transition. */
#define DM2_V1_UPDATE_WEATHER_MAX_RETRY 0x1f

/* Requeue delay bounds: RAND16(256) + 50. */
#define DM2_V1_UPDATE_WEATHER_REQUEUE_MIN 50
#define DM2_V1_UPDATE_WEATHER_REQUEUE_MAX 305

/* Bound table sizes. */
#define DM2_V1_UPDATE_WEATHER_TABLE1D6B76_LEN 132
#define DM2_V1_UPDATE_WEATHER_PATTERN_LEN 128
#define DM2_V1_UPDATE_WEATHER_PATTERN_ROWS 4
#define DM2_V1_UPDATE_WEATHER_MAX_ZONE 31

/* Weather-flag offset of zone z inside table1d6b76: 4*z + 0x70. */
#define DM2_V1_UPDATE_WEATHER_FLAG_BASE 0x70

/* Mutable weather state — the v1e1472-family globals of the source,
 * carried explicitly so tests can drive synthetic states. Types match
 * src/dm2/dm2data.h. */
typedef struct DM2_V1_UpdateWeatherState {
  int16_t zone_index;         /* v1e1472 (i16): current map zone      */
  int8_t  weather_allowed;    /* v1e147f (i8): table1d6b76 flag       */
  int8_t  retry;              /* v1e147b (i8): pre-increment retry    */
  int8_t  pattern_row;        /* v1e1478 (i8): v1d7108 row selector   */
  int8_t  step;               /* v1e1484 (i8): intensity step scale   */
  int16_t intensity;          /* v1e1474 (i16): current intensity     */
  int16_t previous_intensity; /* v1e146e (i16): previous snapshot     */
} DM2_V1_UpdateWeatherState;

/* Receipt for one 0x54 -> DM2_UPDATE_WEATHER(1) call. */
typedef struct DM2_V1_UpdateWeatherReceipt {
  int     valid;                /* 1 when the bounded slice ran          */
  int     weather_allowed;      /* v1e147f read from table1d6b76         */
  int     retry;                /* post-increment retry value used       */
  int     pattern_delta;        /* (i8)v1d7108[(row<<5)+retry]           */
  int16_t intensity_before;     /* v1e1474 before the step               */
  int16_t intensity_after;      /* v1e1474 after step + clamp            */
  int     transition_forced;    /* 1 when retry > 0x1f (no requeue)      */
  int     reschedule_delay;     /* RAND16(256)+50, or -1 when no requeue */
  int     rand_draw;            /* raw RAND16(256) value (0..255), or -1 */
} DM2_V1_UpdateWeatherReceipt;

/* Bound source tables (defined in the .c file):
 *   dm2_v1_update_weather_table1d6b76 — dm2data.cpp:889-893 verbatim
 *   dm2_v1_update_weather_pattern     — v1d7108.dat verbatim (128 bytes)
 */
extern const uint8_t
    dm2_v1_update_weather_table1d6b76[DM2_V1_UPDATE_WEATHER_TABLE1D6B76_LEN];
extern const int8_t
    dm2_v1_update_weather_pattern[DM2_V1_UPDATE_WEATHER_PATTERN_LEN];

/* Run one arg == 1 DM2_UPDATE_WEATHER step against `state`.
 *
 * Follows c_weather.cpp:36-65 exactly:
 *   1. read the zone weather flag into state->weather_allowed;
 *   2. ++retry; if retry > 0x1f: set transition_forced, no intensity
 *      mutation, no requeue, reschedule_delay = -1;
 *   3. snapshot previous_intensity, apply step * pattern delta, clamp
 *      to [0, 0xff];
 *   4. draw RAND16(256) from `rng` and report the requeue delay.
 *
 * Bounds: zone_index in 0..31 and pattern_row in 0..3 are required;
 * anything else returns 0 with no state mutation and receipt->valid=0.
 * `rng` is only advanced on the requeue path.
 *
 * Returns 1 when the slice ran (either stepped+requeued or forced a
 * transition), 0 on bounds failure. `out_receipt` may be NULL. */
int dm2_v1_update_weather_1(DM2_V1_UpdateWeatherState *state,
                            DM2_V1_DropRng *rng,
                            DM2_V1_UpdateWeatherReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_UPDATE_WEATHER_PC34_COMPAT_H */
