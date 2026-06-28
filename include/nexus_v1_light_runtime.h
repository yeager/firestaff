/*
 * nexus_v1_light_runtime.h
 * ========================
 *
 * Nexus V1 light-overflow runtime cast hook (M11 wire-in layer).
 *
 * Background
 * ----------
 * `nexus_v1_light_overflow.h` already exposes the BUG0_18 data model
 * + classification hook (`Nexus_V1_LightOverflowKind`). What is still
 * missing is the M11-side runtime: a small object that:
 *
 *   1. Owns one timeline + one light state per active game (so the
 *      classification hook has somewhere to live).
 *   2. Exposes a single apply_cast() entry point the M11 spellcast
 *      dispatch can call from a Light/Torch/Darkness cast.
 *   3. Lets the runtime or a debug HUD poll the live classification
 *      and decide between emulate (default — let BUG0_18 surface)
 *      and guard (clamp casts at the cap to avoid the documented
 *      permanent-light / light-bleed symptom).
 *   4. Round-trips its state across a Firestaff native `FNXS` save
 *      so the MagicalLightAmount + timeline + cast/decay/dropped
 *      counters survive a load without losing classification
 *      correctness.
 *
 * This header is the data-free boundary between the upstream data
 * model (`nexus_v1_light_overflow.h`) and a future M11 frame-path
 * integration. It does NOT touch any real Nexus asset, real Saturn
 * DM.BIN/FONT256.S2D/MNS, or a real `.sav` byte stream. The save
 * blob it produces is meant to be embedded as a typed sub-section
 * inside the existing `FNXS` data section (NEXUS_SAVE_VERSION v2),
 * not to replace the existing save header layout.
 *
 * Source-lock
 * -----------
 *   nexus_v1_light_overflow.h         (data model + classification)
 *   include/nexus_v1_save.h            (FNXS save container)
 *   include/nexus_v1_engine.h          (game_started / world tick)
 *   ReDMCSB TIMELINE.C F0238 line 487  (BUG0_18 silent drop)
 *   ReDMCSB TIMELINE.C F0257 line 1720 (recursive weaker chain)
 *   ReDMCSB MENU.C    :1926-1942       (Light/Torch/Darkness dispatch)
 *   ReDMCSB LOADSAVE.C:2041            (EventMaximumCount = 100 base)
 *   ReDMCSB CHAMPION.C:27              (MagicalLightAmount field)
 */

#ifndef NEXUS_V1_LIGHT_RUNTIME_H
#define NEXUS_V1_LIGHT_RUNTIME_H

#include <stddef.h>
#include <stdint.h>
#include "nexus_v1_light_overflow.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Magic + version for the embedded light-runtime blob ─────────── */

/* 'NGLT' little-endian: 'N' + 'G'<<8 + 'L'<<16 + 'T'<<24. The
 * blob is embedded inside the FNXS data section, never replaces
 * the FNXS header magic. Bumping the major byte makes the blob
 * reject older loads cleanly (FNV-style "old save, fresh runtime"
 * rather than silent corruption). */
#define NEXUS_V1_LIGHT_RUNTIME_MAGIC   0x544C474EU
#define NEXUS_V1_LIGHT_RUNTIME_VERSION 1U

/* On-disk blob layout (little-endian):
 *   uint32 magic              NEXUS_V1_LIGHT_RUNTIME_MAGIC
 *   uint32 version            NEXUS_V1_LIGHT_RUNTIME_VERSION
 *   uint32 guard_rejects      0 = emulate, 1 = guard
 *   uint32 cast_counter
 *   uint32 decay_counter
 *   uint32 dropped_counter
 *   uint32 current_tick
 *   int32  magical_light_amount
 *   uint32 active_count       (count of in-use slots below)
 *   uint32 slot_count         NEXUS_V1_LIGHT_TIMELINE_BASE_CAP (100)
 *   Nexus_V1_LightBlobSlot slot_count (each is type, light_power,
 *                                     fire_at_tick, in_use)
 *
 * The blob total size is:
 *   9 * sizeof(uint32) + sizeof(int32) + slot_count * 16 bytes
 * = 40 + slot_count * 16 = 40 + 100 * 16 = 1640 bytes
 */
typedef struct {
    uint8_t  type;          /* always 70 / C70_EVENT_LIGHT */
    int16_t  light_power;
    uint16_t reserved;      /* explicit pad so 16-byte slot is
                             * stable across layouts */
    uint32_t fire_at_tick;
    uint32_t in_use;
} Nexus_V1_LightBlobSlot;

/* Pre-computed size for callers that need it at compile time.
 * Matches: 9*4 + 4 + 100*16 = 1640 bytes. */
#define NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE \
    (9u * sizeof(uint32_t) + sizeof(int32_t) + \
     ((uint32_t)NEXUS_V1_LIGHT_TIMELINE_BASE_CAP) * \
     sizeof(Nexus_V1_LightBlobSlot))

/* ── Runtime object ────────────────────────────────────────────── */

/* Per-game bounded runtime. One per active Nexus V1 session is
 * enough; the M11 layer instantiates it on game start, ticks it
 * from the world tick, and tears it down on shutdown. */
typedef struct {
    int                       initialized;        /* 1 after init() */
    int                       guard_rejects;      /* 0 = emulate,
                                                    * 1 = guard */
    Nexus_V1_LightState       state;
    Nexus_V1_LightTimeline    timeline;

    /* Runtime audit counters. These are independent of the upstream
     * timeline counters so the M11 layer can attribute blame
     * (e.g. "did the upstream emit N drops or did the runtime
     *  reject M casts?"). */
    uint64_t                  total_casts_applied;
    uint64_t                  total_casts_rejected;
    uint64_t                  total_decays_observed;
    uint64_t                  total_drops_observed;

    /* Last decision cache: the most recent classify() + should_guard()
     * values, so a HUD can poll without re-walking the timeline. */
    Nexus_V1_LightOverflowKind last_classification;
    int                         last_should_guard;

    /* Last cast kind observed (for diagnostics). NEXUS_LIGHT_KIND_LIGHT
     * by default; we treat NEXUS_LIGHT_KIND_LIGHT as 0 so memcmp/zero-
     * init lands on a known value. */
    Nexus_V1_LightKind       last_cast_kind;
    int                      last_cast_power_symbol_ordinal;
} Nexus_V1_LightRuntime;

/* Initialize / shutdown. Both are idempotent and side-effect free
 * on NULL. shutdown() leaves the runtime in a reusable zero state. */
void nexus_v1_light_runtime_init(Nexus_V1_LightRuntime *rt,
                                 int guard_rejects);
void nexus_v1_light_runtime_shutdown(Nexus_V1_LightRuntime *rt);

/* Apply a Light / Torch / Darkness cast through the runtime. This
 * is the single entry point M11 spellcast dispatch should call —
 * it wraps the upstream cast(), applies the audit counters, caches
 * the classification, and returns the LightPower the cast actually
 * applied (0 if rejected).
 *
 * The runtime stays source-faithful: the upstream BUG0_18 silent-
 * drop semantics are preserved when guard_rejects == 0, and the
 * upstream guard-reject path is preserved when guard_rejects == 1.
 */
int nexus_v1_light_runtime_apply_cast(Nexus_V1_LightRuntime *rt,
                                      Nexus_V1_LightKind kind,
                                      int power_symbol_ordinal);

/* Tick the runtime forward by n_ticks. Returns the total number of
 * decay events that fired across the run. */
size_t nexus_v1_light_runtime_tick(Nexus_V1_LightRuntime *rt,
                                   size_t n_ticks);

/* Re-poll the upstream classification. The runtime caches the last
 * result on every apply_cast / tick, but callers can also force a
 * fresh poll (e.g. a debug HUD that wants to re-check between
 * command dispatches). */
Nexus_V1_LightOverflowKind
nexus_v1_light_runtime_classify(const Nexus_V1_LightRuntime *rt);

int nexus_v1_light_runtime_should_guard(const Nexus_V1_LightRuntime *rt);

/* Accessors for the M11 layer / HUD. */
uint64_t nexus_v1_light_runtime_total_casts_applied(const Nexus_V1_LightRuntime *rt);
uint64_t nexus_v1_light_runtime_total_casts_rejected(const Nexus_V1_LightRuntime *rt);
uint64_t nexus_v1_light_runtime_total_decays_observed(const Nexus_V1_LightRuntime *rt);
uint64_t nexus_v1_light_runtime_total_drops_observed(const Nexus_V1_LightRuntime *rt);
int32_t  nexus_v1_light_runtime_magical_light_amount(const Nexus_V1_LightRuntime *rt);

/* Human-readable name for the kind, for diagnostics / log lines.
 * Returns "?" on out-of-range or NULL. */
const char *nexus_v1_light_runtime_kind_name(Nexus_V1_LightKind kind);
const char *nexus_v1_light_runtime_overflow_kind_name(
    Nexus_V1_LightOverflowKind kind);

/* ── Save blob: serialize / deserialize ──────────────────────────── */

/* Returns the canonical blob size (== NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE). */
size_t nexus_v1_light_runtime_blob_size(void);

/* Write the runtime into the user-provided buffer. Returns the
 * number of bytes written, or 0 on NULL/invalid input. The buffer
 * must be at least nexus_v1_light_runtime_blob_size() bytes. */
size_t nexus_v1_light_runtime_serialize(const Nexus_V1_LightRuntime *rt,
                                         void *buf, size_t buf_size);

/* Read a runtime blob into rt. The runtime must be init()'d first
 * so we know the emulate/guard setting before we apply the blob.
 * On success returns 1, on magic/version/size mismatch returns 0
 * and leaves rt untouched. */
int nexus_v1_light_runtime_deserialize(Nexus_V1_LightRuntime *rt,
                                        const void *buf, size_t buf_size);

/* Compute a 32-bit FNV-1a hash of (rt) so callers can include the
 * runtime in a parent state-hash without exposing the internals. */
uint32_t nexus_v1_light_runtime_state_hash(const Nexus_V1_LightRuntime *rt);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_LIGHT_RUNTIME_H */
