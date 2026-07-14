/* CSBWin resumed TIMER/TimerQueue restart-export regression.
 * Source: SaveGame.cpp body write order; Timer.cpp CheckTimers. */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (condition) {
        printf("PASS: %s\n", message);
    } else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void set_timer(CSB_V1_CSBWin512TimerSummary *timer,
                      uint16_t index, uint32_t time, uint8_t function,
                      uint16_t sequence)
{
    memset(timer, 0, sizeof(*timer));
    timer->valid = 1;
    timer->source_index = index;
    timer->time = time;
    timer->function = function;
    timer->ubyte5 = (uint8_t)(index + 3u);
    timer->ubyte6 = (uint8_t)(index + 4u);
    timer->ubyte7 = (uint8_t)(index + 5u);
    timer->ubyte8 = (uint8_t)(index + 6u);
    timer->ubyte9 = (uint8_t)(index + 7u);
    timer->sequence = sequence;
    timer->level = 0u;
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_CSBWin512BodyReport report;
    uint8_t bytes[8192];
    size_t size = 0u;
    int event_index;

    csb_v1_runtime_init(&profile, NULL);
    profile.party_state_valid = 1;
    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_max_timers = 3u;
    profile.csbwin_num_timer = 3u;
    profile.csbwin_first_avail_timer = 3u;
    profile.csbwin_timer_sequence = 0x90u;
    profile.csbwin_timer_summary_count = 3u;
    profile.csbwin_timer_summary_total = 3u;
    profile.csbwin_timer_queue_summary_count = 3u;
    profile.csbwin_timer_queue_summary_total = 3u;
    set_timer(&profile.csbwin_timers[0], 0u, 101u, 7u, 0x31u);
    set_timer(&profile.csbwin_timers[1], 1u, 102u, 8u, 0x42u);
    set_timer(&profile.csbwin_timers[2], 2u, 100u, 6u, 0x53u);
    profile.csbwin_timer_queue[0] = 2u;
    profile.csbwin_timer_queue[1] = 0u;
    profile.csbwin_timer_queue[2] = 1u;

    check(csb_v1_runtime_materialize_csbwin_timer_queue(&profile) == 3,
          "resumed CSBWin queue materializes with its serialized heap order");
    check(csb_v1_runtime_export_csbwin_core_save_to_memory(
              &profile, bytes, sizeof(bytes), &size) == 0,
          "exact resumed timer receipts permit CSBWin core export");
    memset(&report, 0, sizeof(report));
    check(csb_v1_csbwin_512_verify_save_body(bytes, size, 0u, &report) ==
              CSB_V1_CSBWIN_512_OK,
          "preserved CSBWin timer export passes the production body verifier");
    check(report.timer_queue[0] == 2u && report.timer_queue[1] == 0u &&
              report.timer_queue[2] == 1u &&
              report.timers[2].sequence == 0x53u &&
              report.timers[0].sequence == 0x31u &&
              report.timers[1].sequence == 0x42u,
          "export retains source timer indexes, heap topology, and sequences");
    check(report.max_timers == 3u && report.num_timer == 3u &&
              report.first_avail_timer == 3u && report.timer_sequence == 0x90u,
          "export retains source GAMEBLOCK2 timer counters");

    event_index = profile.timeline_queue.timeline[0];
    profile.timeline_queue.events[event_index].c_effect ^= 1u;
    check(csb_v1_runtime_export_csbwin_core_save_to_memory(
              &profile, bytes, sizeof(bytes), &size) != 0,
          "unmatched live event rejects export instead of inventing a replacement queue");

    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
