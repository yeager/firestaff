#ifndef FIRESTAFF_DM2_V1_CREATURE_H
#define FIRESTAFF_DM2_V1_CREATURE_H
#include <stdint.h>

/* DM2 V1 — Creature AI, Attacks, and Spells
 * Phase 6 source-lock (2026-05-26)
 * ReDMCSB: SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
 * Secondary: skproject/SKWIN/SkWinCore.cpp, DME.h, defines.h
 * Secondary: skproject/SKULLWIN/c_ai.cpp, c_creature.cpp, c_creature.h
 *
 * DM2 uses a CCM (creature command message) byte-dispatch state machine
 * driven by the per-creature primary state register `b_1a`.
 * Source: SKULLWIN/c_creature.cpp: DM2_PROCEED_CCM
 */

/* ── AI_ATTACK_FLAGS — creature attack/spell flags ──────────────────────
 * Source: skproject/SKWIN/defines.h:705-716
 * Resolved at: SkWinCore.cpp:415-437 (dispatch), 27038-27096 (spell effects) */

#define AI_ATTACK_FLAGS__MELEE          0x0001
#define AI_ATTACK_FLAGS__PUSH_BACK      0x0002  /* knockback on hit */
#define AI_ATTACK_FLAGS__STEAL          0x0004  /* Giggler(26), Thicket Thief(27) */
#define AI_ATTACK_FLAGS__SHOOT          0x0008  /* Archer Guard(36) */
#define AI_ATTACK_FLAGS__FIREBALL       0x0010  /* Amplifier(51) */
#define AI_ATTACK_FLAGS__DISPELL        0x0020  /* remove enchantments */
#define AI_ATTACK_FLAGS__LIGHTNING      0x0040  /* single-target electric */
#define AI_ATTACK_FLAGS__POISON_CLOUD   0x0080  /* AoE poison cloud */
#define AI_ATTACK_FLAGS__POISON_BOLT    0x0100  /* single-target poison bolt */
#define AI_ATTACK_FLAGS__POISON_BLOB    0x0200  /* contact poison blob (Giggler) */
#define AI_ATTACK_FLAGS__PUSH_SPELL     0x0400  /* telekinetic push */
#define AI_ATTACK_FLAGS__PULL_SPELL     0x0800  /* telekinetic pull */

/* ── w0AIFlags bitfield — AIDefinition behavior flags ───────────────────
 * Source: skproject/SKWIN/DME.h:1545-1560
 * Accessors defined in skproject/SKWIN/DME.h */

#define DM2_AIFLAG_STATIC        0x0001  /* IsStaticObject(): non-moving object */
#define DM2_AIFLAG_REFLECTOR     0x0002  /* w0_1_1: reflects attacks */
#define DM2_AIFLAG_SPECTRE       0x0008  /* w0_3_3: spectre/ghost type */
#define DM2_AIFLAG_SPECTRE_VEXIRK 0x0010 /* w0_4_4: spectre+vexirks */
#define DM2_AIFLAG_NONMATERIAL   0x0020  /* w0_5_5: intangible creature */
#define DM2_AIFLAG_WORM_GLOP     0x00C0  /* w0_6_7: worms and glops */
#define DM2_AIFLAG_PUSH_WHEN_MOVE 0x0100 /* PushWhenMoving(): pushes target */
#define DM2_AIFLAG_ABSORBS_MISSILE 0x0200 /* AbsorbsMissile(): blocks projectiles */
#define DM2_AIFLAG_INVISIBLE     0x0400  /* w0_a_a: invisibility (ghosts+dragoth) */

/* w30 flag for missile turning (checked at SkWinCore.cpp:10479,10561) */
#define DM2_AI_W30_TURNS_MISSILE  0x0800  /* creature redirects projectiles */

/* ── AIDefinition struct — 36 bytes per creature type ──────────────────
 * Source: skproject/SKWIN/DME.h:1505-1545
 * Instance lookup: QUERY_CREATURE_AI_SPEC_FROM_TYPE(type) → AIDefinition*
 * Extended mode override: EXTENDED_LOAD_AI_DEFINITION() at SkWinCore.cpp:233-400 */

typedef struct __attribute__((packed)) {
    uint16_t w0AIFlags;      /* @0  — behavior/static/flying/invisible bits */
    uint8_t  ArmorClass;     /* @2  — defense rating */
    int8_t   b3;             /* @3  */
    uint16_t BaseHP;         /* @4  — initial hit points */
    uint8_t  AttackStrength; /* @6  — base physical damage */
    uint8_t  PoisonDamage;   /* @7  — poison damage on hit */
    uint8_t  Defense;        /* @8  — 255=undestroyable */
    uint8_t  b9x;            /* @9  — 0x40: pit ghost marker */
    uint16_t w10;            /* @10 */
    uint16_t w12;            /* @12 */
    uint16_t AttacksSpells;  /* @14 — AI_ATTACK_FLAGS (bitfield) */
    uint16_t w16;            /* @16 — switch triggers */
    uint16_t w18;            /* @18 */
    uint16_t w20;            /* @20 */
    uint16_t w22;            /* @22 */
    uint16_t w24;            /* @24 — resistance (fire/poison) */
    uint16_t w26;            /* @26 */
    uint8_t  b28;            /* @28 */
    uint8_t  Weight;         /* @29 — push resistance, 255=immovable */
    uint16_t w30;            /* @30 — 0x0800=turns missiles */
    uint16_t w32;            /* @32 */
    uint8_t  b34;            /* @34 */
    uint8_t  b35;            /* @35 */
} DM2_AIDefinition;  /* 36 bytes */

/* ── CCM command byte values (b_1a primary state register) ──────────────
 * Source: SKULLWIN/c_creature.cpp: DM2_PROCEED_CCM, c_creature.h
 * b_1a written directly by action handlers — no explicit next-state field */

#define DM2_CCM_WALK_NOW              0x00  /* movement dispatch */
#define DM2_CCM_ATTACK_HANDLER        0x01  /* attack handler */
#define DM2_CCM_WALK_CONT              0x02  /* movement continuation */
#define DM2_CCM_SPECIAL_ACTION        0x05  /* CCM06/CCM0B/CCM0C */
#define DM2_CCM_STEAL_ITEM            0x09  /* thief-type item theft */
#define DM2_CCM_MERCHANT_BEHAVIOR     0x0a  /* merchant/shop behavior */
#define DM2_CCM_SHOOT_ITEM            0x0d  /* ranged throw/pickup */
#define DM2_CCM_KILL_ON_TIMER_POS     0x0f  /* delayed-position kill (0x0f-0x13) */
#define DM2_CCM_ROTATES_TARGET        0x13  /* reorient another creature */
#define DM2_CCM_CAST_SPELL            0x15  /* monster spellcasting */
#define DM2_CCM_CREATURE_ATTACKS_PARTY 0x17 /* fallback attack */
#define DM2_CCM_EXPLODE_OR_SUMMON     0x26  /* self-destruct or spawn minion */

/* ── AI index table size ────────────────────────────────────────────────
 * Source: skproject/SKWIN/SkGlobal.h:636, SkWinCore.cpp:741-810 */

#define DM2_AI_TABLE_SIZE         64   /* 0x00–0x3E used, index 62 duplicated */
#define DM2_AI_INDEX_MAX          255  /* extended mode creature ID max */
#define DM2_AI_MAX_NAME           32

/* ── Companion/minion AI indices (DM2-specific, no DM1 equivalent) ─────
 * Source: skproject/SKWIN/SkWinCore.cpp:741-810, getAIName
 * All ally indices: 13–18. All evil indices: 34,43,49,62 */

#define DM2_AI_SCOUT_MINION       13   /* ally: companion scout */
#define DM2_AI_ATTACK_MINION      14   /* ally: combat minion */
#define DM2_AI_CARRY_MINION       15   /* ally: carry items */
#define DM2_AI_FETCH_MINION       16   /* ally: fetch items */
#define DM2_AI_GUARD_MINION       17   /* ally: guard position */
#define DM2_AI_UHAUL_MINION       18   /* ally: move objects */
#define DM2_AI_THORN_DEMON        19   /* enemy: drops sellable worm food */
#define DM2_AI_VORTEX             21   /* enemy: pull hazard */
#define DM2_AI_FLAME_ORB          22   /* enemy: fire hazard */
#define DM2_AI_CAVE_BAT           23   /* enemy: fast mover */
#define DM2_AI_GLOP               24   /* enemy: w0_6_7 worm/glop */
#define DM2_AI_GIGGLER            26   /* enemy: steal (AI_ATTACK_FLAGS__STEAL) */
#define DM2_AI_THICKET_THIEF      27   /* enemy: steal (AI_ATTACK_FLAGS__STEAL) */
#define DM2_AI_WORM               28   /* enemy: w0_6_7 */
#define DM2_AI_TREANT             29   /* enemy: tree gorgon */
#define DM2_AI_LORD_DRAGOTH       30   /* boss: primary antagonist */
#define DM2_AI_MERCHANT           33   /* NPC: shop/trading */
#define DM2_AI_DRAGOTH_MINION     34   /* evil: Dragoth spawn */
#define DM2_AI_ARCHER_GUARD       36   /* enemy: AI_ATTACK_FLAGS__SHOOT */
#define DM2_AI_MAGICK_REFLECTOR   37   /* enemy: w0_1_1 reflector */
#define DM2_AI_POWER_CRYSTAL      38   /* enemy: machine */
#define DM2_AI_SPECTRE            41   /* enemy: ghost type */
#define DM2_AI_VEXIRK             48   /* enemy: Vexirk race (w0_4_4) */
#define DM2_AI_AXEMAN             44   /* enemy: melee */
#define DM2_AI_SKELETON           50   /* enemy: melee */
#define DM2_AI_AMPLIFIER          51   /* enemy: AI_ATTACK_FLAGS__FIREBALL */
#define DM2_AI_WOLF               52   /* enemy: fast */
#define DM2_AI_PIT_GHOST          53   /* enemy: invisible */
#define DM2_AI_DOOR_GHOST         54   /* enemy: ghost variant */
#define DM2_AI_VEXIRK_KING        55   /* boss: elite Vexirk */
#define DM2_AI_GHOST              61   /* enemy: ghost */
#define DM2_AI_FLYING_CHEST       58   /* enemy: mobile trap */
#define DM2_AI_MUMMY              46   /* enemy: poison */

/* ── CCM command dispatch ────────────────────────────────────────────────
 * Source: SKULLWIN/c_creature.cpp: DM2_PROCEED_CCM */

/* b_1a state register written directly by action handlers.
 * Instance struct (sk1c9a02c3 in SKULLWIN): b_1a (primary), b_17 (secondary) */

/* ── Creature spawn helpers ──────────────────────────────────────────────
 * Source: SkWinCore.cpp:16815-16936
 * ALLOC_NEW_CREATURE(type, mult, dir, x, y) — HP scaled by healthMultiplier
 * CREATE_MINION(type, mult, dir, x, y, map, missile, searchdir) — multi-map spawn */

int  dm2_v1_creature_ai_index_count(void);
const char *dm2_v1_creature_ai_name(int ai_index);
const DM2_AIDefinition *dm2_v1_creature_ai_spec(int creature_type);
int  dm2_v1_creature_attacks_party(int ai_index, int distance);
int  dm2_v1_creature_resolves_spell(int ai_index, uint16_t attack_flags);
const char *dm2_v1_creature_source_evidence(void);

#endif /* FIRESTAFF_DM2_V1_CREATURE_H */