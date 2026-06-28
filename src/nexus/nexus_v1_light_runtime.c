/*
 * nexus_v1_light_runtime.c
 * ========================
 *
 * Implementation of the Nexus V1 light-overflow runtime cast hook
 * (M11 wire-in layer) declared in include/nexus_v1_light_runtime.h.
 *
 * The runtime sits between M11 spellcast dispatch and the
 * upstream data-model API in src/nexus/nexus_v1_light_overflow.c.
 *
 * Source-lock:
 *   ReDMCSB TIMELINE.C F0238 line 487   — BUG0_18 silent drop
 *   ReDMCSB TIMELINE.C F0257 line 1720  — recursive weaker chain
 *   ReDMCSB MENU.C    :1926-1942        — Light/Torch/Darkness
 *   ReDMCSB LOADSAVE.C:2041             — EventMaximumCount = 100
 *   ReDMCSB CHAMPION.C:27               — MagicalLightAmount field
 *
 * This module is data-free: no real Nexus asset is opened, no real
 * Saturn DM.BIN/FONT256.S2D/MNS is touched, no real `.sav` byte
 * stream is read. The save blob it produces is meant to be embedded
 * as a typed sub-section inside the existing `FNXS` save format
 * (see include/nexus_v1_save.h, NEXUS_SAVE_VERSION v2).
 */

#include "nexus_v1_light_runtime.h"

#include <string.h>

/* ── Endianness-safe write helpers (little-endian) ────────────────── *
 *
 * The FNXS save format is already little-endian on every platform we
 * target (see src/nexus/nexus_v1_save_load.c). We mirror that choice
 * here so the runtime blob drops into the FNXS data section without
 * a per-platform endian converter.
 */

static void wr_u32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v & 0xFFu);
    p[1] = (uint8_t)((v >> 8)  & 0xFFu);
    p[2] = (uint8_t)((v >> 16) & 0xFFu);
    p[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static void wr_i32(uint8_t *p, int32_t v) {
    wr_u32(p, (uint32_t)v);
}

static uint32_t rd_u32(const uint8_t *p) {
    return  ((uint32_t)p[0])        |
           (((uint32_t)p[1]) << 8) |
           (((uint32_t)p[2]) << 16)|
           (((uint32_t)p[3]) << 24);
}

static int32_t rd_i32(const uint8_t *p) {
    return (int32_t)rd_u32(p);
}

/* ── Lifecycle ───────────────────────────────────────────────────── */

void nexus_v1_light_runtime_init(Nexus_V1_LightRuntime *rt,
                                 int guard_rejects)
{
    if (!rt) return;
    memset(rt, 0, sizeof(*rt));
    rt->guard_rejects = guard_rejects ? 1 : 0;
    nexus_v1_light_state_init(&rt->state);
    nexus_v1_light_timeline_init(&rt->timeline, rt->guard_rejects);
    rt->initialized = 1;
    rt->last_classification = NEXUS_LIGHT_OVERFLOW_NONE;
    rt->last_should_guard = 0;
    rt->last_cast_kind = NEXUS_LIGHT_KIND_LIGHT;
    rt->last_cast_power_symbol_ordinal = -1;
}

void nexus_v1_light_runtime_shutdown(Nexus_V1_LightRuntime *rt) {
    if (!rt) return;
    memset(rt, 0, sizeof(*rt));
    rt->initialized = 0;
}

/* ── Apply cast (the M11 wire-in point) ──────────────────────────── */

int nexus_v1_light_runtime_apply_cast(Nexus_V1_LightRuntime *rt,
                                      Nexus_V1_LightKind kind,
                                      int power_symbol_ordinal)
{
    if (!rt || !rt->initialized) return 0;
    if (rt->guard_rejects && rt->timeline.active_count >=
        NEXUS_V1_LIGHT_TIMELINE_BASE_CAP)
    {
        /* Guard mode rejects BEFORE the upstream cast() runs, so the
         * MagicalLightAmount delta is not applied. We bump the
         * rejection counter and cache the result for HUD probes. */
        rt->total_casts_rejected++;
        rt->last_cast_kind = kind;
        rt->last_cast_power_symbol_ordinal = power_symbol_ordinal;
        rt->last_classification = NEXUS_LIGHT_OVERFLOW_CAST_REJECTED;
        rt->last_should_guard = 1;
        return 0;
    }

    int lp = nexus_v1_light_timeline_cast(&rt->timeline, &rt->state,
                                          kind, power_symbol_ordinal);
    if (lp == 0) {
        /* The upstream returned 0 for one of two reasons:
         *   - upstream spell_power was invalid
         *   - upstream silently dropped via BUG0_18 (emulate mode)
         *   - upstream guard-rejected (which we already pre-checked
         *     above, but the upstream re-checks too)
         *
         * Both cases are non-success from the runtime perspective,
         * so we count them as observed-but-not-applied. The
         * classification will surface CAST_REJECTED if the upstream
         * path produced a guard hit. */
        rt->total_casts_rejected++;
        rt->last_cast_kind = kind;
        rt->last_cast_power_symbol_ordinal = power_symbol_ordinal;
        rt->last_classification =
            nexus_v1_light_overflow_classify(&rt->timeline, &rt->state);
        rt->last_should_guard =
            nexus_v1_light_overflow_should_guard(&rt->timeline, &rt->state);
        return 0;
    }

    rt->total_casts_applied++;
    rt->total_decays_observed = rt->timeline.decay_counter;
    rt->total_drops_observed  = rt->timeline.dropped_counter;
    rt->last_cast_kind = kind;
    rt->last_cast_power_symbol_ordinal = power_symbol_ordinal;
    rt->last_classification =
        nexus_v1_light_overflow_classify(&rt->timeline, &rt->state);
    rt->last_should_guard =
        nexus_v1_light_overflow_should_guard(&rt->timeline, &rt->state);
    return lp;
}

/* ── Tick ────────────────────────────────────────────────────────── */

size_t nexus_v1_light_runtime_tick(Nexus_V1_LightRuntime *rt,
                                   size_t n_ticks)
{
    if (!rt || !rt->initialized) return 0;
    size_t fired = nexus_v1_light_timeline_advance(&rt->timeline,
                                                    &rt->state, n_ticks);
    rt->total_decays_observed = rt->timeline.decay_counter;
    rt->total_drops_observed  = rt->timeline.dropped_counter;
    rt->last_classification =
        nexus_v1_light_overflow_classify(&rt->timeline, &rt->state);
    rt->last_should_guard =
        nexus_v1_light_overflow_should_guard(&rt->timeline, &rt->state);
    return fired;
}

/* ── Classify / should_guard (re-poll wrappers) ──────────────────── */

Nexus_V1_LightOverflowKind
nexus_v1_light_runtime_classify(const Nexus_V1_LightRuntime *rt)
{
    if (!rt || !rt->initialized) return NEXUS_LIGHT_OVERFLOW_NONE;
    return nexus_v1_light_overflow_classify(&rt->timeline, &rt->state);
}

int nexus_v1_light_runtime_should_guard(const Nexus_V1_LightRuntime *rt)
{
    if (!rt || !rt->initialized) return 0;
    return nexus_v1_light_overflow_should_guard(&rt->timeline, &rt->state);
}

/* ── Accessors ───────────────────────────────────────────────────── */

uint64_t nexus_v1_light_runtime_total_casts_applied(const Nexus_V1_LightRuntime *rt) {
    return rt ? rt->total_casts_applied : 0;
}
uint64_t nexus_v1_light_runtime_total_casts_rejected(const Nexus_V1_LightRuntime *rt) {
    return rt ? rt->total_casts_rejected : 0;
}
uint64_t nexus_v1_light_runtime_total_decays_observed(const Nexus_V1_LightRuntime *rt) {
    return rt ? rt->total_decays_observed : 0;
}
uint64_t nexus_v1_light_runtime_total_drops_observed(const Nexus_V1_LightRuntime *rt) {
    return rt ? rt->total_drops_observed : 0;
}
int32_t  nexus_v1_light_runtime_magical_light_amount(const Nexus_V1_LightRuntime *rt) {
    return rt ? rt->state.magical_light_amount : 0;
}

/* ── Diagnostic strings ──────────────────────────────────────────── */

const char *nexus_v1_light_runtime_kind_name(Nexus_V1_LightKind kind) {
    switch (kind) {
        case NEXUS_LIGHT_KIND_LIGHT:    return "LIGHT";
        case NEXUS_LIGHT_KIND_TORCH:    return "TORCH";
        case NEXUS_LIGHT_KIND_DARKNESS: return "DARKNESS";
        default:                        return "?";
    }
}

const char *nexus_v1_light_runtime_overflow_kind_name(
    Nexus_V1_LightOverflowKind kind)
{
    switch (kind) {
        case NEXUS_LIGHT_OVERFLOW_NONE:
            return "NONE";
        case NEXUS_LIGHT_OVERFLOW_TIMELINE_FULL_PERMANENT_LIGHT:
            return "TIMELINE_FULL_PERMANENT_LIGHT";
        case NEXUS_LIGHT_OVERFLOW_LIGHT_BLEED_NEGATIVE:
            return "LIGHT_BLEED_NEGATIVE";
        case NEXUS_LIGHT_OVERFLOW_CAST_REJECTED:
            return "CAST_REJECTED";
        default:
            return "?";
    }
}

/* ── Blob size / serialize / deserialize ─────────────────────────── */

size_t nexus_v1_light_runtime_blob_size(void) {
    return (size_t)NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE;
}

size_t nexus_v1_light_runtime_serialize(const Nexus_V1_LightRuntime *rt,
                                         void *buf, size_t buf_size)
{
    if (!rt || !rt->initialized || !buf) return 0;
    if (buf_size < NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE) return 0;

    uint8_t *p = (uint8_t *)buf;

    /* Header (9 * u32 + 1 * i32 = 40 bytes) */
    wr_u32(p, NEXUS_V1_LIGHT_RUNTIME_MAGIC);   p += 4;
    wr_u32(p, NEXUS_V1_LIGHT_RUNTIME_VERSION); p += 4;
    wr_u32(p, (uint32_t)rt->guard_rejects);     p += 4;
    wr_u32(p, rt->timeline.cast_counter);       p += 4;
    wr_u32(p, rt->timeline.decay_counter);      p += 4;
    wr_u32(p, rt->timeline.dropped_counter);    p += 4;
    wr_u32(p, rt->timeline.current_tick);       p += 4;
    wr_i32(p, rt->state.magical_light_amount);  p += 4;
    wr_u32(p, (uint32_t)rt->timeline.active_count); p += 4;
    wr_u32(p, (uint32_t)NEXUS_V1_LIGHT_TIMELINE_BASE_CAP); p += 4;

    /* Slot table (NEXUS_V1_LIGHT_TIMELINE_BASE_CAP * 16 bytes) */
    for (size_t i = 0; i < NEXUS_V1_LIGHT_TIMELINE_BASE_CAP; ++i) {
        const Nexus_V1_LightEvent *src = &rt->timeline.slots[i];
        wr_u32(p, (uint32_t)src->type);          p += 4; /* widen to 4 bytes */
        wr_i32(p, (int32_t)src->light_power);    p += 4;
        wr_u32(p, src->fire_at_tick);            p += 4;
        wr_u32(p, (uint32_t)src->in_use);        p += 4;
    }

    /* Sanity: we should have written exactly NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE
     * bytes. The cast below is intentional: a regression here would be
     * caught by the nexus_v1_light_runtime_probe invariants. */
    return NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE;
}

int nexus_v1_light_runtime_deserialize(Nexus_V1_LightRuntime *rt,
                                        const void *buf, size_t buf_size)
{
    if (!rt || !rt->initialized || !buf) return 0;
    if (buf_size < NEXUS_V1_LIGHT_RUNTIME_BLOB_SIZE) return 0;

    const uint8_t *p = (const uint8_t *)buf;

    uint32_t magic   = rd_u32(p); p += 4;
    uint32_t version = rd_u32(p); p += 4;
    if (magic != NEXUS_V1_LIGHT_RUNTIME_MAGIC) return 0;
    if (version != NEXUS_V1_LIGHT_RUNTIME_VERSION) return 0;

    uint32_t guard_rejects   = rd_u32(p); p += 4;
    uint32_t cast_counter    = rd_u32(p); p += 4;
    uint32_t decay_counter   = rd_u32(p); p += 4;
    uint32_t dropped_counter = rd_u32(p); p += 4;
    uint32_t current_tick    = rd_u32(p); p += 4;
    int32_t  mla             = rd_i32(p); p += 4;
    uint32_t active_count    = rd_u32(p); p += 4;
    uint32_t slot_count      = rd_u32(p); p += 4;

    if (slot_count != (uint32_t)NEXUS_V1_LIGHT_TIMELINE_BASE_CAP) {
        nexus_v1_light_state_init(&rt->state);
        nexus_v1_light_timeline_init(&rt->timeline, rt->guard_rejects);
        rt->last_classification = NEXUS_LIGHT_OVERFLOW_NONE;
        rt->last_should_guard = 0;
        return 0;
    }

    /* Reset timeline + state to the loaded values. We deliberately
     * preserve the emulate/guard setting that was passed at init()
     * (the blob carries the *original* guard setting as a sanity
     * check, but the runtime owns the active mode flag so callers
     * can flip it on the fly). */
    nexus_v1_light_state_init(&rt->state);
    rt->state.magical_light_amount = mla;
    nexus_v1_light_timeline_init(&rt->timeline, rt->guard_rejects);
    rt->timeline.cast_counter    = cast_counter;
    rt->timeline.decay_counter   = decay_counter;
    rt->timeline.dropped_counter = dropped_counter;
    rt->timeline.current_tick    = current_tick;
    rt->timeline.active_count    = 0;

    size_t loaded_active = 0;
    for (size_t i = 0; i < NEXUS_V1_LIGHT_TIMELINE_BASE_CAP; ++i) {
        uint32_t type_v       = rd_u32(p); p += 4;
        int32_t  light_power  = rd_i32(p); p += 4;
        uint32_t fire_at_tick = rd_u32(p); p += 4;
        uint32_t in_use       = rd_u32(p); p += 4;
        if (in_use) {
            if (loaded_active >= active_count) {
                /* Saved blob claims fewer active slots than are
                 * marked in_use. Treat the blob as corrupt so the
                 * caller can surface a clean error. */
                nexus_v1_light_state_init(&rt->state);
                nexus_v1_light_timeline_init(&rt->timeline, rt->guard_rejects);
                rt->last_classification = NEXUS_LIGHT_OVERFLOW_NONE;
                rt->last_should_guard = 0;
                return 0;
            }
            rt->timeline.slots[i].type = (uint8_t)(type_v & 0xFFu);
            rt->timeline.slots[i].light_power = (int16_t)light_power;
            rt->timeline.slots[i].fire_at_tick = fire_at_tick;
            rt->timeline.slots[i].in_use = 1;
            loaded_active++;
        }
    }
    rt->timeline.active_count = (size_t)loaded_active;

    /* Sanity check: did we walk the active_count we expected? */
    if (loaded_active != (size_t)active_count) {
        nexus_v1_light_state_init(&rt->state);
        nexus_v1_light_timeline_init(&rt->timeline, rt->guard_rejects);
        rt->last_classification = NEXUS_LIGHT_OVERFLOW_NONE;
        rt->last_should_guard = 0;
        return 0;
    }

    /* Sanity check: did the saved blob's guard flag match what the
     * runtime was init()'d with? If a caller init()s the runtime
     * differently than the saved blob, the upstream cast/tick
     * behavior would diverge silently. We surface that as a
     * mismatch so the M11 layer can decide whether to clobber the
     * mode flag or refuse the load. */
    if (((uint32_t)rt->guard_rejects) != guard_rejects) {
        /* The mismatch is real but not catastrophic: the runtime
         * stays init()'d with the caller's mode flag, and we let
         * the caller decide. We surface it through a return-0 +
         * re-init to make the failure obvious to tests / probes. */
        nexus_v1_light_state_init(&rt->state);
        nexus_v1_light_timeline_init(&rt->timeline, rt->guard_rejects);
        rt->last_classification = NEXUS_LIGHT_OVERFLOW_NONE;
        rt->last_should_guard = 0;
        return 0;
    }

    /* Refresh audit counters and classification cache. */
    rt->total_decays_observed = rt->timeline.decay_counter;
    rt->total_drops_observed  = rt->timeline.dropped_counter;
    rt->last_classification =
        nexus_v1_light_overflow_classify(&rt->timeline, &rt->state);
    rt->last_should_guard =
        nexus_v1_light_overflow_should_guard(&rt->timeline, &rt->state);

    return 1;
}

/* ── FNV-1a state hash for parent state-hash aggregation ───────── */

uint32_t nexus_v1_light_runtime_state_hash(const Nexus_V1_LightRuntime *rt) {
    if (!rt) return 0;
    /* FNV-1a 32-bit, identical offset/basis to the upstream probe's
     * hash so the values can be compared in the same hash table. */
    uint32_t h = 0x811c9dc5u;
    const uint8_t *bytes;
    size_t n;
    h ^= (uint32_t)(rt->initialized ? 1u : 0u); h *= 0x01000193u;
    h ^= (uint32_t)(rt->guard_rejects ? 1u : 0u); h *= 0x01000193u;
    h ^= (uint32_t)rt->state.magical_light_amount; h *= 0x01000193u;
    h ^= (uint32_t)rt->timeline.active_count;     h *= 0x01000193u;
    h ^= rt->timeline.cast_counter;                h *= 0x01000193u;
    h ^= rt->timeline.decay_counter;               h *= 0x01000193u;
    h ^= rt->timeline.dropped_counter;             h *= 0x01000193u;
    h ^= rt->timeline.current_tick;                h *= 0x01000193u;

    /* Mix in the slot table. We walk the array so that an in-use
     * flip in any slot changes the hash, but a free slot still
     * contributes (so the hash is sensitive to position). */
    bytes = (const uint8_t *)rt->timeline.slots;
    n = sizeof(rt->timeline.slots);
    for (size_t i = 0; i < n; ++i) {
        h ^= bytes[i];
        h *= 0x01000193u;
    }
    return h;
}
