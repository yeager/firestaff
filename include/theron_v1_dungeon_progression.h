#ifndef THERON_V1_DUNGEON_PROGRESSION_H
#define THERON_V1_DUNGEON_PROGRESSION_H

#include <stdint.h>

/* ══════════════════════════════════════════════════════════════════════
 * Theron V1 Phase 6 — Dungeon Progression
 *
 * Implements the 7-dungeon sequence for Theron's Quest, per-dungeon
 * item reset semantics, and seven-quest-item retrieval goal.
 *
 * Key design constraints (from TQR provenance):
 *   - 7 mini-dungeons, 3 levels each (max).
 *   - Between-dungeon saves only (no in-dungeon save).
 *   - Champion inventory resets each dungeon; Theron's stats/skills persist.
 *   - 7 quest items must be collected across the sequence.
 *   - Dungeon exits only after all quest items in that dungeon are found.
 *
 * Source references:
 *   THQUEST.ASM T000 — title/startup entry
 *   THQUEST.ASM T080 — between-dungeon save/load
 *   THQUEST.ASM T400 — dungeon bank loading
 *   THQUEST.ASM T520 — party placement / start position
 *   THQUEST.ASM T560 — dungeon loading (header parsing, dungeon_seed)
 *   THQUEST.ASM T800 — champion persistence between dungeons
 *   docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md (JP/US disc hashes)
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Dungeon IDs ─────────────────────────────────────────────────── */

/* Theron's Quest 7-dungeon sequence.
 * Order mirrors the original PC Engine game progression.
 * Each dungeon has a distinct theme and 1-2 quest items associated. */
typedef enum {
    THERON_DUNGEON_1_HALL_OF_RECORDS   = 1,  /* Tutorial dungeon — 1 quest item */
    THERON_DUNGEON_2_CRYPT_OF_SHADOWS  = 2,  /* 1 quest item */
    THERON_DUNGEON_3_ABYSS_OF_FLAMES   = 3,  /* 1 quest item */
    THERON_DUNGEON_4_TOMB_OF_WOE       = 4,  /* 1 quest item */
    THERON_DUNGEON_5_VAULT_OF_SECRETS  = 5,  /* 1 quest item */
    THERON_DUNGEON_6_CASTLE_OF_FATE    = 6,  /* 1 quest item */
    THERON_DUNGEON_7_TOWER_OF_EPILOGUE = 7,  /* 1 quest item (final) */
    THERON_DUNGEON_COUNT = 7,
    THERON_DUNGEON_INVALID = 0,
} Theron_DungeonID;

/* Dungeon metadata — read from dungeon header (THQUEST.ASM T560) */
typedef struct {
    Theron_DungeonID  id;
    char              name[32];             /* e.g. "Hall of Records" */
    uint8_t           level_count;          /* 1–3 levels */
    uint8_t           quest_item_count;    /* 1 quest item per dungeon */
    uint8_t           quest_item_bit;       /* 1 << (id-1) — tracks collected */
    uint8_t           champion_reset;       /* 1 = reset inventory; 0 = keep */
    uint32_t          dungeon_seed;        /* deterministic RNG seed */
    uint32_t          size_bytes;           /* dungeon data size */
} Theron_DungeonMeta;

/* ── Quest items ─────────────────────────────────────────────────── */

/* Seven quest items — one per dungeon.
 * These are unique artifacts that must be retrieved in sequence.
 * Each is tracked as a bit in the quest_items_collected bitmap.
 *
 * TQR design: item is placed in the final room / treasure vault
 * of each dungeon. Champions reset per dungeon but Theron persists
 * with stats, skills, and accumulated quest items. */
typedef enum {
    THERON_QUEST_ITEM_NONE = 0,

    /* Dungeon 1 — Hall of Records */
    THERON_QUEST_ITEM_1_SACRED_AMPLIFIER = (1 << 0), /* Bit 0 */

    /* Dungeon 2 — Crypt of Shadows */
    THERON_QUEST_ITEM_2_SHADOW_KEY = (1 << 1),        /* Bit 1 */


    /* Dungeon 3 — Abyss of Flames */
    THERON_QUEST_ITEM_3_FLAME_ORBS = (1 << 2),       /* Bit 2 */

    /* Dungeon 4 — Tomb of Woe */
    THERON_QUEST_ITEM_4_STONE_SIGIL = (1 << 3),      /* Bit 3 */

    /* Dungeon 5 — Vault of Secrets */
    THERON_QUEST_ITEM_5_WAYWARD_RIBBON = (1 << 4),    /* Bit 4 */

    /* Dungeon 6 — Castle of Fate */
    THERON_QUEST_ITEM_6_DESTINYS_THREAD = (1 << 5),   /* Bit 5 */

    /* Dungeon 7 — Tower of Epilogue */
    THERON_QUEST_ITEM_7_COSMIC_SHARD = (1 << 6),      /* Bit 6 */

    /* All 7 collected — quest complete */
    THERON_QUEST_ALL_ITEMS = 0x7F,  /* 0b01111111 */

    THERON_QUEST_ITEM_COUNT = 7,
} Theron_QuestItem;

#define THERON_QUEST_ITEM_MASK_FROM_DUNGEON(dungeon_id) \
    (1U << ((dungeon_id) - 1))

/* ── Dungeon state machine ───────────────────────────────────────── */

/* Each dungeon has up to 3 sub-levels. State transitions:
 *   DUNGEON_STATE_LOCKED → DUNGEON_STATE_AVAILABLE (between-dungeon save restored)
 *   DUNGEON_STATE_AVAILABLE → DUNGEON_STATE_IN_PROGRESS (entered dungeon)
 *   DUNGEON_STATE_IN_PROGRESS → DUNGEON_STATE_COMPLETE (quest item found + exit)
 *   DUNGEON_STATE_COMPLETE → DUNGEON_STATE_NEXT_UNLOCKED (auto on exit)
 *
 * No mid-dungeon saves; player must complete dungeon or forfeit progress. */
typedef enum {
    THERON_DUNGEON_STATE_LOCKED       = 0,
    THERON_DUNGEON_STATE_AVAILABLE    = 1, /* Between-dungeon save slot ready */
    THERON_DUNGEON_STATE_IN_PROGRESS  = 2, /* Currently exploring */
    THERON_DUNGEON_STATE_COMPLETE     = 3, /* Quest item collected, can exit */
    THERON_DUNGEON_STATE_COUNT
} Theron_DungeonState;

/* ── Per-dungeon item reset semantics ───────────────────────────── */

/* TQR design: per-dungeon item reset.
 * On dungeon entry (THERON_DUNGEON_STATE_LOCKED → IN_PROGRESS):
 *   - Champion inventories are CLEARED (all items removed).
 *   - Champion gold is KEPT (gold persists between dungeons).
 *   - Champion stats/skills persist (no stat reset).
 *   - Theron (party leader) keeps equipped items.
 *   - Floor/decoration items in dungeon are regenerated.
 *
 * Source: THQUEST.ASM T800 — champion persistence + inventory reset logic */
typedef enum {
    THERON_ITEM_RESET_MODE_NONE     = 0, /* No reset (dungeon 1 start) */
    THERON_ITEM_RESET_MODE_CHAMPION = 1, /* Clear champion inventories */
    THERON_ITEM_RESET_MODE_PARTY   = 2, /* Clear all (Theron too — rare) */
} Theron_ItemResetMode;

/* ── Dungeon progression state ───────────────────────────────────── */

/* Tracks the overall 7-dungeon sequence progress.
 * Persisted in between-dungeon saves (saves/theron/slotN.tqsv). */
typedef struct {
    /* Current position in the sequence */
    Theron_DungeonID    current_dungeon;
    Theron_DungeonState dungeon_states[THERON_DUNGEON_COUNT]; /* index 0 = INVALID */

    /* Quest items collected — bitmask of THERON_QUEST_ITEM_* flags */
    uint8_t             quest_items_collected;          /* 7-bit bitmap (0..127) */
    uint8_t             quest_items_in_current_dungeon;  /* items found so far */

    /* Per-dungeon item reset tracking */
    Theron_ItemResetMode item_reset_mode;               /* reset mode for current dungeon */
    uint8_t              item_reset_applied;             /* flag: reset applied this entry */

    /* Champion persistence (per dungeon):
     * champion_stats_persist = 1 (Theron and champions keep stat growth)
     * champion_inv_persist   = 0 (inventories reset each dungeon) */
    uint8_t              champion_stats_persist;  /* always 1 for TQR */
    uint8_t              champion_inv_persist;    /* always 0 (reset per design) */

    /* Dungeon seeds — one per dungeon (read from dungeon headers).
     * Used for deterministic RNG during dungeon generation/placement. */
    uint32_t             dungeon_seeds[THERON_DUNGEON_COUNT];

    /* Level within current dungeon (1..3) */
    uint8_t              current_level;

    /* Playtime tracking (seconds since dungeon start) */
    uint32_t             dungeon_playtime_seconds;

    /* Quest complete flag */
    uint8_t              quest_complete; /* 1 when all 7 items collected */
    uint8_t              padding[3];
} Theron_DungeonProgression;

/* ── API ─────────────────────────────────────────────────────────── */

/* Initialize dungeon progression to initial state (dungeon 1 unlocked). */
void theron_v1_dungeon_progression_init(Theron_DungeonProgression *prog);

/* Get dungeon metadata by ID. Returns NULL for invalid IDs. */
const Theron_DungeonMeta *theron_v1_dungeon_meta(Theron_DungeonID id);

/* Get the next dungeon ID after completion, or INVALID if all done. */
Theron_DungeonID theron_v1_dungeon_next(Theron_DungeonID current);

/* Advance dungeon sequence after quest item collected and exit triggered.
 * Updates dungeon_states[current] → COMPLETE, next → AVAILABLE.
 * Returns next dungeon ID, or INVALID if game complete. */
Theron_DungeonID theron_v1_dungeon_advance(Theron_DungeonProgression *prog);

/* Collect a quest item in the current dungeon.
 * Sets the corresponding bit in quest_items_collected.
 * If all items in current dungeon found, marks dungeon COMPLETE. */
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

/* Check if all 7 quest items collected. */
int theron_v1_quest_complete(const Theron_DungeonProgression *prog);

/* Get quest item bitmask (for serialization). */
uint8_t theron_v1_quest_item_bitmask(const Theron_DungeonProgression *prog);

/* Restore progression state from saved bitmask + dungeon state.
 * Used when loading a between-dungeon save. */
void theron_v1_dungeon_progression_restore(Theron_DungeonProgression *prog,
                                            uint8_t quest_items_bitmask,
                                            Theron_DungeonID current,
                                            const uint32_t seeds[THERON_DUNGEON_COUNT]);

/* Human-readable dungeon name lookup. */
const char *theron_v1_dungeon_name(Theron_DungeonID id);
const char *theron_v1_dungeon_state_name(Theron_DungeonState state);

/* Debug/diagnostics print. */
void theron_v1_dungeon_progression_print(const Theron_DungeonProgression *prog);

/* Source evidence citation. */
const char *theron_v1_dungeon_progression_source_evidence(void);

#endif /* THERON_V1_DUNGEON_PROGRESSION_H */