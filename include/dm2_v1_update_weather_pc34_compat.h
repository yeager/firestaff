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
 *   - The arg == 0 branch of DM2_UPDATE_WEATHER (frame update: day
 *     rollover, lightning evaluation, light/cloud command selection) is
 *     bound by dm2_v1_update_weather_0 below.
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
 * src/dm2/dm2data.h (v1e1434/v1e1438/v1d7188 i32, v1e1476/v1e1470 i16,
 * v1e147d/v1e147e/v1e1480/v1e1482/v1e1483 i8). */
typedef struct DM2_V1_UpdateWeatherState {
  int16_t zone_index;         /* v1e1472 (i16): current map zone      */
  int8_t  weather_allowed;    /* v1e147f (i8): table1d6b76 flag       */
  int8_t  retry;              /* v1e147b (i8): pre-increment retry    */
  int8_t  pattern_row;        /* v1e1478 (i8): v1d7108 row selector   */
  int8_t  step;               /* v1e1484 (i8): intensity step scale   */
  int16_t intensity;          /* v1e1474 (i16): current intensity     */
  int16_t previous_intensity; /* v1e146e (i16): previous snapshot     */
  int32_t day_tick;           /* v1e1434 (i32): next day-rollover tick */
  int32_t day_offset;         /* v1e1438 (i32): session day offset     */
  int16_t cloud_timer;        /* v1e1470 (i16): RAND16(4)+4 countdown  */
  int16_t day_word;           /* v1e1476 (i16): table1d70f0[hour]      */
  int8_t  rain_counter;       /* v1e147d (i8): rain intensity counter  */
  int8_t  cloud_state;        /* v1e147e (i8): cloud state selector    */
  int8_t  storm_active;       /* v1e1480 (i8): storm flag              */
  int8_t  lightning_flag;     /* v1e1482 (i8): lightning state         */
  int8_t  wind_dir;           /* v1e1483 (i8): RANDDIR wind direction  */
  int32_t storm_request;      /* v1d7188 (i32): caller storm-forcing   */
  int8_t  clouds_enabled;     /* v1e147a (i8): cloud command gate      */
  int8_t  rain_enabled;       /* v1e1479 (i8): rain command gate       */
  int8_t  lightning_enabled;  /* v1e1481 (i8): flash-eval gate         */
  int8_t  light_pending;      /* v1e024c (i8): light-change pending    */
  int8_t  thunder_latch;      /* v1d718c (i8): thunder-sound latch     */
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
 *   dm2_v1_update_weather_table1d6b76 — dm2data.cpp:889-896 verbatim
 *   dm2_v1_update_weather_pattern     — v1d7108.dat verbatim (128 bytes)
 *   dm2_v1_weather_table1d70f0        — dm2data.cpp:182-191 verbatim
 *     (24 time-of-day words indexed by (gametick+offset)/0x555 % 0x18)
 */
extern const uint8_t
    dm2_v1_update_weather_table1d6b76[DM2_V1_UPDATE_WEATHER_TABLE1D6B76_LEN];
extern const int8_t
    dm2_v1_update_weather_pattern[DM2_V1_UPDATE_WEATHER_PATTERN_LEN];
#define DM2_V1_WEATHER_TABLE1D70F0_LEN 24
extern const int8_t
    dm2_v1_weather_table1d70f0[DM2_V1_WEATHER_TABLE1D70F0_LEN];

/* Day length in gameticks (c_weather.cpp:96 lcon(0x555)). */
#define DM2_V1_WEATHER_DAY_TICKS 0x555
/* Hours per day (c_weather.cpp:97 lcon(0x18)). */
#define DM2_V1_WEATHER_HOURS_PER_DAY 0x18

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

/* Receipt for one DM2_weather_3df7_0037 call
 * (c_weather.cpp:509-567 — the weather transition/reseed). */
typedef struct DM2_V1_WeatherTransitionReceipt {
  int   valid;                  /* 1 when the bounded slice ran          */
  int   arg;                    /* arg passed in (0 = full transition)   */
  int   reseeded;               /* arg==0: pattern/step reseeded         */
  int   storm_path;             /* arg==0 with v1d7188 != 0 branch       */
  int   light_update_requested; /* DM2_UPDATE_GLOB_VAR(0x40,0,6): host   */
  int   queue_delay;            /* DM2_SET_TIMER_WEATHER delay, -1 none  */
  int   pattern_row;            /* v1e1478 after the call                */
  int   step;                   /* v1e1484 after the call                */
  int16_t cloud_timer;          /* v1e1470 = RAND16(4)+4                 */
  int16_t day_word;             /* v1e1476 = table1d70f0[hour]           */
  int   hour;                   /* (gametick+offset)/0x555 % 0x18        */
  int32_t days;                 /* return value: days elapsed            */
  int   draws;                  /* LCG draws consumed                    */
} DM2_V1_WeatherTransitionReceipt;

/* Run DM2_weather_3df7_0037 (c_weather.cpp:509-567) against `state`.
 *
 * arg == 0 (full transition, c_weather.cpp:518-555):
 *   - requests the host-owned light update DM2_UPDATE_GLOB_VAR(0x40,0,6)
 *     (receipt flag only — the light subsystem is not part of this
 *     slice);
 *   - day_tick = gametick + 0x555; storm_active = 0; weather_allowed = 0;
 *   - storm_request == 0: queue_delay = RAND16(8000) + 500,
 *     pattern_row = RANDDIR(), step = RAND16(3) + 1;
 *   - storm_request != 0: rain_counter = 0, queue_delay = RAND16(500),
 *     pattern_row = 3, step = 1;
 *   - cloud_state = 1, lightning_flag = 0, intensity = 0,
 *     previous_intensity = 0, retry = 0, wind_dir = RANDDIR();
 *   - the source queues the next 0x54 timer with queue_delay.
 * arg != 0 (c_weather.cpp:557-560):
 *   - previous_intensity = 0; step floored to 1 when 0; no reseed, no
 *     requeue (queue_delay = -1).
 * Common tail (c_weather.cpp:562-567, both args):
 *   - cloud_timer = RAND16(4) + 4;
 *   - t = (gametick + day_offset) / 0x555; hour = t % 0x18;
 *     day_word = table1d70f0[hour]; storm_request = 0;
 *   - returns days = t / 0x18.
 *
 * `rng` is required (the source draws unconditionally); NULL returns 0
 * with no mutation. Bounds: day_offset/gametick are summed as i32 like
 * the source. Returns days elapsed (0 when the slice did not run). */
int32_t dm2_v1_weather_transition(DM2_V1_UpdateWeatherState *state,
                                  int32_t gametick, int arg,
                                  DM2_V1_DropRng *rng,
                                  DM2_V1_WeatherTransitionReceipt *out_receipt);

/* ── arg == 0 frame update (c_weather.cpp:91-506) ─────────────────── */

/* retrieve_mask bits: host-owned DM2_RETRIEVE_ENVIRONMENT_CMD_CD_FW
 * outcomes for the three command slots this slice can write, in write
 * order (cloud, rain, lightning bolt). A clear bit means the source
 * does NOT advance the 10-byte slot pointer, so the next command (or
 * the 0xff terminator) overwrites the same slot. */
#define DM2_V1_UPDATE_WEATHER_RETRIEVE_CLOUD 0x1u
#define DM2_V1_UPDATE_WEATHER_RETRIEVE_RAIN  0x2u
#define DM2_V1_UPDATE_WEATHER_RETRIEVE_BOLT  0x4u

/* Receipt for one DM2_UPDATE_WEATHER(0) frame update. */
typedef struct DM2_V1_UpdateWeatherFrameReceipt {
  int     valid;                 /* 1 when the bounded slice ran        */
  int     weather_allowed;       /* v1e147f read from table1d6b76       */
  int     day_rolled;            /* day rollover ran (c_weather.cpp:92) */
  int     hour;                  /* rollover hour, -1 when none         */
  int16_t day_word;              /* table1d70f0[hour], -1 when none     */
  int     flash_eval;            /* vql_28 after evaluation             */
  int     weather_gate;          /* 0 = early exit at c_weather.cpp:201 */
  int     cloud_cmd;             /* 0x67/0x68/0x69 or 0 (not written)   */
  int     rain_cmd;              /* 0x6a/0x6b/0x6c or 0                 */
  int     bolt_cmd;              /* 100..102 (0x64..0x66) or 0          */
  int     storm_set;             /* cloud cmd 0x69 set storm_active     */
  uint8_t live_cmds[3];          /* final slot bytes before 0xff        */
  int     slots;                 /* live 10-byte slots used (0..3)      */
  int     light_recalc_requests; /* RECALC_LIGHT_LEVEL handoffs (host)  */
  int     light_flash_request;   /* UPDATE_GLOB_VAR(0x40,0,6) (host)    */
  int     thunder_count;         /* vql_2c cloud attempts 0..8          */
  int     thunder_sound;         /* QUEUE_NOISE_GEN1 handoff (host)     */
  int     thunder_volume;        /* clamped 1..15, -1 when no sound     */
  int     cloud_placement_request;  /* CREATE_CLOUD loop (host, ends)   */
  int     invoke_message_request;   /* INVOKE_MESSAGE path (host, ends) */
  int     rng_diverges;          /* host continuation draws unbound     */
  int     bolt_rect_rand;        /* RAND16(100) for host rect, -1 n/a   */
  int     bolt_dir;              /* RANDDIR slot byte, -1 n/a           */
  int     draws;                 /* LCG draws consumed by this slice    */
} DM2_V1_UpdateWeatherFrameReceipt;

/* Run one arg == 0 DM2_UPDATE_WEATHER frame update (c_weather.cpp:91-506)
 * against `state`.
 *
 * Bound inside the slice, in source order:
 *   1. v1e147f = table1d6b76[4*zone + 0x70] (c_weather.cpp:60-64, runs
 *      for both args);
 *   2. day rollover (92-105): day_word = table1d70f0[hour],
 *      day_tick = gametick + 0x555, light-recalc handoff when weather
 *      is allowed;
 *   3. lightning evaluation (106-199) with the exact RNG draw order:
 *      intensity == 0: rain_counter decay every 3rd tick, flash =
 *      RAND16(64) == 0, lightning_flag = 0, cloud_state = 1;
 *      intensity != 0: threshold = (0x100 - intensity + (RAND&0xf)) as
 *      u16, RG51w = intensity < 0xcd ? 7 : 0x28, cloud_state =
 *      CUTX8(intensity), lightning_flag latch via RAND16(threshold) <=
 *      7, rain_counter increment gating (148-183), flash = RAND16(
 *      threshold) <= RG51w when v1e1481 != 0;
 *   4. weather gate (201-203): weather_allowed == 0 exits;
 *   5. v1e024c consumed (205-209): cleared, light-recalc handoff;
 *   6. cloud command (213-237): cloud_state >= 0x10 -> 0x67 (<0x40),
 *      0x68 (<0x80) or 0x69 + storm_active; rain command (239-256):
 *      rain_counter >= 0x40 -> 0x6a (<0x80), 0x6b (<0xc0) or 0x6c;
 *   7. lightning execution (258-275): flash handoff when intensity <
 *      0xb6; thunder_count = (RAND&7)+1 when RAND16(intensity+1) >=
 *      60 — the cloud-placement/message branches that follow are
 *      host-owned: the slice ends with cloud_placement_request (GDAT
 *      entry absent) or invoke_message_request plus rng_diverges;
 *   8. lightning bolt command (441-474): when thunder_count == 0 and
 *      RANDBIT: cmd = 100 + RAND16(3); on retrieve ok the slice draws
 *      RAND16(100) (bolt_rect_rand, host rect geometry) and RANDDIR
 *      (bolt_dir slot byte);
 *   9. thunder sound + light (476-504): v1d718c latch toggle, volume =
 *      clamp(1..15, intensity != 0 ? 0x4c - intensity/step : RAND16(10)
 *      + 5), thunder_sound handoff, then v1e024c = 1 + light-recalc
 *      handoff (m_4A899);
 *  10. the source terminates the slot chain with 0xff (m_4A8A8);
 *      live_cmds/slots report the compacted chain.
 *
 * Host-owned (receipt flags only, never drawn or queued here):
 * DM2_RECALC_LIGHT_LEVEL, DM2_UPDATE_GLOB_VAR, DM2_CREATE_CLOUD,
 * DM2_INVOKE_MESSAGE, DM2_QUEUE_NOISE_GEN1/GEN2, the bolt rect geometry
 * DM2_rect_098d_04c7, and DM2_RETRIEVE_ENVIRONMENT_CMD_CD_FW itself
 * (its outcomes arrive via retrieve_mask; gdat_entry_6c is the host's
 * QUERY_GDAT_ENTRY_DATA_INDEX(8, v1d6c02, 0xb, 0x6c) value, 0 = entry
 * absent -> cloud placement path).
 *
 * Bounds / fail-closed: zone_index 0..31 is required (table read);
 * intensity != 0 with step == 0 is rejected up front (the source would
 * divide by v1e1484 in the thunder-volume path). NULL state or rng
 * returns 0 with no mutation (the source draws on every path). */
int dm2_v1_update_weather_0(DM2_V1_UpdateWeatherState *state,
                            int32_t gametick,
                            unsigned int retrieve_mask,
                            uint16_t gdat_entry_6c,
                            DM2_V1_DropRng *rng,
                            DM2_V1_UpdateWeatherFrameReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_UPDATE_WEATHER_PC34_COMPAT_H */
