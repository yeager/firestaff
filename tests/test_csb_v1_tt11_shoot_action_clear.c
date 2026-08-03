/* TT_11 SHOOT action-clear path regression.
 * ReDMCSB TIMELINE.C F0253: the SHOOT branch refills the ready hand from
 * the quiver, then clears the action lock. Previously the SHOOT branch
 * was fail-closed and skipped the action clear entirely. */

#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_character_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void setup_champion(CSB_V1_RuntimeProfile *profile, int ci,
                           uint8_t action_index)
{
    CSB_V1_Champion *c = &profile->party_state.Champions[ci];
    c->CurrentHealth = 100;
    c->Attributes = 0x0008u;
    c->EnableActionEventIndex = 5;
    c->CsbWinWord64 = 42;
    c->ActionIndex = action_index;
    for (int s = 0; s < CSB_V1_SLOT_COUNT; ++s)
        c->Slots[s] = THING_NONE;
}

static void setup_timer(CSB_V1_CsbWinTimer *timer, uint8_t champion_idx)
{
    memset(timer, 0, sizeof(*timer));
    timer->valid = 1;
    timer->source_index = 0;
    timer->time = 100;
    timer->level = 0;
    timer->function = DM1_EVENT_ENABLE_CHAMPION_ACTION;
    timer->ubyte5 = champion_idx;
    timer->ubyte6 = 5;
    timer->ubyte7 = 5;
    timer->ubyte8 = 0;
    timer->ubyte9 = 0;
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    struct DM1_Event_V1 event;
    CSB_V1_Champion *champion;

    csb_v1_runtime_init(&profile, NULL);
    profile.party_state_valid = 1;
    profile.champion_count = 1;
    profile.party_state.ChampionCount = 1;

    /* Test 1: Non-SHOOT action (ActionIndex != 32) clears state. */
    setup_champion(&profile, 0, 10);
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(0u, 100u);
    event.type = DM1_EVENT_ENABLE_CHAMPION_ACTION;
    event.b_mapX = 5;
    event.b_mapY = 5;
    event.c_cell = 0;
    event.c_effect = 0;

    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_max_timers = 1;
    profile.csbwin_num_timer = 1;
    profile.csbwin_first_avail_timer = 1;
    profile.csbwin_timer_summary_count = 1;
    profile.csbwin_timer_summary_total = 1;
    profile.csbwin_timer_queue_summary_count = 1;
    profile.csbwin_timer_queue_summary_total = 1;
    setup_timer(&profile.csbwin_timers[0], 0);
    profile.csbwin_timer_queue[0] = 0;

    csb_v1_runtime_materialize_csbwin_timer_queue(&profile);
    csb_v1_runtime_dispatch_next_event(&profile);

    champion = &profile.party_state.Champions[0];
    check(champion->ActionIndex == CSB_V1_ACTION_NONE,
          "non-SHOOT TT_11 clears ActionIndex");
    check(champion->EnableActionEventIndex == -1,
          "non-SHOOT TT_11 clears EnableActionEventIndex");
    check((champion->Attributes & 0x0008u) == 0,
          "non-SHOOT TT_11 clears action-locked attribute");
    check(champion->CsbWinWord64 == 0,
          "non-SHOOT TT_11 clears CsbWinWord64");

    /* Test 2: SHOOT action (ActionIndex == 32) also clears state.
     * Ready hand refill will fail (no weapon info), but the action lock
     * must still clear — that was the previous bug. */
    csb_v1_runtime_cleanup(&profile);
    csb_v1_runtime_init(&profile, NULL);
    profile.party_state_valid = 1;
    profile.champion_count = 1;
    profile.party_state.ChampionCount = 1;
    setup_champion(&profile, 0, 32);

    profile.csbwin_body_runtime_summary_valid = 1;
    profile.csbwin_max_timers = 1;
    profile.csbwin_num_timer = 1;
    profile.csbwin_first_avail_timer = 1;
    profile.csbwin_timer_summary_count = 1;
    profile.csbwin_timer_summary_total = 1;
    profile.csbwin_timer_queue_summary_count = 1;
    profile.csbwin_timer_queue_summary_total = 1;
    setup_timer(&profile.csbwin_timers[0], 0);
    profile.csbwin_timer_queue[0] = 0;

    csb_v1_runtime_materialize_csbwin_timer_queue(&profile);
    csb_v1_runtime_dispatch_next_event(&profile);

    champion = &profile.party_state.Champions[0];
    check(champion->ActionIndex == CSB_V1_ACTION_NONE,
          "SHOOT TT_11 clears ActionIndex");
    check(champion->EnableActionEventIndex == -1,
          "SHOOT TT_11 clears EnableActionEventIndex");
    check((champion->Attributes & 0x0008u) == 0,
          "SHOOT TT_11 clears action-locked attribute");
    check(champion->CsbWinWord64 == 0,
          "SHOOT TT_11 clears CsbWinWord64");

    csb_v1_runtime_cleanup(&profile);
    return failures == 0 ? 0 : 1;
}
