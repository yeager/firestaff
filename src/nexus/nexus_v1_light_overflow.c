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
#include "nexus_v1_save.h"  /* NEXUS_SAVE_OK + NEXUS_SAVE_ERR_* */
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

/* ═══════════════════════════════════════════════════════════════════
 * Runtime cast-path wire-in (M11 / M12 hook)
 * ═══════════════════════════════════════════════════════════════════
 *
 * These helpers wrap the data-only Nexus_V1_LightState +
 * Nexus_V1_LightTimeline pair into a single bundle that the
 * future M11 cast dispatch (or a debug overlay / replay capture)
 * can call. The cast-path helpers preserve the emulate-vs-guard
 * mode distinction by carrying the `mode` flag alongside the
 * timeline's own `guard_rejects` flag, so a caller can flip
 * modes at runtime without rebuilding the cast-path struct.
 *
 * ReDMCSB anchors (cross-referenced with the data model above):
 *   - MENU.C:1926-1942  (Light / Torch / Darkness dispatch)
 *   - TIMELINE.C F0238  (BUG0_18 silent drop)
 *   - TIMELINE.C F0257  (recursive weaker decay at GameTime+4)
 *   - CHAMPION.C:27     (MagicalLightAmount state)
 *   - LOADSAVE.C:2041   (EventMaximumCount = 100 base cap)
 *
 * Source-locked field-by-field for every accessor below.
 */

void nexus_v1_light_cast_path_init(Nexus_V1_LightCastPath *path,
                                    Nexus_V1_LightCastMode mode)
{
    if (!path) return;
    memset(path, 0, sizeof(*path));
    nexus_v1_light_state_init(&path->state);
    /* guard_rejects mirrors the cast-path mode. The cast-path
     * helpers below consult the mode flag for the public API,
     * but nexus_v1_light_timeline_cast() reads guard_rejects
     * directly so we keep them in sync. */
    nexus_v1_light_timeline_init(&path->timeline, (mode == NEXUS_LIGHT_CAST_MODE_GUARD) ? 1 : 0);
    path->mode = mode;
}

void nexus_v1_light_cast_path_reset(Nexus_V1_LightCastPath *path)
{
    if (!path) return;
    Nexus_V1_LightCastMode saved_mode = path->mode;
    nexus_v1_light_cast_path_init(path, saved_mode);
}

Nexus_V1_LightCastResult
nexus_v1_light_cast_path_cast(Nexus_V1_LightCastPath *path,
                              Nexus_V1_LightKind kind,
                              int power_symbol_ordinal)
{
    Nexus_V1_LightCastResult r;
    memset(&r, 0, sizeof(r));
    if (!path) {
        /* Even the "no path" case returns a populated result so a
         * caller can treat NULL as a soft no-op without a separate
         * branch. */
        r.classification = NEXUS_LIGHT_OVERFLOW_NONE;
        r.applied_light_power = 0;
        r.was_rejected = 1;
        r.magical_light_amount_after = 0;
        return r;
    }

    /* Sync the timeline's guard flag with the cast-path mode so
     * the underlying cast helper sees the same policy the public
     * accessor advertises. This is the only place where the two
     * flags diverge. */
    path->timeline.guard_rejects =
        (path->mode == NEXUS_LIGHT_CAST_MODE_GUARD) ? 1 : 0;

    /* Track pre-cast light so we can detect a guard reject (the
     * timeline will not have changed MagicalLightAmount in that
     * case). */
    int32_t pre_light = path->state.magical_light_amount;

    /* Forward to the data-model cast helper. Returns the LightPower
     * that was applied, or 0 if dropped/rejected. The cast-path
     * helper routes through this so the BUG0_18 silent drop and
     * the F0257 recursive weaker chain are preserved verbatim. */
    int applied = nexus_v1_light_timeline_cast(&path->timeline,
                                                &path->state,
                                                kind,
                                                power_symbol_ordinal);

    r.applied_light_power = applied;
    /* Guard mode rejects return 0 AND leave MagicalLightAmount
     * untouched. Detect that combination explicitly. */
    r.was_rejected = (applied == 0) ? 1 : 0;
    /* If the cast helper returned 0 because the input was invalid
     * (e.g. an out-of-range power_symbol_ordinal), the timeline
     * is also unchanged. Distinguish "rejected by guard" from
     * "rejected by validation" by also checking the pre-state. */
    if (r.was_rejected &&
        path->state.magical_light_amount != pre_light)
    {
        /* Sanity guard: if the data-model helper mutated state,
         * the rejection must have been a validation error, not
         * a guard reject. */
        r.was_rejected = 0;
    }

    r.magical_light_amount_after = path->state.magical_light_amount;
    r.classification = nexus_v1_light_overflow_classify(&path->timeline,
                                                        &path->state);
    return r;
}

size_t nexus_v1_light_cast_path_tick(Nexus_V1_LightCastPath *path) {
    if (!path) return 0;
    return nexus_v1_light_timeline_tick(&path->timeline, &path->state);
}

size_t nexus_v1_light_cast_path_advance(Nexus_V1_LightCastPath *path,
                                        size_t n_ticks) {
    if (!path) return 0;
    return nexus_v1_light_timeline_advance(&path->timeline, &path->state, n_ticks);
}

Nexus_V1_LightOverflowKind
nexus_v1_light_cast_path_classify(const Nexus_V1_LightCastPath *path) {
    if (!path) return NEXUS_LIGHT_OVERFLOW_NONE;
    return nexus_v1_light_overflow_classify(&path->timeline, &path->state);
}

int nexus_v1_light_cast_path_should_guard(const Nexus_V1_LightCastPath *path) {
    if (!path) return 0;
    if (path->mode != NEXUS_LIGHT_CAST_MODE_GUARD) return 0;
    /* Match the data-model helper so a probe / test that drives
     * the cast path and the underlying timeline sees the same
     * guard verdict. */
    return nexus_v1_light_overflow_should_guard(&path->timeline, &path->state);
}

void nexus_v1_light_cast_path_set_mode(Nexus_V1_LightCastPath *path,
                                        Nexus_V1_LightCastMode mode) {
    if (!path) return;
    path->mode = mode;
    path->timeline.guard_rejects = (mode == NEXUS_LIGHT_CAST_MODE_GUARD) ? 1 : 0;
}

Nexus_V1_LightCastMode
nexus_v1_light_cast_path_get_mode(const Nexus_V1_LightCastPath *path) {
    if (!path) return NEXUS_LIGHT_CAST_MODE_EMULATE;
    return path->mode;
}

int32_t nexus_v1_light_cast_path_light_amount(
    const Nexus_V1_LightCastPath *path) {
    if (!path) return 0;
    return path->state.magical_light_amount;
}

uint32_t nexus_v1_light_cast_path_cast_count(
    const Nexus_V1_LightCastPath *path) {
    if (!path) return 0;
    return path->timeline.cast_counter;
}

uint32_t nexus_v1_light_cast_path_decay_count(
    const Nexus_V1_LightCastPath *path) {
    if (!path) return 0;
    return path->timeline.decay_counter;
}

uint32_t nexus_v1_light_cast_path_dropped_count(
    const Nexus_V1_LightCastPath *path) {
    if (!path) return 0;
    return path->timeline.dropped_counter;
}

size_t nexus_v1_light_cast_path_active_count(
    const Nexus_V1_LightCastPath *path) {
    if (!path) return 0;
    return path->timeline.active_count;
}

/* ── Save/load proof for the cast-path state (FNXL format) ───────── */

size_t nexus_v1_light_cast_path_save_size(const Nexus_V1_LightCastPath *path) {
    if (!path) return 0;
    /* Magic + version + mode + scalars + crc + per-active-slot
     * payload. See header for the format. */
    return 4u + 2u + 2u + 4u + 4u + 4u + 4u + 4u + 4u + 4u
           + (size_t)path->timeline.active_count *
             (1u + 2u + 4u);
}

static uint32_t fnxl_crc32(const uint8_t *data, size_t len) {
    /* Same zlib polynomial the FNXS save loader uses
     * (crc32_init in src/nexus/nexus_v1_save_load.c). We re-init
     * the table here so the cast-path module stays standalone
     * (it should be callable from a probe without the world
     * save loader present). */
    static uint32_t table[256];
    static int table_ready = 0;
    if (!table_ready) {
        uint32_t poly = 0xEDB88320U;
        int i, j;
        for (i = 0; i < 256; i++) {
            uint32_t c = (uint32_t)i;
            for (j = 0; j < 8; j++) c = (c >> 1) ^ ((c & 1) ? poly : 0);
            table[i] = c;
        }
        table_ready = 1;
    }
    uint32_t crc = 0xFFFFFFFFU;
    size_t i;
    for (i = 0; i < len; i++) {
        crc = table[(crc ^ data[i]) & 0xFFu] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFU;
}

static uint8_t *cp_wr8(uint8_t *p, uint8_t v)  { *p++ = v;  return p; }
static uint8_t *cp_wr16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v);      p[1] = (uint8_t)(v >> 8);
    return p + 2;
}
static uint8_t *cp_wr32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v);      p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
    return p + 4;
}
static const uint8_t *cp_rd8(const uint8_t *p, uint8_t *v)  { *v = *p++; return p; }
static const uint8_t *cp_rd16(const uint8_t *p, uint16_t *v) {
    *v = (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
    return p + 2;
}
static const uint8_t *cp_rd32(const uint8_t *p, uint32_t *v) {
    *v = (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
         ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    return p + 4;
}

size_t nexus_v1_light_cast_path_save(const Nexus_V1_LightCastPath *path,
                                      void *buf, size_t bufsize)
{
    if (!path || !buf) return 0;
    size_t needed = nexus_v1_light_cast_path_save_size(path);
    if (bufsize < needed) return 0;

    uint8_t *p = (uint8_t *)buf;
    const uint8_t *crc_start = p;
    p = cp_wr32(p, NEXUS_LIGHT_CAST_PATH_SAVE_MAGIC);
    p = cp_wr16(p, NEXUS_LIGHT_CAST_PATH_SAVE_VERSION);
    p = cp_wr16(p, (uint16_t)path->mode);
    p = cp_wr32(p, (uint32_t)path->state.magical_light_amount);
    p = cp_wr32(p, path->timeline.current_tick);
    p = cp_wr32(p, path->timeline.cast_counter);
    p = cp_wr32(p, path->timeline.decay_counter);
    p = cp_wr32(p, path->timeline.dropped_counter);
    p = cp_wr32(p, (uint32_t)path->timeline.active_count);
    /* CRC over everything written so far (excluding the CRC field
     * itself). We retro-fit: write a placeholder, compute CRC over
     * [crc_start, p), then overwrite. */
    uint8_t *crc_pos = p;
    p = cp_wr32(p, 0xDEADBEEFu);
    /* Now write the active slots in declared order. */
    for (size_t i = 0; i < NEXUS_V1_LIGHT_TIMELINE_BASE_CAP; ++i) {
        const Nexus_V1_LightEvent *e = &path->timeline.slots[i];
        if (!e->in_use) continue;
        p = cp_wr8(p, e->type);
        p = cp_wr16(p, (uint16_t)e->light_power);
        p = cp_wr32(p, e->fire_at_tick);
    }
    /* Replace the placeholder with the actual CRC. */
    uint32_t crc = fnxl_crc32(crc_start, (size_t)(crc_pos - crc_start));
    cp_wr32(crc_pos, crc);
    return (size_t)(p - (uint8_t *)buf);
}

int nexus_v1_light_cast_path_load(Nexus_V1_LightCastPath *path,
                                    const void *buf, size_t bufsize)
{
    if (!path || !buf) return NEXUS_SAVE_ERR_NULL;

    /* Always reset on entry so a failed load leaves the path in
     * a known state. We do not require the caller to pre-init. */
    nexus_v1_light_cast_path_init(path, NEXUS_LIGHT_CAST_MODE_EMULATE);

    /* Minimum header is 4 (magic) + 2 (version) + 2 (mode) +
     * 5 * 4 (scalars) + 4 (active_count) + 4 (crc) = 36 bytes. */
    if (bufsize < 36u) return NEXUS_SAVE_ERR_READ;

    const uint8_t *p = (const uint8_t *)buf;
    uint32_t magic, v32;
    uint16_t version, mode_u16;
    p = cp_rd32(p, &magic);
    if (magic != NEXUS_LIGHT_CAST_PATH_SAVE_MAGIC) return NEXUS_SAVE_ERR_MAGIC;
    p = cp_rd16(p, &version);
    if (version != NEXUS_LIGHT_CAST_PATH_SAVE_VERSION) return NEXUS_SAVE_ERR_VERSION;
    p = cp_rd16(p, &mode_u16);

    /* CRC check: cover everything from the start of the buffer up
     * to (but not including) the CRC field. */
    size_t crc_offset = 4u + 2u + 2u + 4u + 4u + 4u + 4u + 4u + 4u;
    if (bufsize < crc_offset + 4u) return NEXUS_SAVE_ERR_READ;
    uint32_t stored_crc;
    const uint8_t *crc_pos = p + (crc_offset - 4u);
    /* The CRC field is the last 4 bytes of the fixed header. */
    (void)crc_pos;
    {
        const uint8_t *tmp = (const uint8_t *)buf + crc_offset;
        cp_rd32(tmp, &stored_crc);
    }
    uint32_t computed = fnxl_crc32((const uint8_t *)buf, crc_offset);
    if (stored_crc != computed) return NEXUS_SAVE_ERR_CRC;

    p = cp_rd32(p, &v32); path->state.magical_light_amount = (int32_t)v32;
    p = cp_rd32(p, &path->timeline.current_tick);
    p = cp_rd32(p, &path->timeline.cast_counter);
    p = cp_rd32(p, &path->timeline.decay_counter);
    p = cp_rd32(p, &path->timeline.dropped_counter);
    p = cp_rd32(p, &v32);
    uint32_t active_count = v32;
    if (active_count > NEXUS_V1_LIGHT_TIMELINE_BASE_CAP) {
        return NEXUS_SAVE_ERR_READ;
    }
    /* Skip past the CRC field. */
    p = cp_rd32(p, &stored_crc);

    /* Clear all slots, then populate in declared order so the
     * round-trip is deterministic. */
    memset(path->timeline.slots, 0, sizeof(path->timeline.slots));
    path->timeline.active_count = 0;
    for (uint32_t i = 0; i < active_count; ++i) {
        if ((size_t)(p - (const uint8_t *)buf) + 7u > bufsize) {
            /* Truncated payload. Roll back to fresh init. */
            nexus_v1_light_cast_path_init(path, NEXUS_LIGHT_CAST_MODE_EMULATE);
            return NEXUS_SAVE_ERR_READ;
        }
        uint8_t  type;
        uint16_t power_u16;
        uint32_t fire;
        p = cp_rd8(p, &type);
        p = cp_rd16(p, &power_u16);
        p = cp_rd32(p, &fire);
        path->timeline.slots[i].type          = type;
        path->timeline.slots[i].light_power   = (int16_t)power_u16;
        path->timeline.slots[i].fire_at_tick  = fire;
        path->timeline.slots[i].in_use        = 1;
        path->timeline.active_count++;
    }

    /* Mode goes in last so the cast-path state matches the
     * saved mode flag exactly. */
    path->mode = (mode_u16 == (uint16_t)NEXUS_LIGHT_CAST_MODE_GUARD)
                 ? NEXUS_LIGHT_CAST_MODE_GUARD
                 : NEXUS_LIGHT_CAST_MODE_EMULATE;
    path->timeline.guard_rejects = (path->mode == NEXUS_LIGHT_CAST_MODE_GUARD) ? 1 : 0;
    return NEXUS_SAVE_OK;
}

int nexus_v1_light_cast_path_probe(const void *buf, size_t bufsize) {
    if (!buf || bufsize < 4u) return 0;
    const uint8_t *p = (const uint8_t *)buf;
    uint32_t magic = (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
                     ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    return (magic == NEXUS_LIGHT_CAST_PATH_SAVE_MAGIC) ? 1 : 0;
}
