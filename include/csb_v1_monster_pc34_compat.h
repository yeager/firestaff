#ifndef FIRESTAFF_CSB_V1_MONSTER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_MONSTER_PC34_COMPAT_H

/*
 * CSB V1 Monster System — Source-Locked to CSBWin + ReDMCSB
 *
 * CSB shares DM1's base creature roster (types 0x00-0x18) but adds:
 *   - Grey Lord (0x1a): new named boss creature
 *   - MONSTERDESC struct: 26-byte binary format matching dungeon.dat
 *   - DSA Monster Attack Filter: script hook intercepting attack parameters
 *   - DSA Monster Movement Filter: per-level script hook for movement decisions
 *   - Extended projectile types: DispellMissile, ZoSpell, PoisonBolt
 *   - Extended sound table: G2003_aauc_CreatureSounds[18][2]
 *
 * Source-locked to:
 *   - CSBWin CSB.h:2323-2375 (MONSTERDESC struct, 26 bytes)
 *   - CSBWin Data.h:1093 (MonsterDescriptor[27])
 *   - CSBWin Monster.cpp:111-1200 (CreateMonster, MonsterAttacks, DSA hooks)
 *   - CSBWin Attack.cpp:2423 (Grey Lord spawn)
 *   - CSBWin Chaos.cpp:791-804 (Grey Lord attack bytes)
 *   - ReDMCSB DEFS.H:1575-1594 (CREATURE_INFO base struct)
 *   - ReDMCSB GROUP.C:716-1807 (drops, sounds, attack dispatch)
 *   - ReDMCSB DEFS.H:5618-5626 (G0245-G0253 drop tables)
 *   - ReDMCSB DEFS.H:1673-1680 (C0-C5_ATTACK_* types)
 */

#include <stdint.h>
#include "memory_combat_pc34_compat.h"  /* RngState_Compat */

/* Forward declarations (full definitions in respective compat headers) */
struct CSB_V1_DungeonData_Compat;

/* ============================================================
 *  CSB Creature Type Constants (DEFS.H:1339-1366, CSB AsciiDump)
 *  C027_CREATURE_TYPE_COUNT = 27 (0x00-0x1a)
 * ============================================================ */

#define CSB_CREATURE_TYPE_COUNT 27
#define CSB_CREATURE_TYPE_GREY_LORD 0x1a
#define CSB_CREATURE_TYPE_LORD_ORDER 0x19  /* unused placeholder */
#define CSB_CREATURE_TYPE_LORD_CHAOS 0x17
#define CSB_CREATURE_TYPE_VEXIRK 0x0e
#define CSB_CREATURE_TYPE_DEMON 0x16
#define CSB_CREATURE_TYPE_RED_DRAGON 0x18
#define CSB_CREATURE_TYPE_GIGGLER 0x02
#define CSB_CREATURE_TYPE_SWAMP_SLIME 0x01
#define CSB_CREATURE_TYPE_WIZARD_EYE 0x03
#define CSB_CREATURE_TYPE_ZYTAZ 0x13

/* ============================================================
 *  MONSTERDESC Struct (CSB binary format, 26 bytes)
 *
 *  Source: CSB.h:2323-2375
 *  Layout mirrors DEFS.H CREATURE_INFO with additional fields
 *  for DSA-scriptable extended behavior.
 * ============================================================ */

/* word2 bitfield helpers (CSB.h:2335-2356) */
#define CSB_MON_DESC_WORD2_NONMATERIAL(w)   (((w) & 0x0040) != 0)
#define CSB_MON_DESC_WORD2_LEVITATING(w)    (((w) & 0x0020) != 0)
#define CSB_MON_DESC_WORD2_SEES_INVISIBLE(w) (((w) & 0x0800) != 0)
#define CSB_MON_DESC_WORD2_SEES_360(w)       (((w) & 0x0004) != 0)
#define CSB_MON_DESC_WORD2_BLOCKED(w)       (((w) & 0x0008) != 0)
#define CSB_MON_DESC_WORD2_CAN_SEE_DARK(w)   (((w) & 0x1000) != 0)
#define CSB_MON_DESC_WORD2_LEAVES_DROPPINGS(w) (((w) & 0x0200) != 0)
#define CSB_MON_DESC_WORD2_ABSORBS_DAGGERS(w) (((w) & 0x0400) != 0)
#define CSB_MON_DESC_WORD2_INVINCIBLE(w)    (((w) & 0x2000) != 0)
#define CSB_MON_DESC_WORD2_HORIZONTAL_SIZE(w) ((w) & 3)
#define CSB_MON_DESC_WORD2_VERTICAL_SIZE(w)   (((w) >> 7) & 3)

/* word14 bitfield helpers (CSB.h:2370) */
#define CSB_MON_DESC_SIGHT(sm)    ((sm) & 15)
#define CSB_MON_DESC_SMELL(sm)    (((sm) >> 8) & 15)
#define CSB_MON_DESC_ATTACK_RANGE(sm) (((sm) >> 12) & 15)

/* word16 bitfield helpers (CSB.h:2372) */
#define CSB_MON_DESC_BRAVERY(w)   (((w) >> 4) & 15)

/* word4 MONSTERDESC_WORD4 bitfield helpers (CSB.h:2299-2318) */
#define CSB_MON_DESC_WORD4_SIDE_GRAPHIC(w)   (((w) & 0x0008) != 0)
#define CSB_MON_DESC_WORD4_BACK_GRAPHIC(w)   (((w) & 0x0010) != 0)
#define CSB_MON_DESC_WORD4_ATTACK_GRAPHIC(w) (((w) & 0x0020) != 0)
#define CSB_MON_DESC_WORD4_NONATTACKING_MIRROR_OK(w) (((w) & 0x0004) != 0)
#define CSB_MON_DESC_WORD4_ATTACKING_MIRROR_OK(w) (((w) & 0x0200) != 0)
#define CSB_MON_DESC_WORD4_NIBBLE12(w) ((int)(((w) >> 12) & 3))
#define CSB_MON_DESC_WORD4_NIBBLE14(w) ((int)(((w) >> 14) & 3))

/*
 * CSB MONSTERDESC — 26-byte creature descriptor from dungeon.dat
 * Source: CSB.h:2323-2375, AsciiDump.cpp:1811-1855
 *
 * Binary layout (little-endian):
 *   Offset 0  (1): uByte0          — creature type index (same as offset 0)
 *   Offset 1  (1): attackSound     — sound ordinal (0 = no sound)
 *   Offset 2  (2): word2           — attributes/size (little-endian swapped)
 *   Offset 4  (2): word4.mdw4      — graphic info flags (little-endian)
 *   Offset 6  (1): movementTicks06  — ticks per movement; 255 = immobile
 *   Offset 7  (1): attackTicks07    — minimum ticks between attacks
 *   Offset 8  (1): defense08        — defense points
 *   Offset 9  (1): baseHealth09     — base health
 *   Offset 10 (1): attack10         — attack power
 *   Offset 11 (1): poisonAttack11   — poison attack power
 *   Offset 12 (1): dexterity12      — dexterity (used as missile damage)
 *   Offset 13 (1): unused13         — padding (compiler-added on Atari ST)
 *   Offset 14 (2): word14           — sight/smell/attack range (little-endian)
 *   Offset 16 (2): word16           — bravery/experience (little-endian)
 *   Offset 18 (2): word18           — resistances (little-endian)
 *   Offset 20 (2): word20           — animation timing (little-endian)
 *   Offset 22 (4): uByte22[4]       — wound probabilities (head/legs/torso/feet)
 *
 * Total: 26 bytes per creature × 27 creatures = 702 bytes (dungeon.dat)
 *
 * Note: word2, word14, word16, word18, word20 are stored little-endian
 * (LE16 swapped on read from file, matching ReDMCSB DUNGEON.C swap pattern).
 * word4.mdw4 is also little-endian (swapped on read).
 */
typedef struct {
    uint8_t  uByte0;         /* creature type index; bits 0-1 tested for == 1 */
    uint8_t  attackSound;     /* sound ordinal; 0 = no sound */

    /* word2 — attributes/size flags (little-endian on disk)
     * Bits: 0-1=horizontal size, 2=see 360, 3=blocked, 4=????,
     *       5=levitating, 6=non-material, 7=immune to missiles,
     *       7-8=door size, 9=leaves droppings, 10=absorbs daggers,
     *       11=sees invisible, 12=sees in dark, 13=invincible */
    int16_t   word2;

    /* word4 — MONSTERDESC_WORD4 (little-endian on disk)
     * Bit 2=non-attacking mirror ok, 3=side graphic exists,
     * 4=back graphic exists, 5=attack graphic exists,
     * 7=????, 8=????, 9=????, 10=????, 12-13=????, 14-15=???? */
    uint16_t  word4;

    uint8_t  movementTicks06;  /* ticks per move; 255 = immobile */
    uint8_t  attackTicks07;   /* minimum ticks between attacks */
    uint8_t  defense08;        /* defense points */
    uint8_t  baseHealth09;    /* base health */
    uint8_t  attack10;         /* attack power */
    uint8_t  poisonAttack11;   /* poison attack power */
    uint8_t  dexterity12;      /* dexterity (used as missile damage) */
    uint8_t  unused13;         /* padding byte */
    int16_t  word14;          /* sight(bits0-3)/smell(bits8-11)/attackRange(bits12-15) */
    int16_t  word16;          /* bravery(bits4-7)/experience/wariness */
    int16_t  word18;          /* resistances */
    int16_t  word20;          /* non-attack/attack aspect timing */
    uint8_t  uByte22[4];       /* wound probabilities: head/legs/torso/feet */
} CSB_V1_MonsterDesc;

/* ============================================================
 *  Attack Parameters — Built by MonsterAttacks (Monster.cpp:923)
 *
 *  These are passed to the DSA Monster Attack Filter and then
 *  used to launch the actual attack.
 * ============================================================ */

typedef struct {
    int32_t  monsterID;           /* monster RN as integer */
    int32_t  monsterType;        /* CSB_CREATURE_TYPE_* */
    int32_t  monsterIndex;       /* index within group (0-3) */
    int32_t  monsterLevel;       /* dungeon level */
    int32_t  monsterX;           /* group map X */
    int32_t  monsterY;           /* group map Y */
    int32_t  monsterPos;         /* 2-bit cell position within square */
    int32_t  directionToParty;   /* primary direction to party (0-3) */
    int32_t  distanceToParty;    /* orthogonal distance */
    int      monsterShouldLaunchMissile;  /* bool: ranged attack flag */
    int      monsterShouldSteal;         /* bool: Giggler steal */
    int32_t  missileType;        /* RNVAL of projectile thing */
    int32_t  missileRange;       /* computed range (attack10/4 + 1 + random) */
    int32_t  missileDamage;      /* damage (from dexterity12) */
    int32_t  missileDecayRate;   /* step energy (always 8) */
    int32_t  heroToDamage;       /* target champion index (0-3) */
    int32_t  missileOriginPosition; /* cell for missile origin */
    int32_t  attackSoundOrdinal;    /* from MONSTERDESC.attackSound */
    int16_t  supressPoison;      /* poison suppression (-1 = not suppressed) */
} CSB_V1_AttackParameters;

/* ============================================================
 *  Projectile Type RNVAL Constants (CSB binary)
 * ============================================================ */

#define CSB_PROJECTILE_FIREBALL          0x00  /* RNFireball */
#define CSB_PROJECTILE_POISON            0x01  /* RNPoison */
#define CSB_PROJECTILE_LIGHTNING          0x02  /* RNLightning */
#define CSB_PROJECTILE_DISPELL_MISSILE    0x03  /* RNDispellMissile — CSB new */
#define CSB_PROJECTILE_ZO_SPELL          0x04  /* RNZoSpell — CSB new */
#define CSB_PROJECTILE_POISON_CLOUD      0x05  /* RNPoisonCloud */
#define CSB_PROJECTILE_POISON_BOLT       0x06  /* RNPoisonBolt — shared */
#define CSB_PROJECTILE_OPEN_DOOR         0x07  /* RNOpenDoor */

/* ============================================================
 *  Attack Type Constants (DEFS.H:1673-1681 — all 8 types)
 * ============================================================ */

#define CSB_ATTACK_NORMAL                0  /* Giggler, poison, stamina drain; no wounds */
#define CSB_ATTACK_FIRE                  1  /* Fireball explosions, Black Flame */
#define CSB_ATTACK_SELF                  2  /* Party walks into wall, falls through pit */
#define CSB_ATTACK_BLUNT                 3  /* Non-explosion projectiles, Demon/Mummy/Ruster/etc. */
#define CSB_ATTACK_MATERIAL_PROJECTILE   4  /* Material projectiles; +50% defense */
#define CSB_ATTACK_MAGIC                 5  /* Grey Lord, Lord Chaos, Lord Order, Zytaz, Vexirk, Wizard Eye */
#define CSB_ATTACK_PSYCHIC               6  /* Ghost/Rive, Screamer */
#define CSB_ATTACK_LIGHTNING             7  /* Lightning Bolt explosions */

/* ============================================================
 *  Sound Constants (DEFS.H:63-133, MEDIA529 I34E PC34)
 *
 *  Only the two drop-trigger sounds are needed here (GROUP.C:645).
 *  Full CSB sound constants (C00-C34) mirror DEFS.H:63-133.
 * ============================================================ */

#define CSB_SOUND_METALLIC_THUD  0  /* C00_SOUND_METALLIC_THUD — weapon drop */
#define CSB_SOUND_WOODEN_THUD     4  /* C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM — non-weapon drop */

/* ============================================================
 *  Drop Thing Type Constants (DEFS.H thing type values)
 *  Same values as DM1: WEAPON=5, ARMOUR=6, JUNK=10
 * ============================================================ */

#define CSB_DROP_THING_TYPE_WEAPON  5
#define CSB_DROP_THING_TYPE_ARMOUR  6
#define CSB_DROP_THING_TYPE_JUNK    10

/* ============================================================
 *  DSA Filter Location Keys
 *
 *  Source: Monster.cpp:1134-1180 (attack filter), 3317-3370 (movement filter)
 * ============================================================ */

#define CSB_DSA_KEY_MONSTER_ATTACK_FILTER(esp, esl) \
    (((esp) << 24) | (esl))

/* ============================================================
 *  Fixed Possessions Drop Table References (DEFS.H:5618-5626)
 *
 *  Creature type → drop table index in dungeon data.
 *  Used by GROUP.C F0186_GROUP_DropCreatureFixedPossessions.
 * ============================================================ */

#define CSB_DROP_SKELETON          0   /* G0245: 3 entries */
#define CSB_DROP_STONE_GOLEM       1   /* G0246: 2 entries */
#define CSB_DROP_TROLIN_ANTMAN     2   /* G0247: 2 entries */
#define CSB_DROP_ANIMATED_ARMOUR   3   /* G0248: 7 entries */
#define CSB_DROP_ROCK_ROCKPILE     4   /* G0249: 5 entries */
#define CSB_DROP_PAIN_RAT          5   /* G0250: 3 entries */
#define CSB_DROP_SCREAMER          6   /* G0251: 3 entries */
#define CSB_DROP_MAGENTA_WORM      7   /* G0252: 4 entries */
#define CSB_DROP_RED_DRAGON        8   /* G0253: 11 entries */

/* ============================================================
 *  API — Monster Descriptor Access
 * ============================================================ */

/*
 * csb_v1_monsterdesc_parse — Parse 26-byte MONSTERDESC from dungeon.dat
 *
 * Source: CSB.h:2323-2375 (struct layout)
 *         AsciiDump.cpp:1811-1855 (DumpMonster field listing)
 *
 * @param src      Source buffer (26 bytes, little-endian)
 * @param dst      Parsed descriptor output
 * @param idx      Creature type index (0x00-0x1a), validated against < 27
 *
 * Note: word2, word14, word16, word18, word20 are stored little-endian
 * with byte-swapping on read (matching ReDMCSB DUNGEON.C swap pattern).
 * word4 is similarly byte-swapped.
 */
void csb_v1_monsterdesc_parse(const uint8_t *src,
                               CSB_V1_MonsterDesc *dst,
                               int idx);

/*
 * csb_v1_monsterdesc_sight_distance — Extract sight range
 * Source: CSB.h:2370 (sightDistance())
 */
int csb_v1_monsterdesc_sight_distance(const CSB_V1_MonsterDesc *m);

/*
 * csb_v1_monsterdesc_smell_distance — Extract smell range
 * Source: CSB.h:2370 (smellingDistance())
 */
int csb_v1_monsterdesc_smell_distance(const CSB_V1_MonsterDesc *m);

/*
 * csb_v1_monsterdesc_attack_range — Extract attack range
 * Source: CSB.h:2370 (word14_12_15())
 */
int csb_v1_monsterdesc_attack_range(const CSB_V1_MonsterDesc *m);

/*
 * csb_v1_monsterdesc_bravery — Extract bravery (fear resistance)
 * Source: CSB.h:2372 (bravery())
 */
int csb_v1_monsterdesc_bravery(const CSB_V1_MonsterDesc *m);

/* ============================================================
 *  API — Attack Resolution
 * ============================================================ */

/*
 * csb_v1_monster_get_defense — Get defense vs attack type
 *
 * Source: CSBWin Attack.cpp defense calculation
 *         ReDMCSB CHAMPION.C F0311-F0321 damage pipeline
 *
 * @param m         Monster descriptor
 * @param attack_type CSB_ATTACK_* type
 * @return          Defense points (type-4 gets +50% bonus)
 */
int csb_v1_monster_get_defense(const CSB_V1_MonsterDesc *m, int attack_type);

/*
 * csb_v1_attack_resolve — Resolve net damage from attack
 *
 * Source: CSBWin Attack.cpp
 *         ReDMCSB CHAMPION.C damage resolution
 *
 * @param damage        Raw damage from attacker
 * @param defense       Defense points (from csb_v1_monster_get_defense)
 * @return              Net damage (0 if defense >= damage)
 */
int csb_v1_attack_resolve(int damage, int defense);

/*
 * csb_v1_attack_parameters_build — Build attack parameters from monster state
 *
 * Source: Monster.cpp:949-1130 (MonsterAttacks parameter building)
 *
 * @param params        Output attack parameters
 * @param monsterType   Creature type index
 * @param monsterX      Monster group X
 * @param monsterY      Monster group Y
 * @param dirToParty    Primary direction to party (0-3)
 * @param distToParty   Orthogonal distance to party
 * @param partyPos      2-bit cell position byte (255 = single centered)
 * @param monsterIndex  Index within group (0-3)
 * @param monsterLevel  Dungeon level
 */
void csb_v1_attack_parameters_build(
    CSB_V1_AttackParameters *params,
    int monsterType,
    int monsterX,
    int monsterY,
    int dirToParty,
    int distToParty,
    int partyPos,
    int monsterIndex,
    int monsterLevel);

/*
 * csb_v1_projectile_type_for_creature — Resolve projectile type for creature
 *
 * Source: Monster.cpp:1049-1080 (missileType switch)
 *
 * @param creatureType   CSB_CREATURE_TYPE_*
 * @param rng_state      RNG state for random selection
 * @return               CSB_PROJECTILE_* RNVAL
 */
int csb_v1_projectile_type_for_creature(int creatureType,
                                       struct RngState_Compat *rng);

/*
 * csb_v1_missile_range_compute — Compute missile range from attack power
 *
 * Source: Monster.cpp:1116-1124
 *
 *   baseRange = attack10 / 4 + 1
 *   range += random(range) + random(range)  [two additions]
 */
int csb_v1_missile_range_compute(int attack_power, struct RngState_Compat *rng);

/* ============================================================
 *  API — DSA Filter Support
 * ============================================================ */

/*
 * csb_v1_dsa_filter_attack_preprocess — DSA attack filter preprocessing
 *
 * Called before attack execution. Checks for DSA Monster Attack Filter
 * at the configured ESL_MONSTERATTACKFILTER location. If found (actuator
 * type 47), the filter is called and may modify attackParameters in place.
 *
 * Source: Monster.cpp:1116-1180
 *
 * @param params      Attack parameters (in/out: may be modified by filter)
 * @param dungeon     Current dungeon data reference
 * @return            1 if filter was found and executed, 0 otherwise
 */
int csb_v1_dsa_filter_attack_preprocess(
    CSB_V1_AttackParameters *params,
    const struct CSB_V1_DungeonData_Compat *dungeon);

/*
 * csb_v1_dsa_filter_movement_preprocess — DSA movement filter preprocessing
 *
 * Called during MonsterMovement(). Checks for per-level movement filter
 * object. If found, the filter receives movement parameters and may
 * modify mmr.flgs[0]/flgs[1] to alter movement behavior.
 *
 * Source: Monster.cpp:3222-3370
 *
 * @param level       Dungeon level (0-7 typically)
 * @param mapX        Group map X
 * @param mapY        Group map Y
 * @param monster     Monster RN integer
 * @param partyLevel  Party dungeon level
 * @param partyX      Party X
 * @param partyY      Party Y
 * @param flgs_inout  Movement flags (in/out: may be modified by filter)
 * @param dungeon     Current dungeon data reference
 * @return            1 if filter was found and executed, 0 otherwise
 */
int csb_v1_dsa_filter_movement_preprocess(
    int level,
    int mapX,
    int mapY,
    int32_t monster,
    int partyLevel,
    int partyX,
    int partyY,
    int flgs_inout[2],
    const struct CSB_V1_DungeonData_Compat *dungeon);

/* ============================================================
 *  API — Drop Sound
 * ============================================================ */

/*
 * csb_v1_drop_sound_for_item — Get drop sound for an item type
 *
 * Source: GROUP.C:645 F0064_SOUND_RequestPlay_CPSD
 *   Weapon dropped: C00_SOUND_METALLIC_THUD (0)
 *   No weapon (armour/junk): C04_SOUND_WOODEN_THUD (4)
 *
 * @param itemType   CSB_DROP_THING_TYPE_* (WEAPON=5, ARMOUR=6, JUNK=10)
 * @return            CSB_SOUND_METALLIC_THUD or CSB_SOUND_WOODEN_THUD
 */
int csb_v1_drop_sound_for_item(int itemType);

/* ============================================================
 *  API — Fixed Possessions Drop
 * ============================================================ */

/*
 * csb_v1_drop_fixed_possessions — Drop creature's fixed possessions
 *
 * Mirrors GROUP.C F0186_GROUP_DropCreatureFixedPossessions.
 * Drop table for creature type indexed by G0245-G0253.
 *
 * Source: GROUP.C:550-648 F0186_GROUP_DropCreatureFixedPossessions
 *         DEFS.H:5618-5626 (drop table globals)
 *
 * @param creatureType   CSB_CREATURE_TYPE_*
 * @param mapX           Drop location X
 * @param mapY           Drop location Y
 * @param cell           Cell within square (0-3 or 0xFF for single)
 * @param mode           Play mode (IMMEDIATELY, ONE_TICK_LATER, etc.)
 */
void csb_v1_drop_fixed_possessions(int creatureType,
                                   int mapX,
                                   int mapY,
                                   int cell,
                                   int mode);

/* ============================================================
 *  API — Source Evidence
 * ============================================================ */

const char *csb_v1_monster_source_evidence(void);

#endif /* FIRESTAFF_CSB_V1_MONSTER_PC34_COMPAT_H */