/*
 * nexus_v1_light_overflow.c
 * =========================
 *
 * Nexus V1 light-overflow data model + bug-classification implementation.
 *
 * Source-lock
 * -----------
 * ReDMCSB Toolchains/Common/Source/ — WIP20210206:
 *
 *   DATA.C   : 359  — G0039_ai_Graphic562_LightPowerToLightAmount[16]
 *   LOADSAVE.C: 2041 — G0369_EventMaximumCount = 100 base
 *                     (then LOADSAVE.C:2064 += GROUP/PROJECTILE/etc.)
 *   MENU.C   : 1125-1143 — F0404_MENUS_CreateEvent70_Light
 *   MENU.C   : 1926-1942 — Light / Torch / Darkness dispatch
 *   TIMELINE.C: 487-555 — F0238_AddEvent_GetEventIndex_CPSE (BUG0_18
 *                        silent drop when G0372_ui_EventCount ==
 *                        G0369_EventMaximumCount)
 *   TIMELINE.C: 1720-1767 — F0257_ProcessEvent70_Light (recursive
 *                          weaker follow-up at GameTime+4)
 *   CHAMPION.C: 27   — PARTY_INFO G0407_s_Party (MagicalLightAmount)
 *
 * DMWeb Dungeon Master Nexus (Saturn) edition page
 *   http://dmweb.free.fr/games/dungeon-master-nexus/editions/sega-saturn/
 *   documents the user-visible symptom: repeated `Ful` / `Oh Ir Ra`
 *   casts can overflow the dungeon light into either a permanent
 *   elevated state or a "plunge into darkness" state, depending on
 *   whether the overflow deltas wrap positive or negative.
 *
 *   DMWeb Nexus Cheats/Hacks page also documents the
 *   "permanent spell effect" exploitation of the same BUG0_18
 *   silent drop (https://dmweb.free.fr/games/dungeon-master-nexus/
 *   solutions/cheats-and-hacks/).
 *
 * Bug classification
 * ------------------
 * The bounded `Nexus_V1_LightOverflowKind` enum exposes the four
 * outcomes that the dmweb + greatstone + ReDMCSB evidence jointly
 * identifies. Firestaff chooses to EMULATE the documented behavior
 * by default (source-faithful V1 path) and lets higher layers flip
 * `guard_rejects` on the timeline for any context where the
 * permanent-light / light-bleed side effect would break gameplay
 * (e.g. an automated replay capture).
 */

#include "nexus_v1_light_overflow.h"
#include <string.h>

/* ReDMCSB DATA.C:359 — verified against both data tables defined
 * at DATA.C:359 and DATA.C:1088 (identical content). */
const int16_t nexus_v1_light_power_to_amount[NEXUS_V1_LIGHT_POWER_TABLE_SIZE] = {
    0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100
};

/* MENU.C:1922 — AL1267_ui_SpellPower = (L1268_i_PowerSymbolOrdinal + 1) << 2
 * The power symbol ordinal is 0..4 (Lo..Mon); SpellPower = 4,8,12,16,20. */
int nexus_v1_light_spellpower_for_ordinal(int power_symbol_ordinal) {
    if (power_symbol_ordinal < 0 || power_symbol_ordinal > 4) return -1;
    return (power_symbol_ordinal + 1) << 2;
}

/* MENU.C:1927 — Light (Lo Oh Ir Ra):
 *   AL1267_ui_Ticks = 10000 + ((AL1267_ui_SpellPower - 8) << 9);
 *   AL1267_ui_LightPower >>= 1;
 *   AL1267_ui_LightPower--;
 *
 * LightPower is the same field re-used at MENU.C:1936. We expose
 * the formula as a function so the cast wrapper can reuse it. */
int nexus_v1_light_initial_power_for_light(int spell_power) {
    /* Mirror the MENU.C sequence exactly: bit-shift then subtract.
     * Negative/zero spell_power is rejected — caller is expected
     * to validate via nexus_v1_light_spellpower_for_ordinal(). */
    if (spell_power <= 0) return 0;
    return (spell_power >> 1) - 1;
}

/* MENU.C:1932 — Magic Torch (Lo Ful):
 *   AL1267_ui_Ticks = 2000 + ((AL1267_ui_SpellPower - 3) << 7);
 *   AL1267_ui_LightPower >>= 2;
 *   AL1267_ui_LightPower++;
 *
 * The lowest valid spell_power (4) yields LightPower 2; PowerSymbol
 * ordinal 0 maps to LightPower 2. */
int nexus_v1_light_initial_power_for_torch(int spell_power) {
    if (spell_power <= 0) return 0;
    return (spell_power >> 2) + 1;
}

/* MENU.C:1940 — Darkness:
 *   AL1267_ui_LightPower >>= 2;
 *   G0407_s_Party.MagicalLightAmount -= G0039_ai_Graphic562_LightPowerToLightAmount[AL1267_ui_LightPower];
 *   F0404_MENUS_CreateEvent70_Light(AL1267_ui_LightPower, 98);
 *
 * Darkness uses positive LightPower (no bit-not), so the decay
 * event carries a positive amount and MagicalLightAmount decreases
 * once per fire. */
int nexus_v1_light_initial_power_for_darkness(int spell_power) {
    if (spell_power <= 0) return 0;
    return spell_power >> 2;
}

int16_t nexus_v1_light_amount_for_power(int light_power) {
    if (light_power < 0) light_power = -light_power;
    if (light_power >= NEXUS_V1_LIGHT_POWER_TABLE_SIZE) {
        /* Saturate at the documented table cap. Matches the original
         * table semantics: powers above 15 contribute full party
         * light (100). */
        return nexus_v1_light_power_to_amount[NEXUS_V1_LIGHT_POWER_TABLE_SIZE - 1];
    }
    return nexus_v1_light_power_to_amount[light_power];
}

void nexus_v1_light_state_init(Nexus_V1_LightState *state) {
    if (!state) return;
    state->magical_light_amount = 0;
}

void nexus_v1_light_timeline_init(Nexus_V1_LightTimeline *tl, int guard_rejects) {
    if (!tl) return;
    memset(tl, 0, sizeof(*tl));
    tl->guard_rejects = guard_rejects ? 1 : 0;
}

/* Internal: schedule a Light event. Returns 1 on success, 0 if the
 * BUG0_18 silent-drop path was taken (timeline full).
 *
 * Mirrors TIMELINE.C F0238 first-half: if (G0372_ui_EventCount ==
 * G0369_EventMaximumCount) return 0;
 *
 * We do not model the "duplicate existing event with same map+time
 * + same type" path (TIMELINE.C F0238 second-half) because Light
 * events are inherently recursive in F0257 — every weaker event
 * has its own GameTime so the duplicate-collapse branch never
 * applies to C70_EVENT_LIGHT. */
static int light_timeline_schedule(Nexus_V1_LightTimeline *tl,
                                   int16_t light_power,
                                   uint32_t fire_at_tick)
{
    if (!tl) return 0;
    if (light_power == 0) {
        /* F0257 first line: if (LightPower == 0) return; — and
         * F0404's MENU.C caller never schedules a 0-power event
         * anyway (the cast site sets initial power >= 2). We keep
         * the early-exit so probe-driven degenerate inputs cannot
         * pollute the buffer. */
        return 1;
    }
    if (tl->active_count >= NEXUS_V1_LIGHT_TIMELINE_BASE_CAP) {
        /* BUG0_18 silent drop. Count it so the classification hook
         * can surface it without us having to track a parallel flag. */
        tl->dropped_counter++;
        return 0;
    }
    for (size_t i = 0; i < NEXUS_V1_LIGHT_TIMELINE_BASE_CAP; ++i) {
        if (!tl->slots[i].in_use) {
            tl->slots[i].type = 70; /* C70_EVENT_LIGHT */
            tl->slots[i].light_power = light_power;
            tl->slots[i].fire_at_tick = fire_at_tick;
            tl->slots[i].in_use = 1;
            tl->active_count++;
            return 1;
        }
    }
    /* Should be unreachable given the count check above, but mirror
     * the silent-drop semantics if the bookkeeping ever disagrees. */
    tl->dropped_counter++;
    return 0;
}

/* Internal: free a slot by index. */
static void light_timeline_release(Nexus_V1_LightTimeline *tl, size_t idx) {
    if (!tl || idx >= NEXUS_V1_LIGHT_TIMELINE_BASE_CAP) return;
    if (!tl->slots[idx].in_use) return;
    tl->slots[idx].in_use = 0;
    tl->slots[idx].light_power = 0;
    tl->slots[idx].fire_at_tick = 0;
    if (tl->active_count > 0) tl->active_count--;
}

/* Internal: process one fired Light event per TIMELINE.C F0257.
 *
 *   if (LightPower == 0) return;                          F0257 line 1747
 *   if (LightPower < 0) LightPower = -LightPower;         F0257 line 1750
 *   weaker = LightPower - 1;                              F0257 line 1753
 *   delta = Table[LightPower] - Table[weaker];            F0257 line 1754
 *   if (was_negative) delta = -delta; weaker = -weaker;   F0257 line 1755-1758
 *   MagicalLightAmount += delta;                          F0257 line 1759
 *   if (weaker) schedule weakevent with LightPower=weaker; F0257 line 1760
 *                                                          at GameTime+4
 *
 * Negative-light events are the auto-decay chain. Each step reduces
 * |LightPower| by 1 and the delta equals the difference between the
 * current contribution and the next-weaker contribution, so the
 * chain sums back to LightPowerToLightAmount[initial_power].
 *
 * Positive-light events are Darkness casts; their delta is positive
 * (subtracted from MagicalLightAmount).
 */
static void light_process_event(Nexus_V1_LightTimeline *tl,
                                Nexus_V1_LightState *state,
                                size_t slot_idx)
{
    if (!tl || !state || slot_idx >= NEXUS_V1_LIGHT_TIMELINE_BASE_CAP) return;
    if (!tl->slots[slot_idx].in_use) return;

    int16_t lp = tl->slots[slot_idx].light_power;
    int16_t weaker;
    int32_t delta;
    int was_negative = 0;

    if (lp == 0) {
        light_timeline_release(tl, slot_idx);
        return;
    }
    if (lp < 0) {
        was_negative = 1;
        lp = (int16_t)(-lp);
    }
    weaker = (int16_t)(lp - 1);

    /* F0257 line 1754: delta = Table[LightPower] - Table[weaker].
     * Clamp `weaker` so the negative lookup stays safe. */
    int16_t table_now  = nexus_v1_light_amount_for_power(lp);
    int16_t table_next = nexus_v1_light_amount_for_power(weaker);
    delta = (int32_t)table_now - (int32_t)table_next;

    if (was_negative) {
        delta = -delta;
        weaker = (int16_t)(-weaker);
    }

    state->magical_light_amount += delta;

    if (weaker) {
        /* F0257 line 1760-1763 — schedule the next-weaker event.
         * M033_SET_MAP_AND_TIME uses GameTime + 4; we use current_tick + 4
         * since the timeline is being advanced tick by tick. */
        light_timeline_schedule(tl, weaker,
                                tl->current_tick + NEXUS_V1_LIGHT_DECAY_STEP_TICKS);
    }
    tl->decay_counter++;
    light_timeline_release(tl, slot_idx);
}

int nexus_v1_light_timeline_cast(Nexus_V1_LightTimeline *tl,
                                 Nexus_V1_LightState *state,
                                 Nexus_V1_LightKind kind,
                                 int power_symbol_ordinal)
{
    if (!tl || !state) return 0;
    int spell_power = nexus_v1_light_spellpower_for_ordinal(power_symbol_ordinal);
    if (spell_power <= 0) return 0;

    /* MENU.C:1926-1942 dispatch. Compute initial LightPower + Ticks
     * the same way the original does, then apply the immediate
     * MagicalLightAmount update and schedule the C70_EVENT_LIGHT
     * decay chain. */
    int16_t light_power = 0;
    uint32_t ticks = 0;
    int32_t immediate_delta = 0;
    int16_t initial_event_power = 0;

    switch (kind) {
        case NEXUS_LIGHT_KIND_LIGHT:
            light_power = (int16_t)nexus_v1_light_initial_power_for_light(spell_power);
            ticks = (uint32_t)(10000 + ((spell_power - 8) << 9));
            if (light_power <= 0) return 0;
            /* MENU.C:1936 — MagicalLightAmount += Table[LightPower] */
            immediate_delta = nexus_v1_light_amount_for_power(light_power);
            initial_event_power = (int16_t)(-light_power); /* F0404 uses -LightPower */
            break;

        case NEXUS_LIGHT_KIND_TORCH:
            light_power = (int16_t)nexus_v1_light_initial_power_for_torch(spell_power);
            ticks = (uint32_t)(2000 + ((spell_power - 3) << 7));
            if (light_power <= 0) return 0;
            /* MENU.C:1936 — same path as Light */
            immediate_delta = nexus_v1_light_amount_for_power(light_power);
            initial_event_power = (int16_t)(-light_power);
            break;

        case NEXUS_LIGHT_KIND_DARKNESS:
            light_power = (int16_t)nexus_v1_light_initial_power_for_darkness(spell_power);
            ticks = 98; /* MENU.C:1942 */
            if (light_power <= 0) return 0;
            /* MENU.C:1941 — MagicalLightAmount -= Table[LightPower] */
            immediate_delta = -(int32_t)nexus_v1_light_amount_for_power(light_power);
            /* MENU.C:1942 — positive LightPower, NOT negated */
            initial_event_power = light_power;
            break;

        default:
            return 0;
    }

    /* Guard mode: refuse the cast if the timeline is already full.
     * Documented alternative to BUG0_18 silent drop. The caller can
     * flip this off via nexus_v1_light_timeline_init(..., guard_rejects=0)
     * to recover the documented behaviour. */
    if (tl->guard_rejects && tl->active_count >= NEXUS_V1_LIGHT_TIMELINE_BASE_CAP) {
        return 0;
    }

    /* MENU.C:1936/1941 — immediate MagicalLightAmount update.
     * Done BEFORE scheduling the decay event so the immediate-light
     * total reflects what the player just observed. */
    state->magical_light_amount += immediate_delta;

    /* MENU.C:1937/1942 — F0404_MENUS_CreateEvent70_Light call.
     * fire_at_tick = GameTime + ticks (M033_SET_MAP_AND_TIME). */
    if (!light_timeline_schedule(tl, initial_event_power,
                                 tl->current_tick + ticks))
    {
        /* BUG0_18 silent drop. The cast succeeded visually but the
         * decay chain will not be scheduled. We still count the
         * cast so classify() can detect "many casts, no decays". */
    }

    tl->cast_counter++;
    return light_power;
}

size_t nexus_v1_light_timeline_tick(Nexus_V1_LightTimeline *tl,
                                    Nexus_V1_LightState *state)
{
    if (!tl || !state) return 0;
    size_t fired = 0;
    /* Walk the slot array once. F0257 schedules one weaker follow-up
     * per fire so the worst case per tick is O(active_count + 1). */
    for (size_t i = 0; i < NEXUS_V1_LIGHT_TIMELINE_BASE_CAP; ++i) {
        if (tl->slots[i].in_use && tl->slots[i].fire_at_tick <= tl->current_tick) {
            light_process_event(tl, state, i);
            fired++;
        }
    }
    tl->current_tick++;
    return fired;
}

size_t nexus_v1_light_timeline_advance(Nexus_V1_LightTimeline *tl,
                                       Nexus_V1_LightState *state,
                                       size_t n_ticks)
{
    size_t total_fired = 0;
    for (size_t i = 0; i < n_ticks; ++i) {
        total_fired += nexus_v1_light_timeline_tick(tl, state);
    }
    return total_fired;
}

Nexus_V1_LightOverflowKind
nexus_v1_light_overflow_classify(const Nexus_V1_LightTimeline *tl,
                                 const Nexus_V1_LightState *state)
{
    if (!tl || !state) return NEXUS_LIGHT_OVERFLOW_NONE;

    /* Light bleed takes priority: a negative MagicalLightAmount
     * renders the dungeon visually dark in the original
     * (PANEL.C:412 clamps to 0, but F0337_INVENTORY_SetDungeonViewPalette
     * sees the negative pre-clamp). We surface this as the
     * documented "plunge the dungeon into darkness" symptom. */
    if (state->magical_light_amount < 0) {
        return NEXUS_LIGHT_OVERFLOW_LIGHT_BLEED_NEGATIVE;
    }

    /* Timeline full + decays not keeping up with casts = permanent
     * elevated light. We require BUG0_18 hits (dropped_counter > 0)
     * OR a sustained full buffer without drops but with very high
     * magical-light amount to flag this. The "no drops + full + high"
     * branch covers the slow-creep case where each cast fits but
     * the decay chain never shrinks (initial_power high, ticks low). */
    if (tl->dropped_counter > 0 && tl->active_count >= NEXUS_V1_LIGHT_TIMELINE_BASE_CAP) {
        return NEXUS_LIGHT_OVERFLOW_TIMELINE_FULL_PERMANENT_LIGHT;
    }
    if (state->magical_light_amount > 200 &&
        tl->active_count >= (NEXUS_V1_LIGHT_TIMELINE_BASE_CAP / 2) &&
        tl->cast_counter > tl->decay_counter)
    {
        /* Conservative heuristic: many casts produced few decays AND
         * magical light total is well above the documented single
         * max-power cast contribution (100). Avoids false positives
         * during normal sustained-light play. */
        return NEXUS_LIGHT_OVERFLOW_TIMELINE_FULL_PERMANENT_LIGHT;
    }
    return NEXUS_LIGHT_OVERFLOW_NONE;
}

int nexus_v1_light_overflow_should_guard(
    const Nexus_V1_LightTimeline *tl,
    const Nexus_V1_LightState *state)
{
    if (!tl) return 0;
    if (!tl->guard_rejects) return 0;
    /* Guard mode: reject casts once the timeline is full. The
     * classification hook is independent: callers can still inspect
     * the live state for HUD/debug purposes even when this returns 1. */
    if (tl->active_count >= NEXUS_V1_LIGHT_TIMELINE_BASE_CAP) return 1;
    (void)state;
    return 0;
}
