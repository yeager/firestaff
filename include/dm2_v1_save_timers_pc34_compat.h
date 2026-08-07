#ifndef FIRESTAFF_DM2_V1_SAVE_TIMERS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_SAVE_TIMERS_PC34_COMPAT_H

/*
 * dm2_v1_save_timers_pc34_compat.h — DM2 V1 savegame timer payload
 * materialisation (DM2-009 bounded slice).
 *
 * The saved timer-record byte layout is proven by skproject's own mask
 * table, closing the layout half of SKPROJECT-GAP-001 for timer records:
 *
 *   c_timer.h:8-46        c_tim, 0xc bytes: union { w_00, dummy2, b_03 } /
 *                         l_00 @0, ttype @4, actor @5, wvalueA @6,
 *                         wvalueB @8, dummya @0xa
 *   dm2data.cpp:97-99     vsgame[120]; v1d6463 = vsgame + 0x00
 *                         (dm2data.h:608) — the per-record SUPPRESS mask
 *                         {ff ff ff 3f 7f ff ff ff ff ff 00 00}
 *   c_savegame.cpp:655-733 DM2_SUPPRESS_READER: mask re-armed per record
 *                         (bp_00 -= record size), bit state carried across
 *                         records, masked-off bytes decode to 0
 *   c_savegame.cpp:1493   timdat.num_timers = s33_00.w_14
 *   c_savegame.cpp:1517   SUPPRESS_READER(timerarray, v1d6463, 0xc,
 *                         num_timers, true)
 *   c_savegame.cpp:1519-1524 clrtype() for [num_timers, max_timers)
 *   c_savegame.cpp:1525   DM2_SORT_TIMERS
 *   c_timer.cpp:31-48     DM2_cmp_timers (ticks asc, type desc, actor desc,
 *                         record-address asc)
 *   c_timer.cpp:126-194   DM2_SORT_TIMERS: identity indices + heapify
 *   c_timer.cpp:97-122    DM2_REARRANGE_TIMERLIST free-chain rebuild
 *
 * Fail-closed contract: the stream is decoded into module scratch first and
 * only published on full success, so an underflow leaves caller state
 * untouched (matching the source's M_exit abort).  num_timers > max_timers
 * rejects.  dummya (bytes 10-11) is never restored (mask 0x00).  The
 * post-load DM2_READ_SKSAVE_DUNGEON / DM2_PROCEED_GLOBAL_EFFECT_TIMERS
 * rebuild stays outside this slice and is receipted pending.
 */

#include <stdint.h>

#include "dm2_v1_save_load.h" /* DM2_SuppressReader, dm2_suppress_reader_read */

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_SAVE_TIMER_RECORD_SIZE 12u /* c_timer.h:8 sizeof(c_tim) 0xc */
#define DM2_V1_SAVE_TIMER_MASK_SIZE 12u
#define DM2_V1_SAVE_TIMER_MAX 64u /* module cap; source max_timers base 50 */

typedef struct {
    uint8_t bytes[DM2_V1_SAVE_TIMER_RECORD_SIZE];
} DM2_V1_SaveTimerRecord;

typedef struct {
    int valid;
    int rejected_bounds;
    int rejected_stream_underflow;
    uint16_t decoded_count;
    uint16_t cleared_count;
    int16_t num_timer_indices;   /* DM2_REARRANGE_TIMERLIST result */
    int16_t available_timeridx;  /* free-chain head, -1 none */
    uint32_t payload_hash;       /* FNV-1a over decoded record bytes */
    int post_load_rebuild_pending; /* READ_SKSAVE_DUNGEON +
                                    * PROCEED_GLOBAL_EFFECT_TIMERS outside */
} DM2_V1_SaveTimerReceipt;

/* The verified 12-byte per-record mask (v1d6463 = vsgame+0x00,
 * dm2data.cpp:97-99). */
const uint8_t *dm2_v1_save_timers_suppress_mask(void);

/* c_tim accessors over the exact 12-byte wire layout (c_timer.h:8-96). */
int16_t dm2_v1_save_timer_get_dataw(const DM2_V1_SaveTimerRecord *t);
uint8_t dm2_v1_save_timer_get_map(const DM2_V1_SaveTimerRecord *t);
int32_t dm2_v1_save_timer_get_ticks(const DM2_V1_SaveTimerRecord *t);
uint8_t dm2_v1_save_timer_get_type(const DM2_V1_SaveTimerRecord *t);
uint8_t dm2_v1_save_timer_get_actor(const DM2_V1_SaveTimerRecord *t);
int16_t dm2_v1_save_timer_get_a(const DM2_V1_SaveTimerRecord *t);
int16_t dm2_v1_save_timer_get_b(const DM2_V1_SaveTimerRecord *t);
int dm2_v1_save_timer_is_no_type(const DM2_V1_SaveTimerRecord *t);
void dm2_v1_save_timer_clr_type(DM2_V1_SaveTimerRecord *t);
void dm2_v1_save_timer_set_dataw(DM2_V1_SaveTimerRecord *t, int16_t n);
void dm2_v1_save_timer_set_b(DM2_V1_SaveTimerRecord *t, int16_t n);

/* DM2_cmp_timers, c_timer.cpp:31-48.  The source's pointer tiebreak is the
 * record-array address; array indices are the identical order for
 * array-resident records. */
int dm2_v1_save_timer_cmp(const DM2_V1_SaveTimerRecord *a, int a_index,
                          const DM2_V1_SaveTimerRecord *b, int b_index);

/* DM2_SORT_TIMERS, c_timer.cpp:126-194: identity index fill + heapify.
 * indices must hold at least num_timers entries. */
void dm2_v1_save_timer_sort(const DM2_V1_SaveTimerRecord *records,
                            uint16_t num_timers,
                            int16_t *indices);

/* DM2_REARRANGE_TIMERLIST, c_timer.cpp:97-122: rebuild num_timer_indices
 * and the free-slot dataw chain over [0, max_timers). */
void dm2_v1_save_timer_rearrange(DM2_V1_SaveTimerRecord *records,
                                 uint16_t max_timers,
                                 int16_t *out_num_timer_indices,
                                 int16_t *out_available_timeridx);

/* GAME_LOAD timer section order (c_savegame.cpp:1517-1527): decode
 * num_timers records through the verified mask, clrtype the remainder up
 * to max_timers, heap-sort indices, rebuild the free chain.  Decodes into
 * scratch first: on underflow the caller's records/indices stay untouched
 * and the receipt marks rejected_stream_underflow. */
int dm2_v1_save_timer_materialize(DM2_SuppressReader *reader,
                                  uint16_t num_timers,
                                  uint16_t max_timers,
                                  DM2_V1_SaveTimerRecord *records,
                                  int16_t *indices,
                                  DM2_V1_SaveTimerReceipt *out_receipt);

/* ── Saved weather-timer owner proof (DM2-011) ────────────────────────
 *
 * Source lock:
 *   c_weather.cpp:22-30   DM2_SET_TIMER_WEATHER: tim.setmticks(0,
 *                         gametick + delay), tim.settype(0x54),
 *                         tim.setactor(0)
 *   c_weather.cpp:85-88   DM2_UPDATE_WEATHER(1) re-queues the next 0x54
 *                         timer with DM2_RAND16(256) + 50 ticks
 *   c_savegame.cpp:1486-1487 restored gametick = s33_00.l_00 (also
 *                         ddat.v1e021c)
 *   c_savegame.cpp:1493-1525 the queued 0x54 record is serialized with
 *                         the timer array and sorted back into the live
 *                         queue; the weather chain is NOT re-seeded on
 *                         load — the restored record itself carries the
 *                         owner continuity
 *   c_tim_proc.cpp:4179-4183 timer type 0x54 dispatches to
 *                         DM2_UPDATE_WEATHER(1) (m_49E8E)
 *
 * A saved record is a weather-chain timer iff ttype == 0x54, actor == 0,
 * and map == 0 (setmticks(0, ...)).  remaining_ticks is the signed delta
 * against the restored gametick; a non-positive value means the source
 * fires it on the next timer proceed, never that it is dropped. */
#define DM2_V1_SAVE_TIMER_TYPE_UPDATE_WEATHER 0x54u /* c_weather.cpp:26 */
#define DM2_V1_SAVE_TIMER_WEATHER_ACTOR 0u          /* c_weather.cpp:28 */
#define DM2_V1_SAVE_TIMER_WEATHER_RESCHEDULE_MIN 50  /* RAND16(256)+50 */
#define DM2_V1_SAVE_TIMER_WEATHER_RESCHEDULE_MAX 305 /* c_weather.cpp:86 */

typedef struct {
    int valid;
    uint8_t type;
    uint8_t actor;
    uint8_t map;
    int32_t target_tick;
    int32_t restored_gametick;
    int32_t remaining_ticks;
    int fires_on_next_proceed; /* remaining_ticks <= 0 */
    uint32_t owner_hash;
} DM2_V1_SaveTimerWeatherOwnerReceipt;

int dm2_v1_save_timer_weather_owner_receipt(
    const DM2_V1_SaveTimerRecord *record, int32_t restored_gametick,
    DM2_V1_SaveTimerWeatherOwnerReceipt *out_receipt);

const char *dm2_v1_save_timers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SAVE_TIMERS_PC34_COMPAT_H */
