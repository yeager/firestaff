/*
 * theron_v1_combat.h — Theron's Quest V1 Phase 5: Combat System
 *
 * Champion attacks, creature AI, HP/stat damage, drops, and SFX triggers.
 *
 * TQR combat system — DM1-derived combat with unique altar-of-vi.
 * Spells use mana costs from Track 02 spell descriptor table (UD 0x09EFAF).
 *
 * Source references:
 *   THQUEST.ASM T500  — creature spawning / wave triggers
 *   THQUEST.ASM T600  — party/creature collision + combat resolution
 *   THQUEST.ASM T700  — per-tick HP/Poison processing
 *   THQUEST.ASM T900  — object drops / treasure tables / altar-of-vi
 *
 *   docs/source-lock/csb_v1_phase5_creature_combat_H2242.md
 *     (CSB creature roster — TQR uses a subset of DM1 creatures)
 */

#ifndef THERON_V1_COMBAT_H
#define THERON_V1_COMBAT_H

#include <stdbool.h>
#include <stdint.h>

#include "theron_v1_champions.h"

/* Forward declaration: Theron_V1_World is defined in theron_v1_world.h,
 * which includes this header for the creature/combat/sound types.
 * The typedef is skipped when theron_v1_world.h has already defined it. */
#ifndef THERON_V1_WORLD_TYPEDEF
typedef struct Theron_V1_World Theron_V1_World;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ── Creature constants ───────────────────────────────────────────── */
#define THERON_CREATURE_SLOT_BODYGUARD  0   /* 32KB bodyguard fighter */
#define THERON_CREATURE_SLOT_WARRIOR    1
#define THERON_CREATURE_SLOT_ARCHER    2
#define THERON_CREATURE_SLOT_MAGE       3
#define THERON_CREATURE_SLOT_SUICIDER  4   /* kamikaze bomb creature */
#define THERON_CREATURE_SLOT_COUNT      5

/* ── Creature type IDs — 7 real creature types from Track 02 UD 0x2741EF ── */
typedef enum {
    THERON_CREATURE_NONE          = 0,
    THERON_CREATURE_AKUTUBA       = 1,   /* Dungeon 0 creature */
    THERON_CREATURE_DRATOR        = 2,   /* Dungeon 1 creature */
    THERON_CREATURE_FORMIC        = 3,   /* Dungeon 2 creature */
    THERON_CREATURE_SARMON        = 4,   /* Dungeon 3 creature */
    THERON_CREATURE_SHADO         = 5,   /* Dungeon 4 creature */
    THERON_CREATURE_THIEF         = 6,   /* Dungeon 5 creature */
    THERON_CREATURE_DEMON         = 7,   /* Dungeon 6 creature (final) */
    THERON_CREATURE_TYPE_COUNT    = 8,
} Theron_CreatureType;

/* ── Creature state flags ────────────────────────────────────────── */
#define THERON_CF_ACTIVE          (1U << 0)   /* alive on map */
#define THERON_CF_SLEEPING        (1U << 1)   /* dormant until party enters */
#define THERON_CF_ALERTED         (1U << 2)   /* noise/door alarm triggered */
#define THERON_CF_FLYING         (1U << 3)   /* ignores pits */
#define THERON_CF_LEVITATING     (1U << 4)   /* anti-pit */
#define THERON_CF_POISONED        (1U << 5)
#define THERON_CF_PARALYZED       (1U << 6)
#define THERON_CF_BLIND           (1U << 7)
#define THERON_CF_CHARMED         (1U << 8)   /* friendly to party */
#define THERON_CF_INVISIBLE       (1U << 9)

/* ── AI behaviour modes ─────────────────────────────────────────── */
typedef enum {
    THERON_AI_PASSIVE       = 0,  /* only attacks if cornered */
    THERON_AI_GUARD         = 1,  /* guards location, attacks if approached */
    THERON_AI_CHASE         = 2,  /* hunts party aggressively */
    THERON_AI_SUICIDE       = 3,  /* moves to party and explodes */
} Theron_AIBehaviour;

/* ── Attack / damage ────────────────────────────────────────────── */
typedef enum {
    THERON_ATTACK_NONE      = 0,   /* no secondary attack */
    THERON_ATTACK_SLASH     = 1,   /* sword — strength-based */
    THERON_ATTACK_PIERCE    = 2,   /* arrow/polearm — dexterity-based */
    THERON_ATTACK_BLAST     = 3,   /* fireball / explosion */
    THERON_ATTACK_POISON    = 4,   /* poison fangs/claws */
    THERON_ATTACK_MAGIC     = 5,   /* magic (anti-magic blocks) */
    THERON_ATTACK_DEATH     = 6,   /* instant death gaze */
    THERON_ATTACK_SELF      = 7,   /* pit fall / fall damage */
} Theron_AttackType;

/* ── Sound IDs (TQ PC Engine ADPCM subset) ───────────────────────── */
typedef enum {
    THERON_SOUND_NONE        = 0,
    THERON_SOUND_SWORD_SWING = 1,
    THERON_SOUND_SHIELD_BLOCK = 2,
    THERON_SOUND_CREATURE_HIT  = 3,
    THERON_SOUND_CREATURE_DIE = 4,
    THERON_SOUND_PARTY_HIT    = 5,
    THERON_SOUND_PARTY_DIE   = 6,
    THERON_SOUND_DOOR_OPEN   = 7,
    THERON_SOUND_DOOR_CLOSE  = 8,
    THERON_SOUND_DOOR_CREAK  = 9,
    THERON_SOUND_KEY_USE    = 10,
    THERON_SOUND_TELEPORT   = 11,
    THERON_SOUND_PIT_FALL   = 12,
    THERON_SOUND_ITEM_PICKUP = 13,
    THERON_SOUND_ITEM_DROP  = 14,
    THERON_SOUND_ALTAR_USE  = 15,
    THERON_SOUND_POOL_USE   = 16,
    THERON_SOUND_ALARM_TRIG = 17,
    THERON_SOUND_FIREBALL   = 18,
    THERON_SOUND_EXPLOSION  = 19,
    THERON_SOUND_AMBIENT_1  = 20,   /* dungeon ambient loop 1 */
    THERON_SOUND_AMBIENT_2  = 21,   /* dungeon ambient loop 2 */
    THERON_SOUND_STAIRS     = 22,
    THERON_SOUND_VICTORY    = 23,
    THERON_SOUND_DEFEAT     = 24,
    THERON_SOUND_BOOT_MUSIC = 25,   /* THQUEST.ASM T000 title music */
    THERON_SOUND_COUNT      = 26,
} Theron_SoundID;

/* ── Creature struct ────────────────────────────────────────────── */
#define THERON_MAX_CREATURES_PER_LEVEL  32

typedef struct {
    int        id;
    uint8_t    type;
    uint8_t    level;           /* dungeon/sub-level */
    int        dungeon_id;
    int        x, y;            /* grid position */
    int        hp;
    int        max_hp;
    int        attack;
    int        defense;
    int        speed;            /* ticks between moves */
    int        next_move_tick;   /* world tick when creature next acts */
    Theron_AIBehaviour ai;
    Theron_AttackType primary_attack;
    Theron_AttackType secondary_attack;
    uint32_t   flags;
    int        gold_drop_min;
    int        gold_drop_max;
    uint8_t    item_drop_table[4]; /* item IDs, 0 = end of list */
    int        link_id;          /* teleporter / trigger link */
} Theron_V1_Creature;

/* ── Combat result ──────────────────────────────────────────────── */
typedef enum {
    THERON_COMBAT_NONE      = 0,
    THERON_COMBAT_HIT      = 1,   /* attack landed */
    THERON_COMBAT_MISS     = 2,   /* attack evaded/blocked */
    THERON_COMBAT_KILL     = 3,   /* target died */
    THERON_COMBAT_FLED     = 4,   /* target fled */
} Theron_CombatResult;

/* ── Combat API ─────────────────────────────────────────────────── */

int  theron_v1_creature_spawn(Theron_V1_World *world,
                               Theron_CreatureType type,
                               int dungeon_id,
                               int level,
                               int x, int y);
int  theron_v1_creature_kill(Theron_V1_World *world, int creature_id);
int  theron_v1_creature_remove(Theron_V1_World *world, int creature_id);
Theron_V1_Creature *theron_v1_creature_at(Theron_V1_World *world,
                                           int level, int x, int y);
Theron_V1_Creature *theron_v1_creature_by_id(Theron_V1_World *world, int id);

int  theron_v1_creature_count(const Theron_V1_World *world,
                               int dungeon_id, int level);

/* ── Champion attack ─────────────────────────────────────────────── */
int theron_v1_champion_attack(Theron_V1_World *world,
                               int attacking_slot,
                               int target_creature_id);

/* ── Spell casting (indices 20-40 from Track 02 spell descriptors) ── */
int theron_v1_champion_cast_spell(Theron_V1_World *world,
                                   int casting_slot,
                                   unsigned int spell_index,
                                   int target_creature_id);

/* ── Creature attack (single creature vs champion) ───────────────── */
Theron_CombatResult theron_v1_creature_attack_champion(
    Theron_V1_World *world,
    int creature_id,
    int champion_slot);

/* ── Creature AI tick ─────────────────────────────────────────────── */
/*
 * theron_v1_creature_ai_tick — called once per world tick.
 *   Runs creature AI for all active creatures.
 *   Each creature has speed=N → acts every N ticks.
 *
 * Source: THQUEST.ASM T500/T600 creature wave logic.
 */
void theron_v1_creature_ai_tick(Theron_V1_World *world);

/* ── Damage calculation helpers ─────────────────────────────────── */
int theron_v1_calc_attack_damage(int attack_power,
                                   const Theron_V1_Champion *defender,
                                   Theron_AttackType type);
int theron_v1_calc_defense(const Theron_V1_Champion *defender,
                              Theron_AttackType type);

/* ── HP stat modification (clamped to valid ranges) ─────────────── */
int theron_v1_modify_champion_hp(Theron_V1_Champion *c, int delta);
int theron_v1_modify_champion_stamina(Theron_V1_Champion *c, int delta);
int theron_v1_modify_champion_mana(Theron_V1_Champion *c, int delta);

/* ── Death processing ─────────────────────────────────────────────── */
void theron_v1_champion_die(Theron_V1_World *world, int champ_slot);
void theron_v1_creature_die(Theron_V1_World *world, int creature_id);

/* ── Drops ────────────────────────────────────────────────────────── */
/*
 * theron_v1_drop_loot — publish a source-backed creature drop when killed.
 *   Returns -1 while the Track 02 T900 drop record is not decoded; no
 *   inferred gold or item is placed on the floor.
 *   Source: THQUEST.ASM T900 creature drops.
 */
int theron_v1_drop_loot(Theron_V1_World *world,
                         int creature_id, int x, int y);

/* ── Sound / audio trigger interface ────────────────────────────── */
/*
 * Sound backend is platform-specific.  Each platform provides:
 *   int firestaff_tqr_play_sound(Theron_SoundID id);
 * If not provided, these functions are no-ops (stubbed below).
 */
int theron_v1_play_sound(Theron_SoundID id);
int theron_v1_sound_is_valid(Theron_SoundID id);

/* ── Source citation ─────────────────────────────────────────────── */
const char *theron_v1_combat_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_COMBAT_H */
