/*
 * theron_v1_dungeon_progression.c — Theron's Quest V1 Phase 6
 * Dungeon progression: 7-dungeon sequence, per-dungeon item reset,
 * between-dungeon save, seven-quest-item retrieval goal.
 *
 * Phase 6 source-lock (2026-05-27)
 *
 * Source references:
 *   THQUEST.ASM T000  — title/startup entry
 *   THQUEST.ASM T080  — between-dungeon save/load (no in-dungeon)
 *   THQUEST.ASM T400  — dungeon bank loading
 *   THQUEST.ASM T520  — party placement / start position
 *   THQUEST.ASM T560  — dungeon loading (header parsing, dungeon_seed)
 *   THQUEST.ASM T800  — champion persistence + inventory reset per dungeon
 *   docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 *
 * Key design constraints:
 *   - 7 mini-dungeons, 3 levels each (max 21 dungeon maps).
 *   - Between-dungeon saves only (in_dungeon_save_allowed = 0).
 *   - Champion inventory resets each dungeon; Theron stats/skills persist.
 *   - 7 quest items collected across sequence (one per dungeon).
 *   - Dungeon exits only after quest item is found.
 */

#include "theron_v1_dungeon_progression.h"
#include <stdio.h>
#include <string.h>

/* ── Static dungeon metadata table ─────────────────────────────── */

/* Each entry maps one dungeon ID to its metadata.
 * name, level_count, quest_item_bit are from TQR data analysis.
 * dungeon_seed is set from dungeon header on load (Phase 2).
 * For Phase 6, default seeds are placeholders to be replaced in Phase 2. */
static const Theron_DungeonMeta g_dungeon_table[THERON_DUNGEON_COUNT] = {
    /* Dungeon 1 — Hall of Records (tutorial) */
    [THERON_DUNGEON_1_HALL_OF_RECORDS - 1] = {
        .id                = THERON_DUNGEON_1_HALL_OF_RECORDS,
        .name              = "Hall of Records",
        .level_count       = 2,
        .quest_item_count  = 1,
        .quest_item_bit    = (1 << 0),  /* Bit 0 */
        .champion_reset    = 1,
        .dungeon_seed      = 313,       /* default; Phase 2 replaces from header */
        .size_bytes        = 0,         /* set at load time */
    },
    /* Dungeon 2 — Crypt of Shadows */
    [THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1] = {
        .id                = THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
        .name              = "Crypt of Shadows",
        .level_count       = 2,
        .quest_item_count  = 1,
        .quest_item_bit    = (1 << 1),  /* Bit 1 */
        .champion_reset    = 1,
        .dungeon_seed      = 414,
        .size_bytes        = 0,
    },
    /* Dungeon 3 — Abyss of Flames */
    [THERON_DUNGEON_3_ABYSS_OF_FLAMES - 1] = {
        .id                = THERON_DUNGEON_3_ABYSS_OF_FLAMES,
        .name              = "Abyss of Flames",
        .level_count       = 3,
        .quest_item_count  = 1,
        .quest_item_bit    = (1 << 2),  /* Bit 2 */
        .champion_reset    = 1,
        .dungeon_seed      = 527,
        .size_bytes        = 0,
    },
    /* Dungeon 4 — Tomb of Woe */
    [THERON_DUNGEON_4_TOMB_OF_WOE - 1] = {
        .id                = THERON_DUNGEON_4_TOMB_OF_WOE,
        .name              = "Tomb of Woe",
        .level_count       = 3,
        .quest_item_count  = 1,
        .quest_item_bit    = (1 << 3),  /* Bit 3 */
        .champion_reset    = 1,
        .dungeon_seed      = 632,
        .size_bytes        = 0,
    },
    /* Dungeon 5 — Vault of Secrets */
    [THERON_DUNGEON_5_VAULT_OF_SECRETS - 1] = {
        .id                = THERON_DUNGEON_5_VAULT_OF_SECRETS,
        .name              = "Vault of Secrets",
        .level_count       = 2,
        .quest_item_count  = 1,
        .quest_item_bit    = (1 << 4),  /* Bit 4 */
        .champion_reset    = 1,
        .dungeon_seed      = 749,
        .size_bytes        = 0,
    },
    /* Dungeon 6 — Castle of Fate */
    [THERON_DUNGEON_6_CASTLE_OF_FATE - 1] = {
        .id                = THERON_DUNGEON_6_CASTLE_OF_FATE,
        .name              = "Castle of Fate",
        .level_count       = 3,
        .quest_item_count  = 1,
        .quest_item_bit    = (1 << 5),  /* Bit 5 */
        .champion_reset    = 1,
        .dungeon_seed      = 856,
        .size_bytes        = 0,
    },
    /* Dungeon 7 — Tower of Epilogue (final) */
    [THERON_DUNGEON_7_TOWER_OF_EPILOGUE - 1] = {
        .id                = THERON_DUNGEON_7_TOWER_OF_EPILOGUE,
        .name              = "Tower of Epilogue",
        .level_count       = 3,
        .quest_item_count  = 1,
        .quest_item_bit    = (1 << 6),  /* Bit 6 */
        .champion_reset    = 1,
        .dungeon_seed      = 967,
        .size_bytes        = 0,
    },
};

/* Quest item names (indexed by dungeon_id - 1) */
static const char *const g_quest_item_names[THERON_DUNGEON_COUNT] = {
    "Sacred Amplifier",
    "Shadow Key",
    "Flame Orbs",
    "Stone Sigil",
    "Wayward Ribbon",
    "Destiny's Thread",
    "Cosmic Shard",
};

/* Dungeon short codes (for UI/diagnostics) */
static const char *const g_dungeon_short[THERON_DUNGEON_COUNT] = {
    "HoR",  /* Hall of Records */
    "CoS",  /* Crypt of Shadows */
    "AoF",  /* Abyss of Flames */
    "ToW",  /* Tomb of Woe */
    "VoS",  /* Vault of Secrets */
    "CoF",  /* Castle of Fate */
    "ToE",  /* Tower of Epilogue */
};

/* ── API implementation ─────────────────────────────────────────── */

void theron_v1_dungeon_progression_init(Theron_DungeonProgression *prog) {
    if (!prog) return;
    memset(prog, 0, sizeof(*prog));

    /* Initial state: dungeon 1 available, rest locked */
    prog->current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    for (int i = 0; i < THERON_DUNGEON_COUNT; i++) {
        prog->dungeon_states[i] = THERON_DUNGEON_STATE_LOCKED;
    }
    prog->dungeon_states[0] = THERON_DUNGEON_STATE_AVAILABLE;

    prog->quest_items_collected        = 0;
    prog->quest_items_in_current_dungeon = 0;
    prog->item_reset_mode              = THERON_ITEM_RESET_MODE_CHAMPION;
    prog->item_reset_applied           = 0;
    prog->champion_stats_persist       = 1;   /* TQR design: stats persist */
    prog->champion_inv_persist         = 0;   /* TQR design: inventories reset */
    prog->current_level                 = 1;
    prog->dungeon_playtime_seconds     = 0;
    prog->quest_complete               = 0;

    /* Default dungeon seeds (replaced in Phase 2 from header) */
    for (int i = 0; i < THERON_DUNGEON_COUNT; i++) {
        prog->dungeon_seeds[i] = g_dungeon_table[i].dungeon_seed;
    }
}

const Theron_DungeonMeta *theron_v1_dungeon_meta(Theron_DungeonID id) {
    if (id < 1 || id > THERON_DUNGEON_COUNT) return NULL;
    return &g_dungeon_table[id - 1];
}

Theron_DungeonID theron_v1_dungeon_next(Theron_DungeonID current) {
    if (current < 1 || current >= THERON_DUNGEON_COUNT) {
        return THERON_DUNGEON_INVALID;
    }
    return (Theron_DungeonID)(current + 1);
}

Theron_DungeonID theron_v1_dungeon_advance(Theron_DungeonProgression *prog) {
    if (!prog) return THERON_DUNGEON_INVALID;

    Theron_DungeonID current = prog->current_dungeon;

    /* Mark current as complete */
    if (current >= 1 && current <= THERON_DUNGEON_COUNT) {
        prog->dungeon_states[current - 1] = THERON_DUNGEON_STATE_COMPLETE;
    }

    /* Advance to next */
    Theron_DungeonID next = theron_v1_dungeon_next(current);
    if (next == THERON_DUNGEON_INVALID) {
        /* All dungeons complete — quest done */
        prog->quest_complete = 1;
        return THERON_DUNGEON_INVALID;
    }

    /* Set next as available (between-dungeon state) */
    prog->dungeon_states[next - 1] = THERON_DUNGEON_STATE_AVAILABLE;
    prog->current_dungeon = next;
    prog->quest_items_in_current_dungeon = 0;
    prog->item_reset_applied = 0;
    prog->current_level = 1;
    prog->dungeon_playtime_seconds = 0;

    /* Reset item reset mode for new dungeon */
    const Theron_DungeonMeta *meta = theron_v1_dungeon_meta(next);
    if (meta) {
        prog->item_reset_mode = meta->champion_reset
            ? THERON_ITEM_RESET_MODE_CHAMPION
            : THERON_ITEM_RESET_MODE_NONE;
    }

    return next;
}

int theron_v1_quest_item_collect(Theron_DungeonProgression *prog,
                                  Theron_QuestItem item) {
    if (!prog) return -1;

    /* Determine which bit this item corresponds to */
    int bit = -1;
    for (int i = 0; i < THERON_QUEST_ITEM_COUNT; i++) {
        if (item == (Theron_QuestItem)(1 << i)) {
            bit = i;
            break;
        }
    }
    if (bit < 0) return -1; /* invalid item */

    /* Check dungeon consistency: item bit must match current dungeon */
    Theron_DungeonID current = prog->current_dungeon;
    uint8_t expected_bit = THERON_QUEST_ITEM_MASK_FROM_DUNGEON(current);
    if ((expected_bit & (uint8_t)item) == 0) {
        /* Item doesn't belong to current dungeon — reject */
        return -2;
    }

    /* Already collected? */
    if (prog->quest_items_collected & (uint8_t)item) {
        return 0; /* already collected, no-op */
    }

    /* Collect it */
    prog->quest_items_collected |= (uint8_t)item;
    prog->quest_items_in_current_dungeon++;

    /* If all quest items in current dungeon found, mark complete */
    const Theron_DungeonMeta *meta = theron_v1_dungeon_meta(current);
    if (meta && prog->quest_items_in_current_dungeon >= meta->quest_item_count) {
        prog->dungeon_states[current - 1] = THERON_DUNGEON_STATE_COMPLETE;
    }

    /* Check quest completion */
    if (prog->quest_items_collected == THERON_QUEST_ALL_ITEMS) {
        prog->quest_complete = 1;
    }

    return 1; /* newly collected */
}

int theron_v1_item_reset_required(const Theron_DungeonProgression *prog,
                                   Theron_DungeonID dungeon_id) {
    if (!prog || dungeon_id < 1 || (int) dungeon_id > THERON_DUNGEON_COUNT) {
        return 0;
    }
    /* Reset required if dungeon is available or in-progress and not yet applied */
    int state = prog->dungeon_states[dungeon_id - 1];
    if (state == THERON_DUNGEON_STATE_AVAILABLE ||
        state == THERON_DUNGEON_STATE_IN_PROGRESS) {
        return !prog->item_reset_applied;
    }
    return 0;
}

void theron_v1_item_reset_mark_applied(Theron_DungeonProgression *prog) {
    if (!prog) return;
    prog->item_reset_applied = 1;
}

int theron_v1_dungeon_enter(Theron_DungeonProgression *prog,
                             Theron_DungeonID dungeon_id) {
    if (!prog) return -1;
    if (dungeon_id < 1 || (int) dungeon_id > THERON_DUNGEON_COUNT) return -1;

    Theron_DungeonState state = prog->dungeon_states[dungeon_id - 1];

    /* Can only enter AVAILABLE or already IN_PROGRESS (retry) dungeons */
    if (state == THERON_DUNGEON_STATE_LOCKED) return -2;
    if (state == THERON_DUNGEON_STATE_COMPLETE) return -3;

    /* Set in-progress */
    prog->dungeon_states[dungeon_id - 1] = THERON_DUNGEON_STATE_IN_PROGRESS;
    prog->current_dungeon = dungeon_id;
    prog->current_level = 1;

    /* Reset applied flag will be cleared when inventory clear is called.
     * (item_reset_required returns 1 until mark_applied is called) */

    return 0;
}

Theron_DungeonID theron_v1_dungeon_exit(Theron_DungeonProgression *prog) {
    if (!prog) return THERON_DUNGEON_INVALID;

    Theron_DungeonID current = prog->current_dungeon;
    if (current < 1 || (int) current > THERON_DUNGEON_COUNT) {
        return THERON_DUNGEON_INVALID;
    }

    /* Must be COMPLETE to exit */
    if (prog->dungeon_states[current - 1] != THERON_DUNGEON_STATE_COMPLETE) {
        return THERON_DUNGEON_INVALID; /* cannot exit — quest item not yet found */
    }

    return theron_v1_dungeon_advance(prog);
}

int theron_v1_quest_complete(const Theron_DungeonProgression *prog) {
    return prog && prog->quest_complete;
}

uint8_t theron_v1_quest_item_bitmask(const Theron_DungeonProgression *prog) {
    return prog ? prog->quest_items_collected : 0;
}

void theron_v1_dungeon_progression_restore(Theron_DungeonProgression *prog,
                                            uint8_t quest_items_bitmask,
                                            Theron_DungeonID current,
                                            const uint32_t seeds[THERON_DUNGEON_COUNT]) {
    if (!prog) return;

    /* Reconstruct progression from save data.
     * We know which items were collected (bitmask) and the current dungeon.
     * Dungeon states are inferred from item bitmask: if item i is collected,
     * dungeon i+1 is COMPLETE; next is AVAILABLE; rest LOCKED. */
    memset(prog, 0, sizeof(*prog));

    prog->quest_items_collected = quest_items_bitmask;
    prog->current_dungeon = current;

    /* Infer dungeon states from item bitmask */
    for (int i = 0; i < THERON_DUNGEON_COUNT; i++) {
        uint8_t item_bit = (uint8_t)(1 << i);
        if ((quest_items_bitmask & item_bit) != 0) {
            prog->dungeon_states[i] = THERON_DUNGEON_STATE_COMPLETE;
        } else if (i + 1 == (int) current) {
            prog->dungeon_states[i] = THERON_DUNGEON_STATE_AVAILABLE;
        } else {
            prog->dungeon_states[i] = THERON_DUNGEON_STATE_LOCKED;
        }
    }

    prog->item_reset_applied = 0;
    prog->champion_stats_persist = 1;
    prog->champion_inv_persist = 0;
    prog->current_level = 1;
    prog->dungeon_playtime_seconds = 0;
    prog->quest_items_in_current_dungeon = 0;
    prog->quest_complete = (quest_items_bitmask == THERON_QUEST_ALL_ITEMS);

    if (seeds) {
        for (int i = 0; i < THERON_DUNGEON_COUNT; i++) {
            prog->dungeon_seeds[i] = seeds[i];
        }
    }
}

const char *theron_v1_dungeon_name(Theron_DungeonID id) {
    if (id < 1 || id > THERON_DUNGEON_COUNT) return "(invalid)";
    return g_dungeon_table[id - 1].name;
}

const char *theron_v1_dungeon_state_name(Theron_DungeonState state) {
    static const char *const names[] = {
        [THERON_DUNGEON_STATE_LOCKED]      = "LOCKED",
        [THERON_DUNGEON_STATE_AVAILABLE]   = "AVAILABLE",
        [THERON_DUNGEON_STATE_IN_PROGRESS]  = "IN_PROGRESS",
        [THERON_DUNGEON_STATE_COMPLETE]     = "COMPLETE",
    };
    if ((unsigned) state >= THERON_DUNGEON_STATE_COUNT) return "(unknown)";
    return names[state];
}

/* ── Debug / diagnostics ─────────────────────────────────────────── */

void theron_v1_dungeon_progression_print(const Theron_DungeonProgression *prog) {
    if (!prog) {
        printf("Theron V1 dungeon progression: NULL\n");
        return;
    }

    printf("=== Theron V1 Dungeon Progression ===\n");
    printf("Current dungeon:  %s (%u)\n",
           theron_v1_dungeon_name(prog->current_dungeon),
           prog->current_dungeon);
    printf("Level:           %u\n", prog->current_level);
    printf("Quest items:     0x%02X / 0x%02X (%u/%u collected)\n",
           prog->quest_items_collected,
           THERON_QUEST_ALL_ITEMS,
           __builtin_popcount(prog->quest_items_collected),
           THERON_QUEST_ITEM_COUNT);
    printf("Quest complete:  %s\n", prog->quest_complete ? "YES" : "NO");
    printf("Item reset:       mode=%u applied=%u\n",
           prog->item_reset_mode, prog->item_reset_applied);
    printf("Champ persistence: stats=%u inv=%u\n",
           prog->champion_stats_persist, prog->champion_inv_persist);
    printf("\n");
    printf("Dungeon states:\n");
    for (int i = 0; i < THERON_DUNGEON_COUNT; i++) {
        Theron_DungeonID id = (Theron_DungeonID)(i + 1);
        printf("  [%u] %-22s %s  seed=%u\n",
               id,
               theron_v1_dungeon_name(id),
               theron_v1_dungeon_state_name(prog->dungeon_states[i]),
               prog->dungeon_seeds[i]);
    }
    printf("\n");
    printf("Quest items per dungeon:\n");
    for (int i = 0; i < THERON_DUNGEON_COUNT; i++) {
        Theron_DungeonID id = (Theron_DungeonID)(i + 1);
        uint8_t bit = (uint8_t)(1 << i);
        int collected = (prog->quest_items_collected & bit) != 0;
        printf("  [%u] %-22s %s\n",
               id,
               g_quest_item_names[i],
               collected ? "COLLECTED" : "pending");
    }
    printf("=======================================\n");
}

const char *theron_v1_dungeon_progression_source_evidence(void) {
    return
        "Theron V1 Dungeon Progression — Phase 6 source-lock\n"
        "THQUEST.ASM T000  — title/startup entry\n"
        "THQUEST.ASM T080  — between-dungeon save/load (no in-dungeon saves)\n"
        "THQUEST.ASM T400  — dungeon bank loading (HuCard ROM mapping)\n"
        "THQUEST.ASM T520  — party placement / start position\n"
        "THQUEST.ASM T560  — dungeon loading (header parsing, dungeon_seed)\n"
        "THQUEST.ASM T800  — champion persistence between dungeons\n"
        "TQR: 7 mini-dungeons, 3 levels each max; champion inv reset per dungeon;\n"
        "  Theron persists with stats/skills; 7 quest items one per dungeon;\n"
        "  no in-dungeon saves; between-dungeon saves only (saves/theron/)\n"
        "Phase 0 provenance: docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md\n"
        "  JP MD5: b7afb338ad31be1025b53f9aff12d73a\n"
        "  US MD5: f23601102138f87c33025877767ebf76";
}