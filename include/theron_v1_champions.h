#ifndef THERON_V1_CHAMPIONS_H
#define THERON_V1_CHAMPIONS_H

#include <stdint.h>
#include <stddef.h>

/* ══════════════════════════════════════════════════════════════════════
 * Theron V1 Phase 3 — Champion State & Persistence
 *
 * Champion structs for Theron's Quest: Theron + 3 companions.
 * Key TQR persistence rule:
 *   - Theron  (slot 0): stats AND skills AND equipped items PERSIST
 *                       across dungeons.
 *   - Companions (slots 1-3): stats/skills persist; inventories RESET
 *                             each dungeon; gold persists.
 *
 * Source references:
 *   THQUEST.ASM T520  — party placement / start position
 *   THQUEST.ASM T800  — champion persistence + inventory reset per dungeon
 *   THQUEST.ASM T560  — dungeon loading (header + dungeon_seed)
 * ══════════════════════════════════════════════════════════════════════ */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Champion slot indices ────────────────────────────────────────── */
#define THERON_CHAMPION_SLOT_THERON     0   /* Theron — persistent across dungeons */
#define THERON_CHAMPION_SLOT_COMPANION_1 1
#define THERON_CHAMPION_SLOT_COMPANION_2 2
#define THERON_CHAMPION_SLOT_COMPANION_3 3
#define THERON_MAX_CHAMPIONS            4
#define THERON_MAX_PARTY                4

/* ── TQR "light" item set ─────────────────────────────────────────── */
/* Theron's Quest uses only a subset of DM1 items.
 * First-party items are the quest items; the rest are supplies.
 * Source: THQUEST.ASM T560 item table; TQR is a "light" version. */
#define THERON_ITEM_NONE          0
#define THERON_ITEM_POTION        1   /* healing potion (common) */
#define THERON_ITEM_ANTIDOTE      2   /* cure poison */
#define THERON_ITEM_PHOENIX_DOWN  3   /* revival at temple */
#define THERON_ITEM_SCROLL       4   /* generic scroll */
#define THERON_ITEM_FOOD         5
#define THERON_ITEM_WATER        6
#define THERON_ITEM_KEY          7   /* dungeon key */
#define THERON_ITEM_CHEST       8   /* treasure chest */
#define THERON_ITEM_WEAPON      9
#define THERON_ITEM_ARMOR       10
#define THERON_ITEM_SHIELD       11
#define THERON_ITEM_HELM         12
#define THERON_ITEM_BOOTS        13
#define THERON_ITEM_AMULET       14
#define THERON_ITEM_GAUNTLETS    15
#define THERON_ITEM_GOLD        127 /* pseudo-item for gold tracking */
#define THERON_ITEM_QUEST_BASE   128 /* quest items start here */
#define THERON_ITEM_QUEST_1      129 /* Sacred Amplifier  — Hall of Records */
#define THERON_ITEM_QUEST_2      130 /* Shadow Key        — Crypt of Shadows */
#define THERON_ITEM_QUEST_3      131 /* Flame Orbs        — Abyss of Flames */
#define THERON_ITEM_QUEST_4      132 /* Stone Sigil       — Tomb of Woe */
#define THERON_ITEM_QUEST_5      133 /* Wayward Ribbon    — Vault of Secrets */
#define THERON_ITEM_QUEST_6      134 /* Destiny's Thread  — Castle of Fate */
#define THERON_ITEM_QUEST_7      135 /* Cosmic Shard      — Tower of Epilogue */

#define THERON_IS_QUEST_ITEM(id) ((id) >= THERON_ITEM_QUEST_BASE)
#define THERON_INVENTORY_SLOTS   30   /* same as DM1 champion inventory */

/* ── Champion classes ─────────────────────────────────────────────── */
typedef enum {
    THERON_CLASS_FIGHTER = 0,
    THERON_CLASS_NINJA   = 1,
    THERON_CLASS_PRIEST  = 2,
    THERON_CLASS_WIZARD  = 3,
    THERON_CLASS_COUNT
} Theron_ChampionClass;

/* ── Wound bitmasks ─────────────────────────────────────────────── */
#define THERON_WOUND_HEAD    (1 << 0)
#define THERON_WOUND_BODY    (1 << 1)
#define THERON_WOUND_ARMS    (1 << 2)
#define THERON_WOUND_LEGS    (1 << 3)

/* ── Attribute flags ────────────────────────────────────────────── */
#define THERON_ATTR_LOAD_CHANGED  (1 << 8)
#define THERON_ATTR_STATISTICS   (1 << 9)
#define THERON_ATTR_DEAD         (1 << 10)
#define THERON_ATTR_POISONED     (1 << 11)
#define THERON_ATTR_DISEASED     (1 << 12)

/* ── Per-champion equip slot count ─────────────────────────────────── */
/* Matches DM1 champion equipment slots: WEAPON, ARMOR, SHIELD, HELM,
 * BOOTS, AMULET, GAUNTLETS, RING1, RING2 (9 slots). */
#define THERON_EQUIP_SLOT_COUNT   9
typedef enum {
    THERON_ESLOT_WEAPON   = 0,
    THERON_ESLOT_ARMOR    = 1,
    THERON_ESLOT_SHIELD   = 2,
    THERON_ESLOT_HELM     = 3,
    THERON_ESLOT_BOOTS    = 4,
    THERON_ESLOT_AMULET   = 5,
    THERON_ESLOT_GAUNTLETS= 6,
    THERON_ESLOT_RING1    = 7,
    THERON_ESLOT_RING2    = 8,
} Theron_EquipSlot;

/* ── Champion struct ──────────────────────────────────────────────── */
/* 128 bytes per champion — mirrors DM1 v1 champion block size.
 * Persists across dungeons: Theron fully, companions partially. */
typedef struct {
    /* Identity */
    char     name[24];               /* null-terminated champion name */
    uint8_t  portrait_index;        /* portrait sprite index */
    Theron_ChampionClass primary_class;
    uint8_t  alive;                 /* 1 = alive, 0 = dead */

    /* Vital stats (persist for all champions) */
    int16_t  health,  max_health;
    int16_t  stamina, max_stamina;
    int16_t  mana,    max_mana;

    /* Attribute stats (persist for all champions) */
    int16_t  strength;
    int16_t  dexterity;
    int16_t  wisdom;
    int16_t  vitality;
    int16_t  anti_magic;
    int16_t  anti_fire;

    /* Class levels (persist for all champions — XP earned stays) */
    int16_t  fighter_level;
    int16_t  ninja_level;
    int16_t  priest_level;
    int16_t  wizard_level;

    /* Wound bitmask (persists) */
    uint8_t  wounds;

    /* Attribute flags */
    uint16_t attributes;

    /* Inventory — RESET each dungeon for all companions.
     * Theron's inventory persists across dungeons. */
    uint8_t  inventory[THERON_INVENTORY_SLOTS];

    /* Equipment slots — RESET for companions, persist for Theron. */
    int16_t  slots[THERON_EQUIP_SLOT_COUNT];  /* item IDs or -1 */

    /* Load tracking (recalculated each dungeon entry) */
    int16_t  load;
    int16_t  max_load;

    /* Consumables (food/water — persist across dungeons) */
    int16_t  food;
    int16_t  water;

    uint8_t  padding[6];
} Theron_V1_Champion;

/* ── Party struct ─────────────────────────────────────────────────── */

/* Party = Theron + up to 3 companions (4 slots).
 * Gold is shared and persists for all champions. */
typedef struct {
    Theron_V1_Champion champions[THERON_MAX_CHAMPIONS];
    int                champion_count;   /* 0..4, starts at 4 */
    int                active_slot;       /* 0-based index of leader */
    uint32_t           gold;             /* shared party gold */
    /* Leader (party front) position and facing direction.
     * Source: THQUEST.ASM T520 — party placement / start position. */
    int16_t            leader_x;
    int16_t            leader_y;
    int8_t             leader_dir;        /* 0=N 1=E 2=S 3=W */
    int8_t             _pad1;
    int                levitating;        /* non-zero while pit fall is suppressed */
    /* Door state override: per-door state snapshot for state hashing.
     * Source: THQUEST.ASM T900 door object state. */
    int                door_state_override;
} Theron_V1_Party;

/* ── Persistence mode descriptors ─────────────────────────────────── */

/* Describes how each champion slot behaves on dungeon entry/exit.
 * Source: THQUEST.ASM T800. */
typedef enum {
    /* Theron persists fully: stats, skills, equipped items, inventory */
    THERON_PERSIST_FULL     = 0,
    /* Companion: stats/skills/gold persist; inventory and equip reset */
    THERON_PERSIST_PARTIAL  = 1,
} Theron_PersistMode;

#define THERON_PERSIST_MODE(slot) \
    ((slot) == THERON_CHAMPION_SLOT_THERON ? THERON_PERSIST_FULL : THERON_PERSIST_PARTIAL)

/* ── API — party management ─────────────────────────────────────── */

/* Init party to fresh state: 1 Theron + 3 blank companion slots.
 * dungeon_index = 1..7 (THERON_DUNGEON_1..THERON_DUNGEON_7). */
void theron_v1_party_init(Theron_V1_Party *party, int dungeon_index);

/* Dungeon entry: apply per-dungeon inventory reset for companions.
 * Call on dungeon entry after level load.
 *   - Companions (slots 1-3): clear inventory + equipment
 *   - Theron (slot 0): unchanged
 *   - max_load is recalculated for all champions. */
void theron_v1_party_dungeon_entry_reset(Theron_V1_Party *party);

/* Dungeon exit: no state change (all champion state is captured in save).
 * Placeholder for future door/torch state if needed. */
void theron_v1_party_dungeon_exit(Theron_V1_Party *party);

/* Get champion pointer by slot index (0..3). */
Theron_V1_Champion *theron_v1_party_getChampion(Theron_V1_Party *party, int slot);

/* Get the active leader (Theron by default, or player-selected). */
Theron_V1_Champion *theron_v1_party_leader(Theron_V1_Party *party);

/* Pack party state into a flat byte buffer for save/load.
 * Size must be >= theron_v1_party_pack_size(). */
size_t theron_v1_party_pack(const Theron_V1_Party *party, void *buf, size_t bufsize);

/* Unpack save buffer back into party struct.
 * Returns 0 on success, -1 on buffer too small. */
int theron_v1_party_unpack(Theron_V1_Party *party, const void *buf, size_t bufsize);

/* Returns the minimum.buffer size needed to pack a party. */
size_t theron_v1_party_pack_size(void);

/* Check if Theron is alive (needed for quest completion). */
int theron_v1_party_theron_alive(const Theron_V1_Party *party);

/* Party total HP for display / death check. */
int16_t theron_v1_party_total_health(const Theron_V1_Party *party);

/* Recalculate max_load for ALL champions after equip/inventory change. */
void theron_v1_party_recalculate_loads(Theron_V1_Party *party);

/* Champion skill level (primary class) — for UI display. */
int theron_v1_champion_skill_level(const Theron_V1_Champion *c);

/* Champion dead/alive predicate. */
int theron_v1_champion_is_alive(const Theron_V1_Champion *c);

/* Reset one champion's inventory (clear all 30 slots, mark equip -1). */
void theron_v1_champion_reset_inventory(Theron_V1_Champion *c);

/* Compute champion block size (for save/load). */
size_t theron_v1_champion_block_size(void);

/* Source evidence citation. */
const char *theron_v1_champions_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_CHAMPIONS_H */
