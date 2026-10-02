#ifndef THERON_V1_DUNGEON_PROGRESSION_H
#define THERON_V1_DUNGEON_PROGRESSION_H

#include <stdint.h>

/* ══════════════════════════════════════════════════════════════════════
 * Theron V1 Phase 6 — Dungeon Progression
 *
 * Represents Firestaff's seven-dungeon progression model and quest-item
 * state. Retail transition and persistence semantics remain evidence-gated.
 *
 * Established game facts and current model boundary:
 *   - 7 mini-dungeons, 3-8 maps each (1 hub + 2-7 dungeon levels).
 *   - Seven named dungeons and their authentic campaign-entry order are
 *     represented; per-edition unlock transitions require original evidence.
 *   - Original saves occur between dungeons (no in-dungeon save transaction).
 *   - Quest-item identity is backed by authentic Track 02 data; acquisition,
 *     reset, and exit transactions remain independently evidence-gated.
 *
 * Source references:
 *   THQUEST.ASM T000 — title/startup entry
 *   docs/source-lock/theron-original-backup-ram-body-layout-2026-09-23.md
 *       authentic DMS-SG.001 writer/restore body and save boundary
 *   THQUEST.ASM T400 — dungeon bank loading
 *   THQUEST.ASM T520 — party placement / start position
 *   THQUEST.ASM T560 — dungeon loading (header parsing, dungeon_seed)
 *   THQUEST.ASM T800 — source label retained for future verification only
 *   docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md (JP/US disc hashes)
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Dungeon IDs ─────────────────────────────────────────────────── */

/* Firestaff's seven-dungeon model. Retail unlock order and availability
 * transitions remain edition- and evidence-gated. */
typedef enum {
    THERON_DUNGEON_1_AKUTUBA = 1,  /* Shield Defiant */
    THERON_DUNGEON_2_DRATOR  = 2,  /* Taza Boots */
    THERON_DUNGEON_3_FORMIC  = 3,  /* Taza Poleyn */
    THERON_DUNGEON_4_SARMON  = 4,  /* Soulcage */
    THERON_DUNGEON_5_SHADO   = 5,  /* Taza Armour */
    THERON_DUNGEON_6_THIEF   = 6,  /* Tazahelm */
    THERON_DUNGEON_7_DEMON   = 7,  /* Retaliator (final) */
    THERON_DUNGEON_COUNT = 7,
    THERON_DUNGEON_INVALID = 0,
} Theron_DungeonID;

/* Dungeon metadata — read from dungeon header (THQUEST.ASM T560) */
typedef struct {
    Theron_DungeonID  id;
    char              name[32];             /* e.g. "AKUTUBA" */
    uint8_t           level_count;          /* 3–8 maps (incl. hub) */
    uint8_t           quest_item_count;    /* 1 quest item per dungeon */
    uint8_t           quest_item_bit;       /* 1 << (id-1) — tracks collected */
    uint8_t           champion_reset;       /* Firestaff host reset policy */
    uint32_t          dungeon_seed;        /* deterministic RNG seed */
    uint32_t          size_bytes;           /* dungeon data size */
} Theron_DungeonMeta;

/* ── Quest items ─────────────────────────────────────────────────── */

/* Seven quest-item identities from the authenticated Track 02 retrieval
 * messages, represented here in Firestaff progression state.
 * These are unique artifacts that must be retrieved in sequence.
 * Each is tracked as a bit in the quest_items_collected bitmap.
 *
 * Per-dungeon placement, reset transaction and persistence are runtime
 * behaviors; do not infer them solely from these host-side flags. */
typedef enum {
    THERON_QUEST_ITEM_NONE = 0,

    /* Real names from Track 02 retrieval messages (UD 0x27715B-0x277272).
     * Creature-region names at UD 0x2741EF map 1:1 to dungeons 1-7. */
    THERON_QUEST_ITEM_1_SHIELD_DEFIANT = (1 << 0), /* Dungeon 1 — AKUTUBA */
    THERON_QUEST_ITEM_2_TAZA_BOOTS     = (1 << 1), /* Dungeon 2 — DRATOR */
    THERON_QUEST_ITEM_3_TAZA_POLEYN    = (1 << 2), /* Dungeon 3 — FORMIC */
    THERON_QUEST_ITEM_4_SOULCAGE       = (1 << 3), /* Dungeon 4 — SARMON */
    THERON_QUEST_ITEM_5_TAZA_ARMOUR    = (1 << 4), /* Dungeon 5 — SHADO */
    THERON_QUEST_ITEM_6_TAZAHELM       = (1 << 5), /* Dungeon 6 — THIEF */
    THERON_QUEST_ITEM_7_RETALIATOR     = (1 << 6), /* Dungeon 7 — DEMON */

    /* All 7 collected — quest complete */
    THERON_QUEST_ALL_ITEMS = 0x7F,  /* 0b01111111 */

    THERON_QUEST_ITEM_COUNT = 7,
} Theron_QuestItem;

#define THERON_QUEST_ITEM_MASK_FROM_DUNGEON(dungeon_id) \
    (1U << ((dungeon_id) - 1))

/* ── Dungeon state machine ───────────────────────────────────────── */

/* Firestaff host-model state transitions (not asserted as retail behavior):
 *   DUNGEON_STATE_LOCKED → DUNGEON_STATE_AVAILABLE (between-dungeon save restored)
 *   DUNGEON_STATE_AVAILABLE → DUNGEON_STATE_IN_PROGRESS (entered dungeon)
 *   DUNGEON_STATE_IN_PROGRESS → DUNGEON_STATE_COMPLETE (quest item found + exit)
 *   DUNGEON_STATE_COMPLETE → DUNGEON_STATE_NEXT_UNLOCKED (auto on exit)
 *
 * The source-locked retail save boundary is between dungeons; acquisition,
 * reset, unlock and exit transitions still require original evidence. */
typedef enum {
    THERON_DUNGEON_STATE_LOCKED       = 0,
    THERON_DUNGEON_STATE_AVAILABLE    = 1, /* Host model: available */
    THERON_DUNGEON_STATE_IN_PROGRESS  = 2, /* Currently exploring */
    THERON_DUNGEON_STATE_COMPLETE     = 3, /* Quest item collected, can exit */
    THERON_DUNGEON_STATE_COUNT
} Theron_DungeonState;

/* ── Firestaff per-dungeon reset policy ──────────────────────────── */

/* These modes describe the host-side progression model. Do not treat them
 * as original T800 behavior until each reset/persistence field is bound to
 * the authentic writer and restore consumers. */
typedef enum {
    THERON_ITEM_RESET_MODE_NONE     = 0, /* No reset (dungeon 1 start) */
    THERON_ITEM_RESET_MODE_CHAMPION = 1, /* Host clears companion inventories */
    THERON_ITEM_RESET_MODE_PARTY   = 2, /* Host clears the modeled party */
} Theron_ItemResetMode;

/* ── Dungeon progression state ───────────────────────────────────── */

/* Tracks Firestaff progression state. The TQSV host format includes this
 * structure, but retail persistence is established field-by-field only. */
typedef struct {
    /* Current position in the sequence */
    Theron_DungeonID    current_dungeon;
    Theron_DungeonState dungeon_states[THERON_DUNGEON_COUNT]; /* index 0 = INVALID */

    /* Quest-item mask; separate from the campaign-completion byte. */
    uint8_t             quest_items_collected;          /* 7-bit bitmap (0..127) */
    uint8_t             quest_items_in_current_dungeon;  /* items found so far */

    /* Per-dungeon item reset tracking */
    Theron_ItemResetMode item_reset_mode;               /* reset mode for current dungeon */
    uint8_t              item_reset_applied;             /* flag: reset applied this entry */

    /* Firestaff host policy flags; these do not assert retail persistence. */
    uint8_t              champion_stats_persist;
    uint8_t              champion_inv_persist;

    /* Dungeon seeds — one per dungeon (read from dungeon headers).
     * Used for deterministic RNG during dungeon generation/placement. */
    uint32_t             dungeon_seeds[THERON_DUNGEON_COUNT];

    /* Level within current dungeon (1..3) */
    uint8_t              current_level;

    /* Playtime tracking (seconds since dungeon start) */
    uint32_t             dungeon_playtime_seconds;

    /* Source-authenticated final completion only. */
    uint8_t              quest_complete;
    uint8_t              padding[3];
} Theron_DungeonProgression;

/* ── API ─────────────────────────────────────────────────────────── */

/* Initialize dungeon progression to initial state (dungeon 1 unlocked). */
void theron_v1_dungeon_progression_init(Theron_DungeonProgression *prog);

/* Get dungeon metadata by ID. Returns NULL for invalid IDs. */
const Theron_DungeonMeta *theron_v1_dungeon_meta(Theron_DungeonID id);

/* Get the next dungeon ID after completion, or INVALID if all done. */
Theron_DungeonID theron_v1_dungeon_next(Theron_DungeonID current);

/* Advance only from an already-recorded COMPLETE stage state.
 * This function never creates completion state; INVALID means rejected or no
 * selectable stage. The original final-stage completion consumer is unbound. */
Theron_DungeonID theron_v1_dungeon_advance(Theron_DungeonProgression *prog);

/* Compatibility helper that records a provisional host bit only. It does not
 * authenticate a T900 pickup, complete a dungeon, or complete the quest. */
int theron_v1_quest_item_collect(Theron_DungeonProgression *prog,
                                  Theron_QuestItem item);

/* Check if item reset should be applied on dungeon entry.
 * Returns 1 if reset needed (first entry to this dungeon, or retry). */
int theron_v1_item_reset_required(const Theron_DungeonProgression *prog,
                                   Theron_DungeonID dungeon_id);

/* Mark item reset as applied (call after inventory clear on entry). */
void theron_v1_item_reset_mark_applied(Theron_DungeonProgression *prog);

/* Enter a dungeon: set state to IN_PROGRESS, apply item reset if needed. */
int theron_v1_dungeon_enter(Theron_DungeonProgression *prog,
                             Theron_DungeonID dungeon_id);

/* Exit a dungeon: requires dungeon COMPLETE state.
 * On success, advances sequence and returns next dungeon.
 * On failure (not complete), returns INVALID. */
Theron_DungeonID theron_v1_dungeon_exit(Theron_DungeonProgression *prog);

/* Check source-backed final quest completion state. */
int theron_v1_quest_complete(const Theron_DungeonProgression *prog);

/* Get quest item bitmask (for serialization). */
uint8_t theron_v1_quest_item_bitmask(const Theron_DungeonProgression *prog);

/* Restore the provisional quest-item mask and saved current stage. This
 * function does not infer campaign completion from quest-item bits. Project
 * authenticated campaign state separately with the API below. */
void theron_v1_dungeon_progression_restore(Theron_DungeonProgression *prog,
                                            uint8_t quest_items_bitmask,
                                            Theron_DungeonID current,
                                            const uint32_t seeds[THERON_DUNGEON_COUNT]);

/* Project the source-bound $267C campaign-completion byte into dungeon states
 * without changing quest-item bits or claiming final-stage completion. */
void theron_v1_dungeon_progression_apply_campaign_completion(
    Theron_DungeonProgression *prog,
    uint8_t campaign_completion_mask);

/* Human-readable dungeon name lookup. */
const char *theron_v1_dungeon_name(Theron_DungeonID id);
const char *theron_v1_dungeon_state_name(Theron_DungeonState state);

/* Debug/diagnostics print. */
void theron_v1_dungeon_progression_print(const Theron_DungeonProgression *prog);

/* Source evidence citation. */
const char *theron_v1_dungeon_progression_source_evidence(void);

#endif /* THERON_V1_DUNGEON_PROGRESSION_H */
