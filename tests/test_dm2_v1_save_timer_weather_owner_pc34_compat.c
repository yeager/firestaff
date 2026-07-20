/* DM2-011 saved weather-timer owner proof.
 *
 * Source lock (skproject/SKULLWIN):
 *   c_weather.cpp:22-30     DM2_SET_TIMER_WEATHER: type 0x54, actor 0,
 *                           map 0, mticks = gametick + delay
 *   c_weather.cpp:85-88     DM2_UPDATE_WEATHER(1) re-queues the chain with
 *                           DM2_RAND16(256) + 50 ticks
 *   c_savegame.cpp:1486-1525 the queued 0x54 record round-trips inside the
 *                           serialized timer array; gametick is restored
 *                           from the same header (s33_00.l_00)
 *   c_tim_proc.cpp:4179-4183 type 0x54 dispatches to DM2_UPDATE_WEATHER(1)
 *
 * Synthetic records in the exact 12-byte wire layout (c_timer.h:8-46);
 * no game data required. */

#include "dm2_v1_save_timers_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_passed;
static int g_failed;

#define CHECK(cond, msg) do { \
    if (cond) { \
        g_passed++; \
        printf("  PASS: %s\n", msg); \
    } else { \
        g_failed++; \
        printf("  FAIL: %s\n", msg); \
    } \
} while (0)

/* Queue a weather timer exactly like DM2_SET_TIMER_WEATHER
 * (c_weather.cpp:22-30): mticks = gametick + delay, map 0, type 0x54,
 * actor 0. */
static void queue_weather_timer(DM2_V1_SaveTimerRecord *t,
                                int32_t gametick, int32_t delay)
{
    int32_t mticks = gametick + delay;

    memset(t, 0, sizeof(*t));
    t->bytes[0] = (uint8_t)((uint32_t)mticks & 0xffu);
    t->bytes[1] = (uint8_t)(((uint32_t)mticks >> 8) & 0xffu);
    t->bytes[2] = (uint8_t)(((uint32_t)mticks >> 16) & 0xffu);
    t->bytes[3] = 0u;    /* map 0 */
    t->bytes[4] = 0x54u; /* ttype */
    t->bytes[5] = 0u;    /* actor */
}

int main(void)
{
    DM2_V1_SaveTimerRecord weather;
    DM2_V1_SaveTimerRecord other;
    DM2_V1_SaveTimerWeatherOwnerReceipt receipt;
    DM2_V1_SaveTimerWeatherOwnerReceipt receipt_b;
    int16_t indices[2];
    int32_t gametick = 100000;
    int32_t delay = 182;

    /* ── 1. owner identity round-trips the save ─────────────────────── */
    queue_weather_timer(&weather, gametick, delay);
    CHECK(dm2_v1_save_timer_weather_owner_receipt(&weather, gametick,
                                                  &receipt) &&
              receipt.valid && receipt.type == 0x54u &&
              receipt.actor == 0u && receipt.map == 0u &&
              receipt.target_tick == gametick + delay &&
              receipt.restored_gametick == gametick &&
              receipt.remaining_ticks == delay &&
              !receipt.fires_on_next_proceed && receipt.owner_hash != 0u,
          "saved 0x54 record binds its DM2_UPDATE_WEATHER(1) owner");

    /* ── 2. an overdue restored timer fires on the next proceed ─────── */
    CHECK(dm2_v1_save_timer_weather_owner_receipt(&weather,
                                                  gametick + delay + 7,
                                                  &receipt_b) &&
              receipt_b.valid && receipt_b.remaining_ticks == -7 &&
              receipt_b.fires_on_next_proceed &&
              receipt_b.owner_hash != receipt.owner_hash,
          "overdue restored weather timer fires on next proceed, not dropped");

    /* ── 3. owner rejects non-chain records ─────────────────────────── */
    other = weather;
    other.bytes[4] = 0x55u; /* ornate animator */
    CHECK(!dm2_v1_save_timer_weather_owner_receipt(&other, gametick,
                                                   &receipt),
          "type 0x55 record is not a weather-chain timer");
    other = weather;
    other.bytes[5] = 1u;
    CHECK(!dm2_v1_save_timer_weather_owner_receipt(&other, gametick,
                                                   &receipt),
          "actor != 0 record is not the source weather timer");
    other = weather;
    other.bytes[3] = 2u;
    CHECK(!dm2_v1_save_timer_weather_owner_receipt(&other, gametick,
                                                   &receipt),
          "map != 0 record cannot be the setmticks(0, ...) weather timer");
    memset(&other, 0, sizeof(other));
    CHECK(!dm2_v1_save_timer_weather_owner_receipt(&other, gametick,
                                                   &receipt),
          "cleared (notype) slot has no weather owner");
    CHECK(!dm2_v1_save_timer_weather_owner_receipt(NULL, gametick,
                                                   &receipt),
          "null record fails closed");

    /* ── 4. restored queue order keeps the weather timer due ────────── */
    {
        /* Source-order dispatch (DM2_cmp_timers, c_timer.cpp:31-48):
         * ticks ascending first, so the restored 0x54 timer precedes a
         * later non-weather timer on the sorted queue. */
        DM2_V1_SaveTimerRecord records[2];
        DM2_V1_SaveTimerWeatherOwnerReceipt sorted_rc;

        queue_weather_timer(&records[1], gametick, delay);
        memset(&records[0], 0, sizeof(records[0]));
        records[0].bytes[0] = 0xffu; /* far-future torch timer */
        records[0].bytes[1] = 0xffu;
        records[0].bytes[2] = 0x7fu;
        records[0].bytes[4] = 0x30u;
        records[0].bytes[5] = 1u;
        dm2_v1_save_timer_sort(records, 2u, indices);
        CHECK(indices[0] == 1 &&
                  dm2_v1_save_timer_weather_owner_receipt(
                      &records[indices[0]], gametick, &sorted_rc) &&
                  sorted_rc.valid && sorted_rc.remaining_ticks == delay,
              "sorted restored queue dispatches the weather timer first");
    }

    /* ── 5. source reschedule bounds ────────────────────────────────── */
    CHECK(DM2_V1_SAVE_TIMER_WEATHER_RESCHEDULE_MIN == 50 &&
              DM2_V1_SAVE_TIMER_WEATHER_RESCHEDULE_MAX == 50 + 255,
          "requeue delay stays inside RAND16(256)+50 (c_weather.cpp:86)");

    printf("DM2 saved weather-timer owner: %d passed, %d failed\n",
           g_passed, g_failed);
    return g_failed ? 1 : 0;
}
