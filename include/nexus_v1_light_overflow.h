/*
 * nexus_v1_light_overflow.h
 * =========================
 *
 * Nexus V1 light-overflow data model + bug-classification API.
 *
 * Background
 * ----------
 * DMWeb's Dungeon Master Nexus page documents a light-overflow bug
 * where repeated casting of `Ful` (Magic Torch) or `Oh Ir Ra` (Light)
 * in a short interval makes the spell effect permanent and pushes
 * the dungeon into a sustained elevated-light state ("plunges the
 * dungeon into darkness" if the wrap goes negative — both
 * manifestations share the same ReDMCSB root cause described below).
 *
 * ReDMCSB root cause (BUG0_18):
 *   F0238_TIMELINE_AddEvent_GetEventIndex_CPSE returns 0 (silently
 *   drops) when the timeline is full (G0372_ui_EventCount ==
 *   G0369_EventMaximumCount). When the player casts Light/Torch
 *   faster than the previous cast's auto-decay chain finishes, the
 *   timeline fills with C70_EVENT_LIGHT entries. New casts fail to
 *   schedule their decay event chain (F0257_TIMELINE_ProcessEvent70_Light
 *   is the only thing that walks LightPower -> LightPower-1 at
 *   GameTime+4). With no decay queued, G0407_s_Party.MagicalLightAmount
 *   stays elevated, leaving the spell effect "permanent" until the
 *   timeline drains enough to accept a fresh decay event.
 *
 *   The negative side of the same overflow: the per-tick `-amount`
 *   delta can flip the magical-light total through 0 and into the
 *   negative range (e.g. `F0337_INVENTORY_SetDungeonViewPalette`
 *   treats <0 light as "pitch darkness" in the original).
 *
 *   Source citations:
 *     - TIMELINE.C F0238 lines 487-555 (AddEvent silent drop)
 *     - TIMELINE.C F0257 lines 1720-1767 (Light decay + recursive
 *       weaker scheduling)
 *     - MENU.C F0404 lines 1125-1143 (CreateEvent70_Light wrapper)
 *     - MENU.C lines 1926-1942 (Light/Torch/Darkness spell dispatch)
 *     - DATA.C line 359 (LightPowerToLightAmount[16] table)
 *     - CHAMPION.C line 529/645 (MagicalLightAmount torch equip)
 *     - LOADSAVE.C line 2041 (EventMaximumCount = 100 base, then
 *       +Group/+Projectile/+Thing extensions at line 2064)
 *
 * What this header provides
 * -------------------------
 * A pure data-model + classification API:
 *
 *   Nexus_V1_LightState        — MagicalLightAmount + 16-entry lookup
 *                                table (G0039_ai_Graphic562_LightPowerToLightAmount)
 *   Nexus_V1_LightTimeline     — Bounded event slot array with the
 *                                documented per-cast behaviour and
 *                                the BUG0_18 silent drop semantics
 *   Nexus_V1_LightOverflowKind — Output of the classification API
 *                                (no bug / permanent light / dark
 *                                bleed-through / pending transient)
 *
 *   nexus_v1_light_state_init()
 *   nexus_v1_light_timeline_init()
 *   nexus_v1_light_timeline_cast()       — wraps MENU.C:1926-1942
 *   nexus_v1_light_timeline_tick()       — wraps TIMELINE.C F0257
 *   nexus_v1_light_overflow_classify()   — bounded policy hook
 *
 * The classification hook is what lets the future M11 runtime or a
 * guard decide between emulating (let the timeline overflow and
 * reproduce the original "permanent Light" state for source-faithful
 * mode) and guarding (clamp the MagicalLightAmount delta per cast to
 * avoid the overflow in non-strict source-faithful mode).
 *
 * This header is data-free: it does not touch any real Nexus asset.
 */

#ifndef NEXUS_V1_LIGHT_OVERFLOW_H
#define NEXUS_V1_LIGHT_OVERFLOW_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DATA.C:359 — G0039_ai_Graphic562_LightPowerToLightAmount.
 * Indexed by LightPower 0..15. LightPower 0 has no contribution; values
 * climb non-linearly to 100 at LightPower 15 (saturates at full party
 * light). Source: ReDMCSB Toolchains/Common/Source/DATA.C line 359. */
#define NEXUS_V1_LIGHT_POWER_TABLE_SIZE 16
extern const int16_t nexus_v1_light_power_to_amount[NEXUS_V1_LIGHT_POWER_TABLE_SIZE];

/* ReDMCSB LOADSAVE.C line 2041 — G0369_EventMaximumCount = 100 base
 * for a new game, then incremented per C04_THING_TYPE_GROUP and
 * C14_THING_TYPE_PROJECTILE thing counts at LOADSAVE.C line 2064.
 * The BUG0_18 silent drop triggers when the active count hits this
 * cap. We expose it so a guard or strict source-faithful path can
 * reuse the documented budget without poking into the runtime. */
#define NEXUS_V1_LIGHT_TIMELINE_BASE_CAP 100

/* ReDMCSB TIMELINE.C F0257 — M033_SET_MAP_AND_TIME uses
 * G0313_ul_GameTime + 4 to schedule the next-weaker Light event.
 * 4 ticks = ~220ms at 18.2Hz; matching this constant keeps the
 * decay chain deterministic for replay/regression tests. */
#define NEXUS_V1_LIGHT_DECAY_STEP_TICKS 4

/* ReDMUSRB MENU.C lines 1926-1942 — initial LightPower computation:
 *   Light (Lo Oh Ir Ra):    LightPower = (SpellPower >> 1) - 1
 *   Torch (Lo Ful):         LightPower = (SpellPower >> 2) + 1
 *   Darkness:               LightPower =  SpellPower >> 2 (negated)
 *
 * SpellPower in MENU.C:1922 is `(L1268_i_PowerSymbolOrdinal + 1) << 2`,
 * so PowerSymbolOrdinal 0..4 -> SpellPower 4,8,12,16,20. We expose
 * the formulas as small helpers so the cast wrapper can mirror them
 * exactly. */
int nexus_v1_light_spellpower_for_ordinal(int power_symbol_ordinal);
int nexus_v1_light_initial_power_for_light(int spell_power);
int nexus_v1_light_initial_power_for_torch(int spell_power);
int nexus_v1_light_initial_power_for_darkness(int spell_power);

/* Spell kind flags for nexus_v1_light_timeline_cast(). These match
 * the ReDMCSB MENU.C switch on M068_SPELL_TYPE (C0/C1/C5_OTHER_*). */
typedef enum {
    NEXUS_LIGHT_KIND_LIGHT    = 0,   /* MENU.C:1926 Lo Oh Ir Ra */
    NEXUS_LIGHT_KIND_TORCH    = 1,   /* MENU.C:1931 Lo Ful */
    NEXUS_LIGHT_KIND_DARKNESS = 2    /* MENU.C:1939 Darkness */
} Nexus_V1_LightKind;

/* Output of nexus_v1_light_overflow_classify(). Lets callers
 * (engine, M11, or a debug overlay) decide between emulating the
 * original "permanent Light" bug or guarding it away. */
typedef enum {
    /* Timeline is below the documented silent-drop threshold and
     * MagicalLightAmount is in the normal range. No bug surface. */
    NEXUS_LIGHT_OVERFLOW_NONE = 0,

    /* Timeline is at or above the cap. New casts cannot schedule
     * their auto-decay chain. Per BUG0_18 the existing chain will
     * keep the elevated light total in place until the timeline
     * drains. Source-faithful mode should report this as a
     * "permanent light" candidate. */
    NEXUS_LIGHT_OVERFLOW_TIMELINE_FULL_PERMANENT_LIGHT = 1,

    /* MagicalLightAmount wrapped through 0 into the negative range.
     * Per original inventory/palette logic, negative light renders
     * as pitch darkness. The dungeon is still physically lit but
     * the visual map is forced to darkness. Documented on DMWeb
     * as "plunge the dungeon into darkness". */
    NEXUS_LIGHT_OVERFLOW_LIGHT_BLEED_NEGATIVE = 2,

    /* Cast was rejected before scheduling (defensive cap reached or
     * repeated-fast-fire rate exceeded). Caller should treat the
     * cast as a no-op. Only reachable when the guard variant of
     * the timeline is in effect. */
    NEXUS_LIGHT_OVERFLOW_CAST_REJECTED = 3
} Nexus_V1_LightOverflowKind;

/* Per-party light total. Mirrors G0407_s_Party.MagicalLightAmount
 * (CHAMPION.C line 529/645). int32 to keep headroom against long
 * sustained cast sequences. */
typedef struct {
    int32_t magical_light_amount;
} Nexus_V1_LightState;

/* One scheduled light event on the timeline. Mirrors the fields
 * TIMELINE.C F0257 / F0404 actually read: type, LightPower (signed
 * — negative = decay, positive = explicit Darkness event), and
 * GameTime at which the event fires.
 *
 * The recursive "weaker" event is built by F0257 itself when
 * abs(LightPower) > 1; we model that here as a fresh slot rather
 * than as a chain so the bounded cap is honored and the
 * classification can detect the silent drop. */
typedef struct {
    uint8_t  type;          /* always C70_EVENT_LIGHT (70) per TIMELINE.C:1761 */
    int16_t  light_power;   /* signed: positive=Darkness, negative=decay */
    uint32_t fire_at_tick;  /* game tick at which the event fires */
    uint8_t  in_use;        /* 1 = slot occupied, 0 = free */
} Nexus_V1_LightEvent;

/* Bounded light-timeline buffer. Capacity matches the documented
 * G0369_EventMaximumCount + extra headroom for the recursive
 * weaker-event chain (F0257 schedules up to abs(LightPower) - 1
 * follow-ups; we model that explicitly per cast). */
typedef struct {
    Nexus_V1_LightEvent slots[NEXUS_V1_LIGHT_TIMELINE_BASE_CAP];
    size_t              active_count;
    uint32_t            current_tick;
    uint32_t            cast_counter;     /* total Light/Torch casts issued */
    uint32_t            decay_counter;    /* decay events successfully fired */
    uint32_t            dropped_counter;  /* silent drops (BUG0_18 hits) */
    int                 guard_rejects;    /* 0 = emulate, 1 = guard */
} Nexus_V1_LightTimeline;

/* Init / lifecycle. */
void nexus_v1_light_state_init(Nexus_V1_LightState *state);
void nexus_v1_light_timeline_init(Nexus_V1_LightTimeline *tl,
                                  int guard_rejects);

/* Cast wrapper. Mirrors MENU.C lines 1926-1942 (Light/Torch/Darkness
 * dispatch) plus the immediate MagicalLightAmount update at line
 * 1936/1941. Schedules the initial decay event via F0404 semantics
 * (TIMELINE.C F0238 silent-drop applies when the buffer is full).
 *
 * Returns the LightPower that was applied (negative on cast-time
 * consumption), or 0 if the cast was dropped/rejected.
 */
int nexus_v1_light_timeline_cast(Nexus_V1_LightTimeline *tl,
                                 Nexus_V1_LightState *state,
                                 Nexus_V1_LightKind kind,
                                 int power_symbol_ordinal);

/* Tick wrapper. Drains all events with fire_at_tick <= current_tick.
 * Each fired event applies the per-event delta (TIMELINE.C F0257)
 * and, if abs(LightPower) > 1, schedules the weaker follow-up.
 *
 * Returns the number of events processed this call (0 if nothing
 * was due). */
size_t nexus_v1_light_timeline_tick(Nexus_V1_LightTimeline *tl,
                                    Nexus_V1_LightState *state);

/* Run `n_ticks` ticks of the light timeline at 1-tick resolution.
 * Convenience for tests that want to step deterministically. */
size_t nexus_v1_light_timeline_advance(Nexus_V1_LightTimeline *tl,
                                       Nexus_V1_LightState *state,
                                       size_t n_ticks);

/* Bounded classification. Inspects the timeline + state and returns
 * the documented overflow kind. Idempotent and side-effect free so
 * callers can poll it from a HUD or M11 wire-in. */
Nexus_V1_LightOverflowKind
nexus_v1_light_overflow_classify(const Nexus_V1_LightTimeline *tl,
                                 const Nexus_V1_LightState *state);

/* Decision recommendation helper. Returns:
 *   0  — emulate the original behavior (do nothing, let the bug
 *        surface; correct for source-faithful mode).
 *   1  — guard (the cast should be rejected before raising
 *        MagicalLightAmount, to keep the runtime honest).
 *
 * The classification hook lets the future engine layer pick per
 * game/version; this helper implements the conservative default
 * (emulate when guard_rejects==0, otherwise guard). */
int nexus_v1_light_overflow_should_guard(
    const Nexus_V1_LightTimeline *tl,
    const Nexus_V1_LightState *state);

/* Light-table lookup. Convenience accessor so callers/tests do not
 * need to know the array symbol. Returns 0 for out-of-range index
 * to mirror the documented `LightPowerToLightAmount[0] == 0`. */
int16_t nexus_v1_light_amount_for_power(int light_power);

/* ═══════════════════════════════════════════════════════════════════
 * Runtime cast-path wire-in (M11 / M12 hook)
 * ═══════════════════════════════════════════════════════════════════
 *
 * The Nexus_V1_LightState + Nexus_V1_LightTimeline pair above is a
 * pure data model. The M11 runtime or any caller that wants to
 * apply Light / Torch / Darkness casts in a way that participates
 * in the BUG0_18 overflow classification needs a small bundle that
 * owns the state, the timeline, the emulate/guard policy, and a
 * way to ask "given what just happened, what kind of overflow is
 * this?".
 *
 * Nexus_V1_LightCastPath is that bundle. It is data-free (no real
 * Saturn assets required) and small enough to be embedded directly
 * in Nexus_MechanicsState, Nexus_V1_Engine, or any test fixture.
 *
 * Source-locked against:
 *   - ReDMCSB MENU.C:1926-1942 (Light / Torch / Darkness dispatch)
 *   - ReDMCSB TIMELINE.C F0238 lines 487-555 (BUG0_18 silent drop)
 *   - ReDMCSB TIMELINE.C F0257 lines 1720-1767 (F0257 decay chain)
 *   - DMWeb Dungeon Master Nexus (Saturn) edition page
 *     (http://dmweb.free.fr/games/dungeon-master-nexus/editions/
 *     sega-saturn/) — user-visible overflow symptoms
 *   - DMWeb Nexus Cheats/Hacks page — BUG0_18 permanent-spell-effect
 *     exploitation
 *
 * What "runtime cast path" means here
 * ------------------------------------
 * When the M11 engine receives a Light / Torch / Darkness cast from
 * the input pipeline, it should call
 *   nexus_v1_light_cast_path_cast(&path, kind, power_symbol_ordinal)
 * which:
 *   1. Routes through the existing Nexus_V1_LightTimeline
 *      (preserving the BUG0_18 silent-drop semantics + the
 *      recursive F0257 decay chain).
 *   2. Updates the bounded MagicalLightAmount field on
 *      Nexus_V1_LightState exactly the way MENU.C:1936 / 1941 do.
 *   3. Classifies the result so the caller can react (HUD warning,
 *      guard rejection, replay capture flag, etc.).
 *
 * Emulate vs guard mode distinction is preserved through the
 * `mode` field. The bounded `Nexus_V1_LightCastMode` enum keeps
 * the distinction explicit at every call site so a future reader
 * cannot accidentally flip into "guard everything" mode without
 * noticing.
 */

/* Cast-path mode — matches the gap-row promise that the engine
 * can either EMULATE the documented behavior (source-faithful V1
 * path that reproduces the dmweb-documented "permanent Light" /
 * "dungeon into darkness" symptoms) or GUARD against it (suitable
 * for replay/capture contexts that want a clean light total). */
typedef enum {
    NEXUS_LIGHT_CAST_MODE_EMULATE = 0,  /* source-faithful: let BUG0_18 surface */
    NEXUS_LIGHT_CAST_MODE_GUARD   = 1   /* reject casts at the cap; preserve light */
} Nexus_V1_LightCastMode;

/* Result of one cast applied through the cast path. Distinct from
 * Nexus_V1_LightOverflowKind (which classifies the *current*
 * state) so a single cast can return TIMELINE_FULL_PERMANENT_LIGHT
 * (overflow detected) while a follow-up call returns NONE (the
 * timeline drained enough). */
typedef struct {
    /* The kind returned by the classification hook on the new state
     * AFTER this cast. Callers can compare against
     * NEXUS_LIGHT_OVERFLOW_NONE to detect "no bug surface" casts. */
    Nexus_V1_LightOverflowKind classification;
    /* The LightPower that was applied (negative on cast-time
     * consumption, 0 if the cast was dropped/rejected). Mirrors
     * the return contract of nexus_v1_light_timeline_cast(). */
    int applied_light_power;
    /* 1 if the cast was rejected (guard mode + cap hit), 0 otherwise.
     * Independent of the classification hook so a debug overlay can
     * show "guard reject" without first asking classify(). */
    int was_rejected;
    /* The MagicalLightAmount value AFTER the cast was applied.
     * Lets a debug overlay draw the post-cast light total without
     * having to re-read the LightState struct. */
    int32_t magical_light_amount_after;
} Nexus_V1_LightCastResult;

/* Engine-side bundle. Holds the bounded data model + the policy.
 * The 1.5 KB total footprint (state 4 B + 100-slot timeline ~1.5 KB
 * + 16 B counters) is small enough to embed directly in
 * Nexus_MechanicsState when the engine wants to wire it in. */
typedef struct {
    Nexus_V1_LightState    state;
    Nexus_V1_LightTimeline timeline;
    Nexus_V1_LightCastMode mode;
} Nexus_V1_LightCastPath;

/* Init the cast path. `mode` selects emulate (source-faithful) vs
 * guard. Safe to call with NULL (no-op). The `magic` field on the
 * save/load proof is a sentinel for buffer-overrun detection —
 * a future caller that tries to round-trip a heap-allocated
 * Nexus_V1_LightCastPath will catch an out-of-sync version. */
void nexus_v1_light_cast_path_init(Nexus_V1_LightCastPath *path,
                                    Nexus_V1_LightCastMode mode);

/* Reset to a fresh cast-path state with the same mode. Useful when
 * the M11 engine wants to start a new dungeon run without
 * re-allocating the bundle. */
void nexus_v1_light_cast_path_reset(Nexus_V1_LightCastPath *path);

/* Apply a cast through the cast path. Returns a populated
 * Nexus_V1_LightCastResult. The function is the "live M11 wire-in
 * to apply the cast hook per champion/mana check" called out in
 * the gap row: callers (M11, M12, or a debug overlay) feed it the
 * same kind + power_symbol_ordinal the menu would have and get
 * back the bounded classification + the post-cast light total.
 *
 * The function does NOT touch champion mana or skills; the
 * M11 caller is expected to gate the call behind the existing
 * nexus_v1_cast_spell() mana/skill check so a probe or test that
 * just wants the light-side behavior can drive the cast path
 * directly. */
Nexus_V1_LightCastResult
nexus_v1_light_cast_path_cast(Nexus_V1_LightCastPath *path,
                              Nexus_V1_LightKind kind,
                              int power_symbol_ordinal);

/* Tick the cast path by 1 V1 tick (55 ms / 18.2 Hz). Returns the
 * number of decay events that fired. */
size_t nexus_v1_light_cast_path_tick(Nexus_V1_LightCastPath *path);

/* Run N ticks at once (deterministic for tests / replay capture). */
size_t nexus_v1_light_cast_path_advance(Nexus_V1_LightCastPath *path,
                                        size_t n_ticks);

/* Query the current overflow kind without mutating state. */
Nexus_V1_LightOverflowKind
nexus_v1_light_cast_path_classify(const Nexus_V1_LightCastPath *path);

/* Should the next cast be rejected? Mirrors
 * nexus_v1_light_overflow_should_guard() but goes through the
 * cast-path mode flag so the emulate/guard distinction stays
 * explicit at the call site. Returns 1 to reject, 0 to allow. */
int nexus_v1_light_cast_path_should_guard(const Nexus_V1_LightCastPath *path);

/* Flip the emulate/guard mode at runtime. M11 can call this when
 * the user toggles a setting; the next cast will pick up the new
 * mode. */
void nexus_v1_light_cast_path_set_mode(Nexus_V1_LightCastPath *path,
                                        Nexus_V1_LightCastMode mode);
Nexus_V1_LightCastMode
nexus_v1_light_cast_path_get_mode(const Nexus_V1_LightCastPath *path);

/* Direct accessors for HUD / debug overlays that need the
 * underlying counters without re-classifying. */
int32_t nexus_v1_light_cast_path_light_amount(
    const Nexus_V1_LightCastPath *path);
uint32_t nexus_v1_light_cast_path_cast_count(
    const Nexus_V1_LightCastPath *path);
uint32_t nexus_v1_light_cast_path_decay_count(
    const Nexus_V1_LightCastPath *path);
uint32_t nexus_v1_light_cast_path_dropped_count(
    const Nexus_V1_LightCastPath *path);
size_t nexus_v1_light_cast_path_active_count(
    const Nexus_V1_LightCastPath *path);

/* ── Save/load proof for the cast-path state ────────────────────────
 *
 * The Firestaff-native `FNXL` (Firestaff Nexus Light) binary format
 * is dedicated to the cast-path state. It does NOT touch the
 * Firestaff `FNXS` world save format from nexus_v1_save.h because:
 *   - The world save is built around a slot manager + atomic
 *     file rename; the cast-path state is ~1.5 KB and lives
 *     inside a single engine struct.
 *   - Adding the cast path to the world save would require
 *     bumping NEXUS_WORLD_SAVE_VERSION and re-locking the format.
 *     A dedicated FNXL format keeps the change strictly additive.
 *
 * FNXL format (little-endian):
 *   uint32_t magic   = 'FNXL' = 0x4C584E46
 *   uint16_t version = 1
 *   uint16_t mode    = 0 (emulate) | 1 (guard)
 *   int32_t  magical_light_amount
 *   uint32_t current_tick
 *   uint32_t cast_counter
 *   uint32_t decay_counter
 *   uint32_t dropped_counter
 *   uint32_t active_count
 *   uint32_t crc32    = zlib CRC-32 of all preceding bytes
 *   [for i = 0..active_count-1:]
 *     uint8_t  type
 *     int16_t  light_power
 *     uint32_t fire_at_tick
 *
 * The format is intentionally narrow: it only persists the cast-path
 * state, not the world or champion pool. Callers that want the full
 * dungeon state should pair FNXL with the existing FNXS world save.
 */

/* FNXL magic: 'F' + ('N'<<8) + ('X'<<16) + ('L'<<24) = 0x4C584E46 */
#define NEXUS_LIGHT_CAST_PATH_SAVE_MAGIC   0x4C584E46U
#define NEXUS_LIGHT_CAST_PATH_SAVE_VERSION 1

/* Return the number of bytes required to serialize the cast path
 * to a buffer. Returns 0 on NULL path. */
size_t nexus_v1_light_cast_path_save_size(const Nexus_V1_LightCastPath *path);

/* Serialize the cast path into `buf`. Returns bytes written, or
 * 0 on error (NULL, buffer too small). The format is described
 * above; a successful round-trip guarantees every state field
 * plus the in-use timeline slots are recovered. */
size_t nexus_v1_light_cast_path_save(const Nexus_V1_LightCastPath *path,
                                      void *buf, size_t bufsize);

/* Deserialize the cast path from `buf` into `path`. Returns
 * NEXUS_SAVE_OK on success, or a negative NEXUS_SAVE_ERR_*
 * code on magic/version/CRC/size failure. On error, `path`
 * is left in a fresh init state so the caller cannot accidentally
 * continue with a partially-populated bundle. */
int nexus_v1_light_cast_path_load(Nexus_V1_LightCastPath *path,
                                    const void *buf, size_t bufsize);

/* Convenience wrapper: returns 1 if a buffer looks like a valid
 * FNXL payload, 0 otherwise. Never reads past the first 4 bytes. */
int nexus_v1_light_cast_path_probe(const void *buf, size_t bufsize);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_LIGHT_OVERFLOW_H */
