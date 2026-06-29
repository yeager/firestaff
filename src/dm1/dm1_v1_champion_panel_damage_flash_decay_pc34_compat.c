#include "dm1/dm1_v1_champion_panel_damage_flash_decay_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Synthetic source-locked gate for the DM1 V1 champion-panel damage
 * flash decay. See the header for the full ReDMCSB anchor list.
 *
 * Behaviour pinned by this file (all source-locked to ReDMCSB
 * CHAMPION.C F0320:1758-1792 + TIMELINE.C F0254:1614-1637 +
 * TIMELINE.C F0261:1933-1934 + CHAMDRAW.C F0292:771,792-815 +
 * DEFS.H C12/C167/C179 + CEDTINCI.C:66 + REVIVE.C:175):
 *
 *  1. DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat simulates one
 *     F0320 pass for the named champion:
 *       - Reads pending damage from the input.
 *       - If 0, returns without applying or scheduling anything.
 *       - If the champion is dead (current_health <= 0), no draw or
 *         schedule occurs (F0320:1725-1727 early-continues on
 *         CurrentHealth == 0).
 *       - Subtracts the damage from current_health; if the result
 *         would drop to <= 0, F0320 takes the F0319_Kill branch before
 *         the F0623/C12 block, so no damage flash or hide event is
 *         created and the status-box redraw path owns the visible erase.
 *       - Picks the C015/C016 graphic + C167/C179 zone by the
 *         inventory-champion comparison (M000_INDEX_TO_ORDINAL ==
 *         G0423_i_InventoryChampionOrdinal) and records the chosen
 *         zone in the state for the overdraw side.
 *       - Schedules the C12_EVENT_HIDE_DAMAGE_RECEIVED event at
 *         GameTime + 5 via the synthetic timeline backing store.
 *         If a previous C12 is already pending for this champion
 *         (hide_damage_received_event_index != -1), the existing
 *         event is rescheduled (F0235 + F0236) — F0320:1791.
 *       - Records the damage_visible flag on the champion and the
 *         step result.
 *
 *  2. DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat
 *     advances synthetic GameTime by N ticks, drains the timeline
 *     backing store of all C12 events whose fire_time <= new
 *     GameTime, and per drained event runs the F0254 simulation:
 *       - Champion dead → early return, no draw (F0254:1624).
 *       - Inventory champion → inventory portrait overdraw (F0354
 *         over C179..C185 damage zone) (F0254:1626-1630).
 *       - Non-inventory champion → MASK0x0080_NAME_TITLE + F0292
 *         (F0254:1632-1635) redraws the name over C167..C173.
 *       - Resets HideDamageReceivedEventIndex to -1.
 *       - Sets damage_visible = 0.
 *       - Records the overdraw into the run result + per-tick trace.
 *
 *  3. DM1_V1_ChampionPanelDamageFlashDecay_OverdrawStatusBoxPc34Compat
 *     simulates the CHAMPION.C:1574 / F0292 MASK0x1000_STATUS_BOX
 *     side-effect: when a champion has its MASK0x1000_STATUS_BOX bit set,
 *     the next F0292(P0615_ui_ChampionIndex) redraws that champion's
 *     C151..C182 status box zone (F0292:792-815) and erases only that
 *     champion's C167..C173 / C179..C185 damage graphic without waiting
 *     for the C12 event.
 *
 *  4. DM1_V1_ChampionPanelDamageFlashDecay_FlushRemainingPc34Compat
 *     drains any remaining C12 events at GameTime + 5 (used for the
 *     final-tick assertion).
 *
 *  5. The C12 hide delay is exactly 5 ticks (F0320:1758 sets
 *     Map_Time = GameTime + 5; F0254 then fires at fire_time <=
 *     current GameTime).
 */

static const DM1_V1_ChampionPanelDamageFlashDecayEvidencePc34Compat
    s_cpdfd_evidence = {
        true,
        "CHAMPION.C F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds:"
        "1758-1792 post-F0623 schedule of C12_EVENT_HIDE_DAMAGE_RECEIVED",
        "TIMELINE.C F0254_TIMELINE_ProcessEvent12_HideDamageReceived:"
        "1614-1637 inventory-portrait vs MASK0x0080_NAME_TITLE+F0292 branches",
        "TIMELINE.C F0261_TIMELINE_Process_CPSEF:1933-1934 case "
        "C12_EVENT_HIDE_DAMAGE_RECEIVED dispatch",
        "CHAMDRAW.C F0292_CHAMPION_DrawState:771,792-815 MASK0x1000_STATUS_BOX "
        "status-box zone overdraw of C151..C182",
        "DEFS.H:942 C12_EVENT_HIDE_DAMAGE_RECEIVED = 12",
        "DEFS.H:640,677 int16_t HideDamageReceivedEventIndex (per-champion)",
        "DEFS.H:3792-3794 C167_ZONE_FIRST_DAMAGE_TO_CHAMPION_SMALL, "
        "C179_ZONE_FIRST_DAMAGE_TO_CHAMPION_BIG",
        "DEFS.H:2176-2177 C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL, "
        "C016_GRAPHIC_DAMAGE_TO_CHAMPION_BIG",
        "CHAMPION.C F0320:1790-1792 F0235_TIMELINE_GetIndex + "
        "F0236_TIMELINE_FixPlacement reschedule branch",
        "CHAMPION.C:1574 M008_SET MASK0x1000_STATUS_BOX on F0319_CHAMPION_Kill "
        "status-box overdraw",
        "contract-only; no real-asset or original-DOS pixel parity claim"
    };

const DM1_V1_ChampionPanelDamageFlashDecayEvidencePc34Compat *
DM1_V1_ChampionPanelDamageFlashDecay_EvidencePc34Compat(void)
{
    return &s_cpdfd_evidence;
}

void DM1_V1_ChampionPanelDamageFlashDecay_InitStatePc34Compat(
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state)
{
    int champion_index;

    if (!state) {
        return;
    }

    memset(state, 0, sizeof(*state));
    state->party_champion_count = DM1_V1_CPDFD_CHAMPION_COUNT_PC34;
    state->inventory_champion_ordinal = 1;
    state->inventory_open = true;
    state->mask0x1000_status_box_dirty = false;
    state->next_timeline_event_index = 1;
    state->pending_timeline_event_count = 0;
    state->game_time = 0;
    for (champion_index = 0;
         champion_index < DM1_V1_CPDFD_CHAMPION_COUNT_PC34;
         ++champion_index) {
        state->champions[champion_index].index = champion_index;
        state->champions[champion_index].current_health = 100;
        state->champions[champion_index].alive = true;
        state->champions[champion_index].is_inventory_champion =
            (champion_index + 1 == state->inventory_champion_ordinal);
        state->champions[champion_index].damage_just_applied = 0;
        state->champions[champion_index].damage_visible = 0;
        state->champions[champion_index].hide_damage_received_event_index =
            DM1_V1_CPDFD_TIMELINE_NONE_PC34;
        if (state->champions[champion_index].is_inventory_champion) {
            state->champions[champion_index].damage_graphic_index =
                DM1_V1_CPDFD_GFX_DAMAGE_BIG_PC34;
            state->champions[champion_index].damage_zone_index =
                DM1_V1_CPDFD_ZONE_DAMAGE_BIG_FIRST_PC34 + champion_index;
        } else {
            state->champions[champion_index].damage_graphic_index =
                DM1_V1_CPDFD_GFX_DAMAGE_SMALL_PC34;
            state->champions[champion_index].damage_zone_index =
                DM1_V1_CPDFD_ZONE_DAMAGE_SMALL_FIRST_PC34 + champion_index;
        }
    }
}

static int cpdfd_allocate_event_index(
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state)
{
    int allocated = state->next_timeline_event_index;
    state->next_timeline_event_index++;
    return allocated;
}

static int cpdfd_add_pending_event(
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state,
    int event_index,
    int event_type,
    int map_index,
    int64_t fire_time,
    int champion_priority)
{
    if (state->pending_timeline_event_count >=
        (int)(sizeof(state->timeline_events) /
              sizeof(state->timeline_events[0]))) {
        return 0;
    }
    state->timeline_events[state->pending_timeline_event_count].event_index =
        event_index;
    state->timeline_events[state->pending_timeline_event_count].event_type =
        event_type;
    state->timeline_events[state->pending_timeline_event_count].map_index =
        map_index;
    state->timeline_events[state->pending_timeline_event_count].fire_time =
        fire_time;
    state->timeline_events[state->pending_timeline_event_count].scheduled_time =
        fire_time;
    state->timeline_events[state->pending_timeline_event_count].champion_priority =
        champion_priority;
    state->timeline_events[state->pending_timeline_event_count].slot = 0;
    state->pending_timeline_event_count++;
    return 1;
}

int DM1_V1_ChampionPanelDamageFlashDecay_TickPc34Compat(
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state,
    int champion_index_with_pending_damage,
    int pending_damage,
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat *out_step)
{
    DM1_V1_ChampionPanelDamageFlashDecayChampionPc34Compat *champion;
    int scheduled_event_index;
    int64_t scheduled_fire_time;
    int rescheduled;
    int scheduled_new;

    if (!state || !out_step) {
        return 0;
    }

    memset(out_step, 0, sizeof(*out_step));
    out_step->champion_index = champion_index_with_pending_damage;

    if (champion_index_with_pending_damage < 0 ||
        champion_index_with_pending_damage >=
            DM1_V1_CPDFD_CHAMPION_COUNT_PC34) {
        return 0;
    }

    champion = &state->champions[champion_index_with_pending_damage];

    if (pending_damage == 0) {
        /*
         * CHAMPION.C F0320:1724-1725:
         *   if (!(L0968_ui_PendingDamage = G0409[...])) continue;
         */
        out_step->damage_visible_after = champion->damage_visible;
        out_step->next_game_time = state->game_time;
        return 1;
    }

    if (!champion->alive || champion->current_health <= 0) {
        /*
         * CHAMPION.C F0320:1726-1727:
         *   if (!(AL0969_i_Health = L0971_ps_Champion->CurrentHealth)) continue;
         */
        out_step->damage_visible_after = 0;
        out_step->next_game_time = state->game_time;
        out_step->dead_champion_early_return = true;
        return 1;
    }

    champion->damage_just_applied = pending_damage;
    champion->current_health -= pending_damage;
    if (champion->current_health <= 0) {
        /*
         * ReDMCSB: CHAMPION.C F0320 lines 1728-1737 enter the
         * F0319_CHAMPION_Kill branch, and the F0623/C12 schedule block is
         * only in the nonlethal else branch at lines 1736-1794.
         */
        champion->current_health = 0;
        champion->alive = false;
        champion->damage_visible = 0;
        state->mask0x1000_status_box_dirty = true;
        out_step->applied_pending_damage = true;
        out_step->scheduled_new_event = false;
        out_step->rescheduled_existing_event = false;
        out_step->damage_visible_after = champion->damage_visible;
        out_step->next_game_time = state->game_time;
        return 1;
    }

    /* F0623_DrawDamageToChampion_F0320_sub(ChampionIndex, PendingDamage) */
    champion->damage_visible = pending_damage;

    /* C12 schedule branch (CHAMPION.C F0320:1758-1792) */
    scheduled_fire_time =
        state->game_time + DM1_V1_CPDFD_HIDE_DELAY_TICKS_PC34;
    rescheduled = 0;
    if (champion->hide_damage_received_event_index ==
        DM1_V1_CPDFD_TIMELINE_NONE_PC34) {
        /* New event branch: F0238_TIMELINE_AddEvent_GetEventIndex_CPSE */
        scheduled_event_index = cpdfd_allocate_event_index(state);
        cpdfd_add_pending_event(state,
                                scheduled_event_index,
                                DM1_V1_CPDFD_EVENT_HIDE_DAMAGE_RECEIVED_PC34,
                                state->party_champion_count /* map sentinel */,
                                scheduled_fire_time,
                                champion_index_with_pending_damage);
        champion->hide_damage_received_event_index = scheduled_event_index;
        scheduled_new = 1;
    } else {
        /*
         * Reschedule branch: F0235_TIMELINE_GetIndex +
         * F0236_TIMELINE_FixPlacement; in the synthetic model this
         * rewrites the fire_time of the exact stored event index, not
         * merely the first C12 row with the same champion priority.
         */
        int i;
        for (i = 0; i < state->pending_timeline_event_count; ++i) {
            if (state->timeline_events[i].event_index ==
                    champion->hide_damage_received_event_index &&
                state->timeline_events[i].event_type ==
                    DM1_V1_CPDFD_EVENT_HIDE_DAMAGE_RECEIVED_PC34) {
                /*
                 * ReDMCSB CHAMPION.C F0320:1791-1793 rewrites
                 * G0370_ps_Events[AL0969_i_EventIndex].Map_Time and
                 * calls F0236_TIMELINE_FixPlacement(F0235_TIMELINE_GetIndex(
                 * AL0969_i_EventIndex)); TIMELINE.C F0235:273-292 is the
                 * event-index lookup in the live timeline.
                 */
                state->timeline_events[i].fire_time = scheduled_fire_time;
                break;
            }
        }
        scheduled_event_index = champion->hide_damage_received_event_index;
        rescheduled = 1;
        scheduled_new = 0;
    }

    out_step->applied_pending_damage = true;
    out_step->scheduled_new_event = (scheduled_new == 1);
    out_step->rescheduled_existing_event = (rescheduled == 1);
    out_step->damage_visible_after = champion->damage_visible;
    out_step->next_game_time = state->game_time;
    return 1;
}

int DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat(
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state,
    int ticks,
    DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat *out_run)
{
    int tick_index;
    int drained_event;
    int events_drained_this_tick;

    if (!state || !out_run || ticks <= 0) {
        return 0;
    }

    memset(out_run, 0, sizeof(*out_run));
    out_run->final_game_time = state->game_time;

    for (tick_index = 0; tick_index < ticks; ++tick_index) {
        state->game_time++;
        events_drained_this_tick = 0;
        do {
            drained_event = -1;
            for (int i = 0; i < state->pending_timeline_event_count; ++i) {
                if (state->timeline_events[i].fire_time <=
                    state->game_time) {
                    drained_event = i;
                    break;
                }
            }
            if (drained_event >= 0) {
                DM1_V1_ChampionPanelDamageFlashDecayTimelineEventPc34Compat ev =
                    state->timeline_events[drained_event];
                int j;
                DM1_V1_ChampionPanelDamageFlashDecayChampionPc34Compat *
                    champion;
                int champion_index = ev.champion_priority;

                for (j = drained_event;
                     j < state->pending_timeline_event_count - 1;
                     ++j) {
                    state->timeline_events[j] =
                        state->timeline_events[j + 1];
                }
                state->pending_timeline_event_count--;

                if (out_run->trace_count <
                    (int)(sizeof(out_run->trace) /
                          sizeof(out_run->trace[0]))) {
                    out_run->trace[out_run->trace_count].game_time =
                        state->game_time;
                    out_run->trace[out_run->trace_count].champion_index =
                        champion_index;
                    out_run->trace[out_run->trace_count].kind =
                        DM1_V1_CPDFD_TICK_C12_HIDE_DAMAGE_PC34;
                    out_run->trace[out_run->trace_count].anchor =
                        "TIMELINE.C F0254:1614-1637 C12 hide damage";
                    out_run->trace_count++;
                }
                out_run->total_c12_hide_fired++;
                events_drained_this_tick++;

                if (champion_index < 0 ||
                    champion_index >= DM1_V1_CPDFD_CHAMPION_COUNT_PC34) {
                    continue;
                }
                champion = &state->champions[champion_index];

                /*
                 * F0254:1624 early return when CurrentHealth == 0
                 * (defer to F0319_Kill status-box redraw). The
                 * synthetic gate honours this by skipping the draw
                 * and only clearing the event index.
                 */
                if (!champion->alive || champion->current_health <= 0) {
                    champion->hide_damage_received_event_index =
                        DM1_V1_CPDFD_TIMELINE_NONE_PC34;
                    out_run->total_dead_champion_early_returns++;
                    if (out_run->trace_count <
                        (int)(sizeof(out_run->trace) /
                              sizeof(out_run->trace[0]))) {
                        out_run->trace[out_run->trace_count].game_time =
                            state->game_time;
                        out_run->trace[out_run->trace_count].champion_index =
                            champion_index;
                        out_run->trace[out_run->trace_count].kind =
                            DM1_V1_CPDFD_TICK_C12_HIDE_DAMAGE_PC34;
                        out_run->trace[out_run->trace_count].anchor =
                            "TIMELINE.C F0254:1624 dead-champion early return";
                        out_run->trace_count++;
                    }
                    continue;
                }

                champion->hide_damage_received_event_index =
                    DM1_V1_CPDFD_TIMELINE_NONE_PC34;
                champion->damage_visible = 0;

                if (champion->is_inventory_champion) {
                    /*
                     * F0254:1626-1630 inventory-portrait overdraw
                     * of the C179..C185 damage zone.
                     */
                    out_run->total_inventory_portrait_overdraws++;
                    if (out_run->trace_count <
                        (int)(sizeof(out_run->trace) /
                              sizeof(out_run->trace[0]))) {
                        out_run->trace[out_run->trace_count].game_time =
                            state->game_time;
                        out_run->trace[out_run->trace_count].champion_index =
                            champion_index;
                        out_run->trace[out_run->trace_count].kind =
                            DM1_V1_CPDFD_TICK_C12_HIDE_DAMAGE_PC34;
                        out_run->trace[out_run->trace_count].anchor =
                            "TIMELINE.C F0254:1626-1630 F0354_INVENTORY_"
                            "DrawStatusBoxPortrait over C179..C185";
                        out_run->trace_count++;
                    }
                } else {
                    /*
                     * F0254:1632-1635 MASK0x0080_NAME_TITLE +
                     * F0292_CHAMPION_DrawState for non-inventory
                     * champion (redraws name over C167..C173).
                     */
                    out_run->total_non_inventory_name_overdraws++;
                    if (out_run->trace_count <
                        (int)(sizeof(out_run->trace) /
                              sizeof(out_run->trace[0]))) {
                        out_run->trace[out_run->trace_count].game_time =
                            state->game_time;
                        out_run->trace[out_run->trace_count].champion_index =
                            champion_index;
                        out_run->trace[out_run->trace_count].kind =
                            DM1_V1_CPDFD_TICK_C12_HIDE_DAMAGE_PC34;
                        out_run->trace[out_run->trace_count].anchor =
                            "TIMELINE.C F0254:1632-1635 MASK0x0080_NAME_TITLE"
                            "+F0292 over C167..C173";
                        out_run->trace_count++;
                    }
                }
            }
        } while (drained_event >= 0);
        (void)events_drained_this_tick;

        /*
         * Status-box overdraw side-effect: if a champion has its
         * MASK0x1000_STATUS_BOX bit set, the next
         * F0292_CHAMPION_DrawState(P0615_ui_ChampionIndex) redraws
         * that champion's C151..C182 status-box zone
         * (CHAMDRAW.C:771,792-815) and incidentally erases only that
         * champion's C167..C173 / C179..C185 damage graphic.
         */
        if (state->mask0x1000_status_box_dirty) {
            for (int ci = 0;
                 ci < DM1_V1_CPDFD_CHAMPION_COUNT_PC34;
                 ++ci) {
                if (state->mask0x1000_status_box_dirty_for_champion[ci] &&
                    state->champions[ci].damage_visible > 0) {
                    state->champions[ci].damage_visible = 0;
                    out_run->total_status_box_overdraws++;
                    if (out_run->trace_count <
                        (int)(sizeof(out_run->trace) /
                              sizeof(out_run->trace[0]))) {
                        out_run->trace[out_run->trace_count].game_time =
                            state->game_time;
                        out_run->trace[out_run->trace_count].champion_index =
                            ci;
                        out_run->trace[out_run->trace_count].kind =
                            DM1_V1_CPDFD_TICK_STATUS_BOX_OVERDRAW_PC34;
                        out_run->trace[out_run->trace_count].anchor =
                            "CHAMDRAW.C F0292:771,792-815 MASK0x1000_STATUS_BOX";
                        out_run->trace_count++;
                    }
                }
                state->mask0x1000_status_box_dirty_for_champion[ci] = false;
            }
            state->mask0x1000_status_box_dirty = false;
        }
    }

    out_run->tick_count = ticks;
    out_run->final_game_time = state->game_time;
    for (int ci = 0; ci < DM1_V1_CPDFD_CHAMPION_COUNT_PC34; ++ci) {
        out_run->damage_visible_for_champion[ci] =
            state->champions[ci].damage_visible;
    }
    return 1;
}

int DM1_V1_ChampionPanelDamageFlashDecay_OverdrawStatusBoxPc34Compat(
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state,
    int champion_index)
{
    /*
     * Simulates the F0319_CHAMPION_Kill MASK0x1000_STATUS_BOX set
     * (CHAMPION.C:1574) and any other path that sets the bit on
     * the given champion. The next F0292 draw-state pass for that
     * champion erases that champion's damage graphic.
     */
    if (!state) {
        return 0;
    }
    if (champion_index < 0 ||
        champion_index >= DM1_V1_CPDFD_CHAMPION_COUNT_PC34) {
        return 0;
    }
    state->mask0x1000_status_box_dirty = true;
    state->mask0x1000_status_box_dirty_for_champion[champion_index] = true;
    return 1;
}

int DM1_V1_ChampionPanelDamageFlashDecay_FlushRemainingPc34Compat(
    DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state,
    DM1_V1_ChampionPanelDamageFlashDecayRunResultPc34Compat *out_run)
{
    if (!state || !out_run) {
        return 0;
    }
    /*
     * Move game_time forward by 6 ticks so the C12 fire_time
     * (GameTime+5 at the time of scheduling) is always <= game_time.
     */
    return DM1_V1_ChampionPanelDamageFlashDecay_AdvanceTimelinePc34Compat(
        state, DM1_V1_CPDFD_HIDE_DELAY_TICKS_PC34 + 1, out_run);
}

int DM1_V1_ChampionPanelDamageFlashDecay_BuildResultPc34Compat(
    const DM1_V1_ChampionPanelDamageFlashDecayStatePc34Compat *state,
    int champion_index,
    DM1_V1_ChampionPanelDamageFlashDecayStepResultPc34Compat *out_step)
{
    if (!state || !out_step) {
        return 0;
    }
    if (champion_index < 0 ||
        champion_index >= DM1_V1_CPDFD_CHAMPION_COUNT_PC34) {
        return 0;
    }
    memset(out_step, 0, sizeof(*out_step));
    out_step->champion_index = champion_index;
    out_step->damage_visible_after =
        state->champions[champion_index].damage_visible;
    out_step->next_game_time = state->game_time;
    out_step->applied_pending_damage =
        (state->champions[champion_index].damage_just_applied > 0);
    out_step->scheduled_new_event = false;
    out_step->rescheduled_existing_event = false;
    return 1;
}
