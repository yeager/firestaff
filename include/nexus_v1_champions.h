
#ifndef NEXUS_V1_CHAMPIONS_H
#define NEXUS_V1_CHAMPIONS_H
#include <stdint.h>
#include <stddef.h>
#include "nexus_v1_inventory.h"

/* DM Nexus has the same champion system as DM1 but with Japanese names.
 * 24 champions in the Hall of Champions, 4 active in party.
 * Stats, skills, spells identical to DM1 engine.
 * Source: ReDMCSB CHAMPION.C F0309 (max load), F0310 (movement ticks),
 * F0306 (stamina adjustment), F0325 (stamina decrement). */

#define NEXUS_MAX_CHAMPIONS 24
#define NEXUS_MAX_PARTY 4

/* NEXUS_SLOT_* slot indices are defined in nexus_v1_inventory.h
 * (Nexus_EquipSlot enum). Keep the local slot array large enough for
 * the Nexus inventory slot ids, including ring slots and amulet.
 * Source: ReDMCSB CHAMPION.C F0309 (equipment layout). */

#define NEXUS_SLOT_COUNT    NEXUS_EQUIP_SLOTS

/* Wound bitmasks — matches DM1 MASK0x00XX_WOUND_* */
#define NEXUS_WOUND_HEAD    (1 << 0)
#define NEXUS_WOUND_BODY    (1 << 1)
#define NEXUS_WOUND_ARMS    (1 << 2)
#define NEXUS_WOUND_LEGS    (1 << 3)

/* Champion attribute flags — matches DM1 MASK0x0XXX */
#define NEXUS_ATTR_LOAD_CHANGED   (1 << 8)   /* load recalculated */
#define NEXUS_ATTR_STATISTICS     (1 << 9)   /* stats changed */
#define NEXUS_ATTR_DEAD           (1 << 10)  /* champion is dead */

typedef enum {
    NEXUS_CLASS_FIGHTER = 0,
    NEXUS_CLASS_NINJA,
    NEXUS_CLASS_PRIEST,
    NEXUS_CLASS_WIZARD,
    NEXUS_CLASS_COUNT
} Nexus_ChampionClass;

typedef struct {
    char name_ascii[32];      /* ASCII romanized name */
    char name_jp[64];         /* UTF-8 Japanese name */
    Nexus_ChampionClass primary_class;
    int health, max_health;
    int stamina, max_stamina;
    int mana, max_mana;
    int strength, dexterity, wisdom, vitality, anti_magic, anti_fire;
    int fighter_level, ninja_level, priest_level, wizard_level;
    int food, water;
    int alive;
    int direction;            /* 0=N 1=E 2=S 3=W, used by Vi altar rebirth */
    int portrait_index;       /* CG texture index for portrait */
    uint8_t inventory[30];    /* item indices (30 slots) */

    /* Equipment slots — added per ReDMCSB CHAMPION.C F0309.
     * Source: CHAMPION.C slot array (approx C05_SLOT_WEAPON..C13_SLOT_AMULET). */
    int slots[NEXUS_SLOT_COUNT];  /* item IDs in each slot, -1=empty */

    /* Load tracking — current load vs maximum load.
     * Source: CHAMPION.C F0309_GetMaximumLoad, F0310_GetMovementTicks.
     * load = sum of all carried item weights.
     * max_load = (strength<<3)+100, stamina-adjusted, wound-adjusted, +boots. */
    int load;           /* current total weight carried */
    int max_load;       /* maximum carry capacity (recalculated) */

    /* Wounds — per ReDMCSB CHAMPION.C F0309 (wound penalty to max_load).
     * Bitmask: HEAD=1, BODY=2, ARMS=4, LEGS=8.
     * Source: CHAMPION.C F0309 lines 1167-1173 (wound load penalty). */
    int wounds;

    /* Attribute flags — matches DM1 champion Attributes field.
     * Source: CHAMPION.C M008_SET pattern for LOAD|STATISTICS flags. */
    int attributes;
} Nexus_V1_Champion;

typedef struct {
    Nexus_V1_Champion champions[NEXUS_MAX_CHAMPIONS];
    int champion_count;
    int party[NEXUS_MAX_PARTY];
    int party_count;
    int leader_index;
} Nexus_V1_ChampionPool;

/* Maximum load calculation — matches ReDMCSB CHAMPION.C F0309.
 * max_load = (strength<<3) + 100, stamina-adjusted, wound-adjusted,
 * elven-boots bonus (+6.25%), rounded to nearest 10.
 * Source: CHAMPION.C F0309 lines 1157-1195, F0306_GetStaminaAdjustedValue.
 *
 * Movement tick rate — matches ReDMCSB CHAMPION.C F0310.
 * Returns number of game ticks between steps (55ms per tick).
 * When load >= max_load, movement is severely slowed (BUG0_72: uses > not >=).
 * Source: CHAMPION.C F0310 lines 1197-1222, BUG0_72 comment at line 1198. */
int nexus_champion_get_maximum_load(const Nexus_V1_Champion *c);
int nexus_champion_get_movement_ticks(const Nexus_V1_Champion *c);

/* Stamina decrement — matches ReDMCSB CHAMPION.C F0325.
 * Decrements stamina by 'cost'. If stamina reaches 0, champion
 * takes damage equal to |negative_stamina|/2 as wounds.
 * Source: CHAMPION.C F0325 lines 2025-2048. */
void nexus_champion_decrement_stamina(Nexus_V1_Champion *c, int cost);

/* Recalculate current load from inventory + equipment.
 * Source: CHAMPION.C load update on inventory change. */
void nexus_champion_recalc_load(Nexus_V1_Champion *c);

void nexus_v1_champions_init(Nexus_V1_ChampionPool *pool);
int nexus_v1_champion_recruit(Nexus_V1_ChampionPool *pool, int mirror_index);
int nexus_v1_champion_resurrect(Nexus_V1_ChampionPool *pool, int party_slot);

/* ── Champion pool binary serialization (Phase 6 save/load) ────────────────
 *
 * Serializes the entire champion pool (24 champions + party + leader state)
 * into a flat binary buffer. Unpacked from buffer on load.
 *
 * Layout (little-endian):
 *   magic(4) + version(2) + champion_count(4) + party_count(4)
 *   + leader_index(4) + leader_index_24(4)
 *   + [champion_count × champion blob]
 *   + [4 × party slot (4 bytes each)]
 *
 * Champion blob: name_ascii(32) + name_jp(64) + all int fields
 *                 + inventory(30) + slots(9×4)
 *                 = 32+64+23*4+30+9*4 = 268 bytes per champion
 *
 * Source-lock: ReDMCSB LOADSAVE.C F0433/F0434 (DM1 save/load structure),
 *              CHAMPION.C F0309 (champion save format). */

#define NEXUS_CHAMPION_POOL_SAVE_MAGIC  0x4348504EU  /* 'CHPN' */
#define NEXUS_CHAMPION_POOL_SAVE_VERSION 1

/* Returns minimum buffer size needed for serialization. */
size_t nexus_v1_champion_pool_serialize_size(const Nexus_V1_ChampionPool *pool);

/* Serialize pool into buf. Returns bytes written, 0 on error. */
size_t nexus_v1_champion_pool_serialize(const Nexus_V1_ChampionPool *pool,
                                         void *buf, size_t bufsize);

/* Deserialize buf into pool. Returns 0 on success, <0 on error. */
int nexus_v1_champion_pool_deserialize(Nexus_V1_ChampionPool *pool,
                                        const void *buf, size_t bufsize);

#endif
