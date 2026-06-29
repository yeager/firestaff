#include "dm1/dm1_v1_champion_panel_damage_flash_decay_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void check_int(const char *label, int actual, int expected,
                      const char *anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d at %s\n",
               label, actual, expected, anchor);
    } else {
        printf("PASS %s == %d (%s)\n", label, expected, anchor);
    }
}

static void check_true(const char *label, int value, const char *anchor)
{
    check_int(label, value ? 1 : 0, 1, anchor);
}

static void check_false(const char *label, int value, const char *anchor)
{
    check_int(label, value ? 1 : 0, 0, anchor);
}

static void check_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing='%s' at %s\n",
               label, needle ? needle : "(null)", anchor);
    } else {
        printf("PASS %s contains '%s' (%s)\n", label, needle, anchor);
    }
}

static void test_evidence(void)
{
    const DM1_V1_ChampionPanelDamageFlashDecayEvidencePc34Compat *evidence =
        DM1_V1_ChampionPanelDamageFlashDecay_EvidencePc34Compat();

    check_true("evidence.contract_only", evidence->contract_only,
               "CHAMPION.C F0320:1758-1792 contract-only route");
    check_contains("evidence.applier", evidence->applier_function_anchor,
                   "F0320", "CHAMPION.C F0320:1758-1792");
    check_contains("evidence.hide_handler", evidence->hide_event_handler_anchor,
                   "F0254", "TIMELINE.C F0254:1614-1637");
    check_contains("evidence.timeline_dispatch",
                   evidence->timeline_dispatch_anchor,
                   "C12_EVENT_HIDE_DAMAGE_RECEIVED",
                   "TIMELINE.C F0261:1933-1934 case C12 dispatch");
    check_contains("evidence.status_box_overdraw",
                   evidence->status_box_overdraw_anchor,
                   "MASK0x1000_STATUS_BOX",
                   "CHAMDRAW.C F0292:771,792-815 status-box overdraw");
    check_contains("evidence.event_constant", evidence->defs_event_anchor,
                   "C12_EVENT_HIDE_DAMAGE_RECEIVED",
                   "DEFS.H:942 C12_EVENT_HIDE_DAMAGE_RECEIVED = 12");
    check_contains("evidence.hide_field", evidence->defs_hide_anchor,
                   "HideDamageReceivedEventIndex",
                   "DEFS.H:640,677 HideDamageReceivedEventIndex");
    check_contains("evidence.zone_anchors", evidence->defs_zone_anchor,
                   "C167_ZONE_FIRST_DAMAGE_TO_CHAMPION_SMALL",
                   "DEFS.H:3792 damage zones");
    check_contains("evidence.graphic_anchors", evidence->defs_graphic_anchor,
                   "C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL",
                   "DEFS.H:2176 damage graphics");
    check_contains("evidence.reschedule", evidence->reschedule_anchor,
                   "F0235_TIMELINE_GetIndex",
                   "CHAMPION.C F0320:1790-1792 reschedule");
    check_contains("evidence.kill_status_box", evidence->kill_status_box_anchor,
                   "MASK0x1000_STATUS_BOX",
                   "CHAMPION.C:1574 F0319_Kill status-box bit");
    check_contains("evidence.no_real_asset", evidence->no_real_asset_claim,
                   "no real-asset",
                   "contract-only no original DOS parity claim");
}

static void test_constants(void)
{
    check_int("const.champion_count", DM1_V1_CPDFD_CHAMPION_COUNT_PC34, 4,
              "CHAMPION.C F0320:1720-1721 four champion panel cells");
    check_int("const.timeline_none", DM1_V1_CPDFD_TIMELINE_NONE_PC34, -1,
              "CEDTINCI.C:66 HideDamageReceivedEventIndex = -1");
    check_int("const.hide_delay", DM1_V1_CPDFD_HIDE_DELAY_TICKS_PC34, 5,
              "CHAMPION.C F0320:1758 GameTime + 5");
    check_int("const.gfx_small", DM1_V1_CPDFD_GFX_DAMAGE_SMALL_PC34, 15,
              "DEFS.H:2176 C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL");
    check_int("const.gfx_big", DM1_V1_CPDFD_GFX_DAMAGE_BIG_PC34, 16,
              "DEFS.H:2177 C016_GRAPHIC_DAMAGE_TO_CHAMPION_BIG");
    check_int("const.zone_small_first",
              DM1_V1_CPDFD_ZONE_DAMAGE_SMALL_FIRST_PC34, 167,
              "DEFS.H:3792 C167_ZONE_FIRST_DAMAGE_TO_CHAMPION_SMALL");
    check_int("const.zone_big_first",
              DM1_V1_CPDFD_ZONE_DAMAGE_BIG_FIRST_PC34, 179,
              "DEFS.H:3794 C179_ZONE_FIRST_DAMAGE_TO_CHAMPION_BIG");
    check_int("const.event_hide_damage",
              DM1_V1_CPDFD_EVENT_HIDE_DAMAGE_RECEIVED_PC34, 12,
              "DEFS.H:942 C12_EVENT_HIDE_DAMAGE_RECEIVED = 12");
}

static void test_init_state_defaults(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    int ci;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    check_int("init.party_count", state.party_champion_count, 4,
              "CHAMPION.C F0320:1720-1721 four champion panel cells");
    check_int("init.inventory_ordinal", state.inventory_champion_ordinal, 1,
              "PANEL.C F0355:2299-2316 G0423 inventory ordinal");
    check_int("init.game_time", (int)state.game_time, 0,
              "G0313_ul_GameTime initial 0");
    check_int("init.pending_count", state.pending_timeline_event_count, 0,
              "G0370_ps_Events empty at init");
    for (ci = 0; ci < DM1_V1_CPDFD_CHAMPION_COUNT_PC34; ++ci) {
        check_int("init.health", state.champions[ci].current_health, 100,
                  "M516_CHAMPIONS[ci].CurrentHealth initial");
        check_true("init.alive", state.champions[ci].alive,
                   "M516_CHAMPIONS[ci].alive initial");
        check_int("init.damage_visible", state.champions[ci].damage_visible, 0,
                  "no flash on init");
        check_int("init.hide_index",
                  state.champions[ci].hide_damage_received_event_index,
                  DM1_V1_CPDFD_TIMELINE_NONE_PC34,
                  "CEDTINCI.C:66 HideDamageReceivedEventIndex = -1");
        if (ci + 1 == state.inventory_champion_ordinal) {
            check_true("init.is_inventory",
                       state.champions[ci].is_inventory_champion,
                       "M000_INDEX_TO_ORDINAL == G0423 inventory");
            check_int("init.gfx", state.champions[ci].damage_graphic_index,
                      DM1_V1_CPDFD_GFX_DAMAGE_BIG_PC34,
                      "CHAMDRAW.C F0623:688 inventory big graphic");
            check_int("init.zone", state.champions[ci].damage_zone_index,
                      DM1_V1_CPDFD_ZONE_DAMAGE_BIG_FIRST_PC34 + ci,
                      "CHAMDRAW.C F0623:696 C179 + championIndex");
        } else {
            check_false("init.is_inventory",
                        state.champions[ci].is_inventory_champion,
                        "non-inventory");
            check_int("init.gfx", state.champions[ci].damage_graphic_index,
                      DM1_V1_CPDFD_GFX_DAMAGE_SMALL_PC34,
                      "CHAMDRAW.C F0623:691 small graphic");
            check_int("init.zone", state.champions[ci].damage_zone_index,
                      DM1_V1_CPDFD_ZONE_DAMAGE_SMALL_FIRST_PC34 + ci,
                      "CHAMDRAW.C F0623:696 C167 + championIndex");
        }
    }
}

static void test_tick_zero_pending_is_noop(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 100;
    check_int("noop.build_return",
              DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
                  &state, 0, 0, &step),
              1, "TickPc34Compat with zero pending damage");
    check_false("noop.applied", step.applied_pending_damage,
                "CHAMPION.C F0320:1724 G0409 == 0 continue");
    check_false("noop.scheduled_new", step.scheduled_new_event,
                "no event scheduled on zero damage");
    check_int("noop.damage_visible", step.damage_visible_after, 0,
              "no flash for zero pending");
    check_int("noop.game_time", (int)state.game_time, 100,
              "G0313_ul_GameTime not bumped by noop");
    check_int("noop.event_count", state.pending_timeline_event_count, 0,
              "G0370_ps_Events still empty after noop");
}

static void test_tick_basic_flash_and_schedule(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 50;
    /* champion 0 is the inventory champion by default ordinal=1. */
    check_int("flash.build_return",
              DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
                  &state, 0, 17, &step),
              1, "TickPc34Compat basic flash for inventory champion");
    check_true("flash.applied", step.applied_pending_damage,
               "CHAMPION.C F0320:1728-1737 damage applied");
    check_true("flash.scheduled_new", step.scheduled_new_event,
               "CHAMPION.C F0320:1780-1784 F0238 add C12 event");
    check_false("flash.rescheduled", step.rescheduled_existing_event,
                "first hit is a NEW event, not a reschedule");
    check_int("flash.damage_visible", step.damage_visible_after, 17,
              "CHAMDRAW.C F0623 C15 damage text");
    check_int("flash.health", state.champions[0].current_health, 83,
              "100 - 17 = 83 CurrentHealth");
    check_true("flash.still_alive", state.champions[0].alive,
               "non-lethal pending damage");
    check_int("flash.pending_count", state.pending_timeline_event_count, 1,
              "G0370_ps_Events has 1 C12 event");
    check_int("flash.hide_index",
              state.champions[0].hide_damage_received_event_index,
              1, "F0238 returns event index 1 (synthetic)");
}

static void test_tick_non_inventory_uses_small_graphic(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 200;
    /* champion 1 is non-inventory (ordinal=1 means champion 0 is inventory) */
    check_int("small.build_return",
              DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
                  &state, 1, 5, &step),
              1, "TickPc34Compat small damage for non-inventory champion");
    check_int("small.gfx", state.champions[1].damage_graphic_index,
              DM1_V1_CPDFD_GFX_DAMAGE_SMALL_PC34,
              "CHAMDRAW.C F0623:691 C015 small graphic");
    check_int("small.zone", state.champions[1].damage_zone_index,
              DM1_V1_CPDFD_ZONE_DAMAGE_SMALL_FIRST_PC34 + 1,
              "CHAMDRAW.C F0623:696 C167 + championIndex");
    check_int("small.damage_visible", state.champions[1].damage_visible, 5,
              "F0623 C15 damage text");
    check_int("small.health", state.champions[1].current_health, 95,
              "100 - 5 = 95 CurrentHealth");
}

static void test_reschedule_second_hit_within_window(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step1, step2;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 10;
    DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
        &state, 2, 3, &step1);
    check_true("resched.first_new", step1.scheduled_new_event,
               "first hit adds new event");
    /* advance one tick, then take a second hit before the C12 fires */
    state.game_time = 11;
    DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
        &state, 2, 7, &step2);
    check_false("resched.second_new", step2.scheduled_new_event,
                "second hit before C12 does NOT add a new event");
    check_true("resched.second_rescheduled", step2.rescheduled_existing_event,
               "CHAMPION.C F0320:1790-1792 F0235+F0236 reschedule");
    check_int("resched.pending_count", state.pending_timeline_event_count, 1,
              "G0370_ps_Events still has 1 C12 event after reschedule");
    check_int("resched.damage_visible", step2.damage_visible_after, 7,
              "latest damage text overwrites");
}

static void test_advance_timeline_fires_c12_at_t_plus_5(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step;
    DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat run;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 1000;
    /* Champion 0 is the inventory champion */
    DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
        &state, 0, 25, &step);

    /* advance 1..4 ticks: damage is still visible */
    for (int t = 1; t <= 4; ++t) {
        DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat(
            &state, 1, &run);
        check_int("t_visible.before_hide",
                  state.champions[0].damage_visible, 25,
                  "CHAMPION.C F0320:1724 G0409 was reset to 0; "
                  "graphic remains on screen until C12 fires");
    }

    /* tick T+5: C12 fires for inventory champion → F0354 portrait overdraw */
    DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat(
        &state, 1, &run);
    check_int("t5.c12_fired", run.total_c12_hide_fired, 1,
              "TIMELINE.C F0261:1933 case C12 dispatched once");
    check_int("t5.inventory_overdraw",
              run.total_inventory_portrait_overdraws, 1,
              "TIMELINE.C F0254:1626-1630 F0354_INVENTORY_"
              "DrawStatusBoxPortrait over C179..C185");
    check_int("t5.damage_visible", state.champions[0].damage_visible, 0,
              "damage graphic erased by inventory portrait");
    check_int("t5.hide_index",
              state.champions[0].hide_damage_received_event_index,
              DM1_V1_CPDFD_TIMELINE_NONE_PC34,
              "F0254:1624 HideDamageReceivedEventIndex reset to -1");
    check_int("t5.pending_count", state.pending_timeline_event_count, 0,
              "C12 event removed from G0370_ps_Events");
}

static void test_advance_timeline_non_inventory_branch(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step;
    DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat run;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 2000;
    /* Champion 1 is non-inventory */
    DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
        &state, 1, 9, &step);
    /* advance 5 ticks to fire the C12 */
    DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat(
        &state, 5, &run);
    check_int("noninv.c12_fired", run.total_c12_hide_fired, 1,
              "TIMELINE.C F0261:1933 case C12 dispatched");
    check_int("noninv.name_overdraw",
              run.total_non_inventory_name_overdraws, 1,
              "TIMELINE.C F0254:1632-1635 MASK0x0080_NAME_TITLE + F0292");
    check_int("noninv.inventory_overdraw",
              run.total_inventory_portrait_overdraws, 0,
              "non-inventory champion does NOT take the F0354 branch");
    check_int("noninv.damage_visible", state.champions[1].damage_visible, 0,
              "damage erased by F0292 name-title redraw over C167..C173");
}

static void test_advance_timeline_multiple_champions(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step0, step2;
    DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat run;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 5000;
    /* Hit champion 0 (inventory, big) AND champion 2 (non-inventory, small) */
    DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
        &state, 0, 11, &step0);
    state.game_time = 5000;
    DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
        &state, 2, 6, &step2);
    check_int("multi.pending_count", state.pending_timeline_event_count, 2,
              "two C12 events scheduled for two champions");
    DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat(
        &state, 5, &run);
    check_int("multi.c12_fired", run.total_c12_hide_fired, 2,
              "both C12 events fire at T+5");
    check_int("multi.inventory_overdraw",
              run.total_inventory_portrait_overdraws, 1,
              "F0354 once for champion 0");
    check_int("multi.name_overdraw",
              run.total_non_inventory_name_overdraws, 1,
              "F0292 once for champion 2");
    check_int("multi.damage_visible[0]", state.champions[0].damage_visible, 0,
              "champion 0 damage erased");
    check_int("multi.damage_visible[2]", state.champions[2].damage_visible, 0,
              "champion 2 damage erased");
    check_int("multi.damage_visible[1]", state.champions[1].damage_visible, 0,
              "champion 1 was never hit; stays at 0");
}

static void test_advance_timeline_dead_champion_early_return(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step;
    DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat run;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 0;
    /* Pre-kill champion 1 (non-inventory) */
    state.champions[1].current_health = 0;
    state.champions[1].alive = false;
    /* Manually stage a C12 event for the dead champion */
    state.champions[1].damage_visible = 9;
    state.champions[1].hide_damage_received_event_index = 99;
    state.timeline_events[0].event_type =
        DM1_V1_CPDFD_EVENT_HIDE_DAMAGE_RECEIVED_PC34;
    state.timeline_events[0].map_index = 0;
    state.timeline_events[0].fire_time = state.game_time;
    state.timeline_events[0].scheduled_time = state.game_time;
    state.timeline_events[0].champion_priority = 1;
    state.timeline_events[0].slot = 0;
    state.pending_timeline_event_count = 1;

    DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat(
        &state, 1, &run);
    check_int("dead.c12_fired", run.total_c12_hide_fired, 1,
              "C12 dispatched but no draw for dead champion");
    check_int("dead.early_return",
              run.total_dead_champion_early_returns, 1,
              "TIMELINE.C F0254:1624 dead-champion early return");
    check_int("dead.name_overdraw",
              run.total_non_inventory_name_overdraws, 0,
              "no F0292 overdraw for dead champion");
    check_int("dead.inventory_overdraw",
              run.total_inventory_portrait_overdraws, 0,
              "no F0354 overdraw for dead champion");
    check_int("dead.hide_index",
              state.champions[1].hide_damage_received_event_index,
              DM1_V1_CPDFD_TIMELINE_NONE_PC34,
              "F0254:1624 HideDamageReceivedEventIndex reset to -1");
    check_int("dead.pending_count", state.pending_timeline_event_count, 0,
              "C12 event removed even on early return");
    (void)step;
}

static void test_status_box_overdraw_erases_damage(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step;
    DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat run;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 100;
    /* Hit champion 0 (inventory) and champion 1 (non-inventory) */
    DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
        &state, 0, 12, &step);
    state.game_time = 100;
    DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
        &state, 1, 4, &step);
    check_int("sb.damage_visible[0]", state.champions[0].damage_visible, 12,
              "champion 0 damage visible");
    check_int("sb.damage_visible[1]", state.champions[1].damage_visible, 4,
              "champion 1 damage visible");
    /* Simulate F0319_CHAMPION_Kill setting MASK0x1000_STATUS_BOX for
     * champion 0 only. CHAMDRAW.C F0292 receives P0615_ui_ChampionIndex,
     * so the overdraw is slot-scoped, not a global panel erase. */
    check_int("sb.mark_return",
              DM1_V1_ChampionPanelDamageFlashDecay_OverdrawStatusBoxPc34Compat(
                  &state, 0),
              1, "CHAMPION.C:1574 sets MASK0x1000_STATUS_BOX on champion 0");
    /* advance one tick — the dirty bit should trigger the C151..C182
     * status-box overdraw that erases champion 0's damage graphic only */
    DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat(
        &state, 1, &run);
    check_int("sb.total_overdraws", run.total_status_box_overdraws, 1,
              "CHAMDRAW.C F0292:771,792-815 status-box overdraw is scoped "
              "to P0615_ui_ChampionIndex");
    check_int("sb.damage_visible[0]", state.champions[0].damage_visible, 0,
              "champion 0 damage erased by status-box overdraw");
    check_int("sb.damage_visible[1]", state.champions[1].damage_visible, 4,
              "champion 1 damage remains visible until its own C12/status draw");
    /* C12 events should still be pending (the status-box overdraw does
     * not cancel them) */
    check_int("sb.c12_pending", state.pending_timeline_event_count, 2,
              "F0292 status-box overdraw does NOT consume C12 events; "
              "they will still fire at GameTime+5");

    check_int("sb.invalid_neg_return",
              DM1_V1_ChampionPanelDamageFlashDecay_OverdrawStatusBoxPc34Compat(
                  &state, -1),
              0, "synthetic guard on negative status-box champion index");
    check_int("sb.invalid_high_return",
              DM1_V1_ChampionPanelDamageFlashDecay_OverdrawStatusBoxPc34Compat(
                  &state, DM1_V1_CPDFD_CHAMPION_COUNT_PC34),
              0, "synthetic guard on high status-box champion index");
}

static void test_advance_negative_ticks_is_rejected(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat run;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    check_int("neg.build_return",
              DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat(
                  &state, 0, &run),
              0, "synthetic guard before TIMELINE.C F0261 model");
    check_int("negneg.build_return",
              DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat(
                  &state, -3, &run),
              0, "synthetic guard against negative ticks");
    check_int("neg.game_time", (int)state.game_time, 0,
              "G0313_ul_GameTime not bumped on rejected call");
}

static void test_out_of_range_champion_index_rejected(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    check_int("oor.neg_return",
              DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
                  &state, -1, 1, &step),
              0, "synthetic guard on negative champion index");
    check_int("oor.high_return",
              DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
                  &state, DM1_V1_CPDFD_CHAMPION_COUNT_PC34, 1, &step),
              0, "synthetic guard on high champion index");
}

static void test_lethal_damage_marks_champion_dead(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 0;
    /* 150 > 100, so the damage is lethal and the champion is killed. */
    DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
        &state, 2, 150, &step);
    check_true("lethal.applied", step.applied_pending_damage,
               "CHAMPION.C F0320:1728-1737 lethal pending damage is applied");
    check_int("lethal.health", state.champions[2].current_health, 0,
              "CurrentHealth pinned to 0 by F0319_CHAMPION_Kill path");
    check_false("lethal.alive", state.champions[2].alive,
                "M516_CHAMPIONS[ci].alive = false after kill");
    /* F0320's F0319 kill branch is before the F0623/C12 block. */
    check_int("lethal.damage_visible",
              state.champions[2].damage_visible, 0,
              "CHAMPION.C F0320:1729-1737 kill branch skips F0623 flash");
    check_false("lethal.scheduled_new", step.scheduled_new_event,
                "CHAMPION.C F0320:1729-1737 kill branch skips C12 add");
    check_false("lethal.rescheduled", step.rescheduled_existing_event,
                "CHAMPION.C F0320:1729-1737 kill branch skips C12 fix");
    check_int("lethal.pending_count", state.pending_timeline_event_count, 0,
              "CHAMPION.C F0320:1736-1794 schedules only in nonlethal else");
    check_true("lethal.status_box_dirty",
               state.mask0x1000_status_box_dirty,
               "CHAMPION.C:1574 F0319 marks MASK0x1000_STATUS_BOX");
}

static void test_build_result_reflects_state(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat result;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 77;
    DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
        &state, 1, 8, &step);
    check_int("build.build_return",
              DM1_V1_ChampionPanelDamageFlashDecay_BuildResultPc34Compat(
                  &state, 1, &result),
              1, "BuildResult with valid champion");
    check_int("build.damage_visible", result.damage_visible_after, 8,
              "F0320 recorded damage visible flag");
    check_int("build.game_time", (int)result.next_game_time, 77,
              "G0313_ul_GameTime not advanced by TickPc34Compat itself");
    check_true("build.applied", result.applied_pending_damage,
               "BuildResult sees damage_just_applied flag");

    check_int("build.neg_return",
              DM1_V1_ChampionPanelDamageFlashDecay_BuildResultPc34Compat(
                  &state, -1, &result),
              0, "BuildResult rejects negative champion index");
    check_int("build.high_return",
              DM1_V1_ChampionPanelDamageFlashDecay_BuildResultPc34Compat(
                  &state, 4, &result),
              0, "BuildResult rejects high champion index");
}

static void test_flush_remaining(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step;
    DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat run;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 1234;
    DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
        &state, 0, 1, &step);
    check_int("flush.pending_before", state.pending_timeline_event_count, 1,
              "one C12 event scheduled");
    DM1_V1_ChampionPanelDamageFlashDecay_FlushRemainingPc34Compat(
        &state, &run);
    check_int("flush.c12_fired", run.total_c12_hide_fired, 1,
              "C12 event fired by FlushRemaining");
    check_int("flush.inventory_overdraw",
              run.total_inventory_portrait_overdraws, 1,
              "inventory-champion branch fires");
    check_int("flush.pending_after", state.pending_timeline_event_count, 0,
              "G0370_ps_Events empty after flush");
    check_int("flush.damage_visible", state.champions[0].damage_visible, 0,
              "damage graphic erased by flush");
}

static void test_advance_inventory_open_uses_portrait(void)
{
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat state;
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat step;
    DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat run;

    DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(&state);
    state.game_time = 0;
    state.inventory_champion_ordinal = 3; /* champion index 2 is inventory */
    state.champions[2].is_inventory_champion = true;
    state.champions[2].damage_graphic_index = DM1_V1_CPDFD_GFX_DAMAGE_BIG_PC34;
    state.champions[2].damage_zone_index =
        DM1_V1_CPDFD_ZONE_DAMAGE_BIG_FIRST_PC34 + 2;

    DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
        &state, 2, 14, &step);
    check_int("oi.gfx", state.champions[2].damage_graphic_index,
              DM1_V1_CPDFD_GFX_DAMAGE_BIG_PC34,
              "CHAMDRAW.C F0623:688 inventory big graphic when ordinal 3");
    DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat(
        &state, 5, &run);
    check_int("oi.inventory_overdraw",
              run.total_inventory_portrait_overdraws, 1,
              "F0354_INVENTORY_DrawStatusBoxPortrait fires for the "
              "new inventory champion (ordinal 3)");
    check_int("oi.name_overdraw",
              run.total_non_inventory_name_overdraws, 0,
              "non-inventory branch does NOT fire for the inventory "
              "champion under the new ordinal");
}

int main(void)
{
    test_evidence();
    test_constants();
    test_init_state_defaults();
    test_tick_zero_pending_is_noop();
    test_tick_basic_flash_and_schedule();
    test_tick_non_inventory_uses_small_graphic();
    test_reschedule_second_hit_within_window();
    test_advance_timeline_fires_c12_at_t_plus_5();
    test_advance_timeline_non_inventory_branch();
    test_advance_timeline_multiple_champions();
    test_advance_timeline_dead_champion_early_return();
    test_status_box_overdraw_erases_damage();
    test_advance_negative_ticks_is_rejected();
    test_out_of_range_champion_index_rejected();
    test_lethal_damage_marks_champion_dead();
    test_build_result_reflects_state();
    test_flush_remaining();
    test_advance_inventory_open_uses_portrait();

    printf("dm1_v1_champion_panel_damage_flash_decay_pc34_compat: "
           "assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return g_failures ? 1 : 0;
}
