#ifndef FIRESTAFF_DM2_V1_CREATURE_H
#define FIRESTAFF_DM2_V1_CREATURE_H
#include <stdint.h>
#include "dm2_v1_asset_loader.h"

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
 * Source: skproject/SKULLWIN/c_creature.cpp:2930-3212 DM2_PROCEED_CCM
 * b_1a written directly by action handlers — no explicit next-state field.
 * 2026-07-19 DM2-005 follow-up: values aligned to the source b_1a
 * dispatch matrix, bound verbatim in dm2_v1_ccm_dispatch_pc34_compat.
 * Previous legacy values diverged from the source (e.g. CAST_SPELL was
 * 0x15 vs source 0x27/0x28, CREATURE_ATTACKS_PARTY was 0x17 vs source
 * 0x08/0x26, SHOOT_ITEM was 0x0d vs source 0x0e/0x0f). */

#define DM2_CCM_WALK_NOW              0x01  /* WALK_NOW (skip00387: 0x01/0x02/0x09) */
#define DM2_CCM_WALK_CONT             0x02  /* WALK_NOW group; movement continuation */
#define DM2_CCM_CCM03                 0x03  /* DM2_CREATURE_CCM03 (0x03/0x04) */
#define DM2_CCM_JUMPS                 0x05  /* DM2_CREATURE_JUMPS */
#define DM2_CCM_CCM06                 0x06  /* DM2_CREATURE_CCM06 (0x06/0x07) */
#define DM2_CCM_CREATURE_ATTACKS_PARTY 0x08 /* ATTACKS_PARTY (skip00388: 0x08/0x26) */
#define DM2_CCM_STEAL_FROM_CHAMPION   0x0a  /* DM2_CREATURE_STEAL_FROM_CHAMPION */
#define DM2_CCM_CCM0B                 0x0b  /* DM2_CREATURE_CCM0B */
#define DM2_CCM_CCM0C                 0x0c  /* DM2_CREATURE_CCM0C (0x0c/0x0d) */
#define DM2_CCM_SHOOT_ITEM            0x0e  /* DM2_CREATURE_SHOOT_ITEM (0x0e/0x0f) */
#define DM2_CCM_KILL_ON_TIMER_POS     0x13  /* DM2_CREATURE_KILL_ON_TIMER_POSITION */
#define DM2_CCM_ROTATES_TARGET        0x15  /* ROTATES_TARGET_CREATURE (0x15/0x16) */
#define DM2_CCM_PLACE_MERCHANDISE     0x17  /* DM2_PLACE_MERCHANDISE */
#define DM2_CCM_TAKE_MERCHANDISE      0x18  /* DM2_TAKE_MERCHANDISE */
#define DM2_CCM_PUTS_DOWN_ITEM        0x19  /* PUTS_DOWN_ITEM (skip00386) */
#define DM2_CCM_TAKES_ITEM            0x1a  /* TAKES_ITEM (skip00389) */
#define DM2_CCM_CAST_SPELL            0x27  /* DM2_CREATURE_CAST_SPELL (0x27/0x28) */
#define DM2_CCM_EXPLODE_OR_SUMMON     0x3d  /* EXPLODE_OR_SUMMON (0x3d-0x40) */

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
/*
 * Data-backed AI-spec flag accessor — binds
 * DM2_QUERY_CREATURE_AI_SPEC_FLAGS (skproject/SKULLWIN/c_record.cpp:1346-1349)
 * over DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD (c_record.cpp:1351-1354):
 * GDAT CREATURES word field 0x05 of the creature type indexes the 36-byte
 * AIDefinition table (source table1d296c), whose word@0 holds the flags.
 * The firestaff GDAT loader (dm2_v1_creature_load_ai_table_from_gdat,
 * the proven EXTENDED_LOAD_AI_DEFINITION path SkWinCore.cpp:233-400)
 * already captures that indirection per creature type; this accessor
 * follows it.  Returns 1 and stores the flags word in *out_flags when
 * the type's AI row was loaded from the current GDAT session; returns 0
 * (fail-closed, *out_flags zeroed) for unloaded/out-of-range types or a
 * NULL out-param.  Unlike dm2_v1_creature_ai_spec (a capped-index legacy
 * view kept for its existing consumers), this accessor never invents a
 * flags word for a type the session did not define.
 */
int  dm2_v1_creature_ai_spec_flags(int creature_type, uint16_t *out_flags);
/*
 * Data-backed full AIDefinition row accessor — same provenance chain as
 * dm2_v1_creature_ai_spec_flags (c_record.cpp:1351-1354: CREATURES
 * word@5 -> AIDefinition row over the proven GDAT extended-mode table).
 * Needed by consumers of row fields beyond the flags word, e.g.
 * DM2_ATTACK_CREATURE's BaseHP percentage probe (c_creature.cpp:420-423
 * reads aidef word@4).  Returns 1 and stores the row pointer in
 * *out_def when the type's AI row was loaded from the current GDAT
 * session; returns 0 (fail-closed, *out_def = NULL) otherwise.  The
 * returned pointer is owned by the session table — do not free, valid
 * until the next loader/reset call.
 */
int  dm2_v1_creature_ai_spec_def(int creature_type,
                                 const DM2_AIDefinition **out_def);
/*
 * Data-backed AIDefinition BaseHP (word@4) accessor — same provenance
 * chain as dm2_v1_creature_ai_spec_flags.  Signature matches the CAII
 * module's DM2_V1_CaiiWordValueFn provider hook so sessions can wire it
 * directly (dm2_v1_caii_set_ai_base_hp_fn) for DM2_ATTACK_CREATURE's
 * aggro percentage probe (c_creature.cpp:420-423).  Returns 1 and
 * stores the word when the type's AI row was loaded; 0 (fail-closed,
 * *out_hp zeroed) otherwise.
 */
int  dm2_v1_creature_ai_base_hp(int creature_type, uint16_t *out_hp);
/*
 * Data-backed AIDefinition Defense (byte @8) accessor — same provenance
 * chain as dm2_v1_creature_ai_spec_flags (c_record.cpp:1351-1354: CREATURES
 * word@5 -> AIDefinition row over the proven GDAT extended-mode table).
 * This is the defense word c_engage.cpp melee resolution subtracts through
 * the source damage formula (255 = undestroyable).  Signature matches the
 * combat module's DM2_V1_CombatCreatureDefenseFn provider hook so sessions
 * can wire it directly (dm2_v1_combat_bind_creature_defense_fn).  Returns 1
 * and stores the byte when the type's AI row was loaded from the current
 * GDAT session; returns 0 (fail-closed, *out_defense zeroed) otherwise. */
int  dm2_v1_creature_ai_defense(int creature_type, uint16_t *out_defense);
/*
 * Data-backed GDAT CREATURES word field 0x01 accessor — the source
 * indexes table1d607e with DM2_QUERY_GDAT_CREATURE_WORD_VALUE(type, 1)
 * (e.g. c_creature.cpp:441 + 612, c_record.cpp:1387).  The AI table
 * loader captures the word per creature type alongside the drop words
 * (skcrture.cpp reads CREATURES word fields directly).  Returns 1 and
 * stores the word when the session defined it; 0 (fail-closed,
 * *out_word zeroed) otherwise.
 */
int  dm2_v1_creature_gdat_word1(int creature_type, uint16_t *out_word);
/* Replaces the current extended-mode table with rows from this GDAT session.
 * Rows absent from the supplied CREATURE_AI category are cleared and cannot
 * retain behavior from a previous graphics session. */
int  dm2_v1_creature_load_ai_table_from_gdat(const DM2_V1_AssetLoader *loader);
/* Explicit field import used by source-shape tests.  Production cannot infer
 * the CCM stream field from decodable bytes; it remains unavailable until the
 * SKProject record owner is bound. */
int  dm2_v1_creature_load_ccm_programs_from_gdat(const DM2_V1_AssetLoader *loader,
                                                  int field);
/* Fixture-only field probe.  Normal builds always return zero and leave no
 * CCM program installed rather than promoting a guessed GDAT field. */
int  dm2_v1_creature_load_ccm_programs_from_gdat_auto(const DM2_V1_AssetLoader *loader,
                                                       int *out_field);
int  dm2_v1_creature_loaded_ccm_program_count(void);
int  dm2_v1_creature_loaded_ccm_program_field(void);
void dm2_v1_creature_reset_ai_table(void);
void dm2_v1_creature_reset_ccm_programs(void);
int  dm2_v1_creature_attacks_party(int ai_index, int distance);
int  dm2_v1_creature_resolves_spell(int ai_index, uint16_t attack_flags);
const char *dm2_v1_creature_source_evidence(void);

/* ── Creature instance lifecycle ────────────────────────────────────────────
 * Source: SkWinCore.cpp:16815-16936 (ALLOC_NEW_CREATURE, CREATE_MINION)
 *         SKULLWIN/c_creature.cpp: DM2_PROCEED_CCM (CCM b_1a dispatch)
 *         SKULLWIN/c_ai.cpp: DM2_THINK_CREATURE (NPC planning tick)
 *
 * DM2 creature instances are spawned from GDAT creature definitions.
 * Each instance carries: position, direction, HP (scaled by healthMultiplier),
 * CCM primary state (b_1a), secondary state (b_17), and world/map association.
 * Instance count is bounded: MAX_CREATURE_INSTANCES per dungeon map. */

#define DM2_MAX_CREATURE_INSTANCES  64
#define DM2_MAX_ACTIVE_CREATURES    32   /* creatures with CCM state != IDLE */

typedef struct {
    int instance_id;        /* unique instance ID (0-63) */
    int ai_index;          /* AIDefinition table index (0–63) */
    int world_x;           /* world/map X coordinate */
    int world_y;           /* world/map Y coordinate */
    int map_index;         /* dungeon map level (0=surface, 1+=indoor) */
    int direction;         /* facing: 0=N, 1=E, 2=S, 3=W */
    int hp_current;        /* current hit points */
    int hp_max;            /* maximum hit points (BaseHP * healthMultiplier) */
    uint8_t b_1a;          /* CCM primary state register */
    uint8_t b_17;          /* CCM secondary context */
    uint8_t alive;         /* 1=alive, 0=dead (pending drop removal) */
    uint8_t is_visible;    /* 1=visible, 0=invisible */
    int target_x;          /* last known target position (party) */
    int target_y;
    int attack_cooldown;   /* ticks until next attack allowed */
    int poison_ticks;      /* poison damage countdown */
    /* Render writeback is owned by the live AI instance.  The viewport must
     * consume this state rather than inventing a frame from its own clock. */
    uint32_t animation_tick;
    uint32_t render_revision;
    uint8_t animation_frame;
    /* SKProject V5 keeps this mutable sequence pair on the live creature
     * context. It is distinct from the legacy presentation frame. */
    uint16_t gdat_animation_sequence;
    uint16_t gdat_animation_info;
} DM2_V1_CreatureInstance;

/* Live CCM pool persistence.  This is deliberately the owning creature
 * module's representation: save/load must not reconstruct instances through
 * spawn helpers because that would reset CCM, animation, and revision state.
 * Source: skproject/SKULLWIN/c_creature.cpp DM2_PROCEED_CCM. */
typedef struct {
    DM2_V1_CreatureInstance instances[DM2_MAX_CREATURE_INSTANCES];
    int next_instance_id;
    int tick_counter;
} DM2_V1_CreatureLiveState;

typedef struct {
    int valid;
    int instance_id;
    int ai_index;
    int before_b_1a;
    int after_b_1a;
    int ccm_opcode;
    int ccm_result;
    int ccm_flag_attack_party;
    int ccm_flag_walk;
    int ccm_flag_steal;
    int ccm_flag_shoot;
    int ccm_flag_cast_spell;
    int ccm_flag_explode_or_summon;
    int ccm_flag_path;
    int ccm_flag_rotate;
    int ccm_flag_special;
    int ccm_flag_item;
    int ccm_requested_state;
    int ccm_target_id;
    int ccm_target_x;
    int ccm_target_y;
    int ccm_stack_top;
    int ccm_stack_value0;
    int ccm_stack_value1;
    int imported_program;
    int program_pc_before;
    int program_pc_after;
    int field_door_valid;
    int field_door_x;
    int field_door_y;
    int field_door_state;
    int field_door_open_pct;
    int field_blocks_movement;
    int field_moved;
    int field_move_distance;
    int direction_before;
    int direction_after;
    int attack_cooldown_before;
    int attack_cooldown_after;
} DM2_V1_CreatureCCMTickObserver;

typedef int (*DM2_V1_CreatureDoorReadFn)(void *user,
                                         int level,
                                         int x,
                                         int y,
                                         int *out_state,
                                         uint16_t *out_attributes);

typedef struct {
    DM2_V1_CreatureDoorReadFn read_door;
    void *user;
} DM2_V1_CreatureFieldRuntime;

void dm2_v1_creature_set_field_runtime(
    const DM2_V1_CreatureFieldRuntime *runtime);
void dm2_v1_creature_reset_field_runtime(void);
int dm2_v1_creature_door_open_pct_from_state(int door_state);

/* ── Creature instance API ───────────────────────────────────────────────── */

/* Test-only creature fixture setup. Production always returns -1 until the
 * source ALLOC_NEW_CREATURE owner is bound: it requires a live DB4 record,
 * current map, loaded record chain, AI row, and RNG rather than host-supplied
 * type/position/direction/health values. Source: SKProject
 * skcrture.cpp:6380-6430, ALLOC_NEW_CREATURE. */
int dm2_v1_creature_spawn(int ai_index, int world_x, int world_y,
                          int map_index, int direction, int health_multiplier);

/* Test-only legacy creature-fixture tick. It is not a live DM2 runtime
 * route: M11 requires source-owned DB4, CAII and command-stream state before
 * DM2_THINK_CREATURE / DM2_PROCEED_CCM can run. This helper only advances the
 * isolated fixture pool used by focused tests and probes. */
void dm2_v1_creature_tick(void);

int dm2_v1_creature_last_ccm_tick(DM2_V1_CreatureCCMTickObserver *out_observer);
void dm2_v1_creature_reset_ccm_tick_observer(void);

/* dm2_v1_creature_count — return count of active (alive) creature instances */
int dm2_v1_creature_count(void);

/* dm2_v1_creature_at — get creature instance at world position (or -1) */
int dm2_v1_creature_at(int world_x, int world_y, int map_index);

/* dm2_v1_creature_deal_damage — apply damage to a creature instance.
 * Returns remaining HP. Triggers creature_death_check when HP <= 0.
 * Source: SKULLWIN/c_creature.cpp: CREATURE_TAKE_DAMAGE */
int dm2_v1_creature_deal_damage(int instance_id, int damage);

/* dm2_v1_creature_death_check — check if creature died, trigger drop+sound.
 * Called after HP drops to 0 or below. Rolls drop from 11-slot GDAT table.
 * Plays death sound (SOUND_CREATURE_DEATH=0x11) at creature position.
 * Source: SKULLWIN/c_creature.cpp, SKULLWIN/c_sound.cpp */
void dm2_v1_creature_death_check(int instance_id);

/* dm2_v1_creature_instance_hp — read HP of a creature instance */
int dm2_v1_creature_instance_hp(int instance_id);

/* dm2_v1_creature_instance_ai — read AI index of a creature instance */
int dm2_v1_creature_instance_ai(int instance_id);

/* dm2_v1_creature_get_instance — read-only access to a creature instance.
 * Returns NULL if instance_id is out of range.  Used by projectile
 * dispatch (Phase 5 wire-up) to read AI attack flags + position. */
const DM2_V1_CreatureInstance *dm2_v1_creature_get_instance(int instance_id);

/* Test-only animation fixture write. Production always returns -1: source
 * CCM/GDAT animation processing owns sequence and frame info together
 * (SKProject skcrture.cpp:1595-1658). */
int dm2_v1_creature_set_gdat_animation_state(int instance_id,
                                              uint16_t sequence,
                                              uint16_t info);

int dm2_v1_creature_export_live_state(DM2_V1_CreatureLiveState *out_state);
/* Validates a serialized live creature pool without changing global runtime
 * state. Save restore uses this before committing any session or dungeon. */
int dm2_v1_creature_live_state_valid(const DM2_V1_CreatureLiveState *state);
int dm2_v1_creature_restore_live_state(const DM2_V1_CreatureLiveState *state);

/* ── Test-only API (compiled in only when FIRESTAFF_DM2_CREATURE_TESTING=1) ──
 * Used by tests/probes to inject a synthetic AIDefinition entry so the
 * collision gate can exercise missile-redirect branches (NONMATERIAL /
 * ABSORBS_MISSILE / REFLECTOR / TURNS_MISSILE) without depending on the
 * production GDAT import. Boot attempts that import from the admitted
 * GRAPHICS.DAT session, but a row remains unavailable (and zeroed) when the
 * source profile does not expose the required CREATURE_AI fields.
 *
 * Always reset after use via dm2_v1_creature_test_clear_ai_overrides().
 *
 * Source: synthetic test scaffold only; no behavioral change in
 * production builds (the macro is undefined by default). */
#ifdef FIRESTAFF_DM2_CREATURE_TESTING
void dm2_v1_creature_test_set_ai_spec(int ai_index,
                                       const DM2_AIDefinition *spec);
void dm2_v1_creature_test_clear_ai_overrides(void);
void dm2_v1_creature_test_set_ccm_state(int instance_id,
                                        uint8_t b_1a,
                                        uint8_t b_17,
                                        int target_x,
                                        int target_y);
void dm2_v1_creature_test_reset_instances(void);
/* DM2-006: inject imported CREATURES drop words (fields 0x0A..0x14) for
 * one creature type, mirroring the GDAT loader path so the death-drop
 * gate can exercise source-ordered resolution without real assets. */
void dm2_v1_creature_test_set_drop_slots(int creature_type,
                                         const uint16_t slot_words[11]);
#endif /* FIRESTAFF_DM2_CREATURE_TESTING */

/* ── Death/drop observer (Phase 5 followup, 2026-06-22) ────────────────
 * The death_check() flow plays a sound, marks the instance dead, and rolls
 * a drop entry from the 11-slot GDAT table.  Without a deterministic
 * observer the drop result is discarded, which makes "creature dies → loot
 * state" impossible to gate.  These accessors expose the most recent death
 * event so the CTest gate can assert the loot-state contract:
 *
 *   1. spawn a creature → deal_damage until hp==0 → tick → death_check
 *      fires. Only imported CREATURES GDAT fields 0x0A..0x14 can produce
 *      drop_observer items; an unbound session reports no generated loot.
 *      → death_observer_count incremented
 *
 *   2. spawn non-Thorn-Demon (Cavern Bat, AI 23) → kill
 *      → drop_observer.item_id == 0 (no drop)
 *      → drop_observer.dropped == 0 (kill landed, but no loot)
 *      → death_observer_count still incremented (death happened)
 *
 * Source-locked against SKULL.ASM death dispatch (sha256 a2a04b0e...)
 * + skproject/SKWIN/SkWinCore.cpp:16815-16936 ALLOC_NEW_CREATURE
 * + SKWin.GDAT2.InternalCodes.txt creature category 0x0A (11 drop slots)
 * + ReDMCSB DEFS.H C040-equivalent dead-instance sentinel. */

typedef struct {
    int instance_id;    /* instance that just died (0..DM2_MAX_CREATURE_INSTANCES-1) */
    int ai_index;       /* creature AI index at death time */
    int world_x;        /* world X coordinate at death */
    int world_y;        /* world Y coordinate at death */
    int map_index;      /* dungeon map index at death */
    int dropped;        /* 1=drop entry non-empty, 0=no drop */
    int item_id;        /* GDAT item ID (0 if dropped==0) */
    int count;          /* drop count (0 if dropped==0) */
    /* DM2-006 source-ordered drop resolution (skcrture.cpp:2084-2118).
     * source_ordered==1 when GDAT CREATURES drop words (fields 0x0A-0x14)
     * were bound for this creature type and the source slot loop ran;
     * source_slots_admitted counts non-zero slot words;
     * source_total_items is the summed final item count. */
    int source_ordered;
    int source_slots_admitted;
    int source_total_items;
} DM2_V1_CreatureDeathDropObserver;

/* dm2_v1_creature_drop_slots_loaded — 1 when GDAT CREATURES drop words
 * (fields 0x0A..0x14) were imported for creature_type by
 * dm2_v1_creature_load_ai_table_from_gdat. */
int dm2_v1_creature_drop_slots_loaded(int creature_type);

/* dm2_v1_creature_drop_slot_word — raw imported slot word for
 * creature_type slot (0..10 → GDAT field 0x0A..0x14), 0 when absent. */
uint16_t dm2_v1_creature_drop_slot_word(int creature_type, int slot);

/* dm2_v1_creature_drop_rng_reset — rewind the skproject LCG stream used
 * by source-ordered drop resolution (c_random.cpp init state 0). */
void dm2_v1_creature_drop_rng_reset(void);

/* dm2_v1_creature_last_death_drop — read the most recent death-drop observer.
 * Returns 1 if a death has been observed since reset, 0 otherwise.
 * When 0 is returned, out_observer is zero-initialized. */
int dm2_v1_creature_last_death_drop(DM2_V1_CreatureDeathDropObserver *out_observer);

/* dm2_v1_creature_death_observer_count — total death events observed
 * (including events that produced no drop).  Monotonic; reset only
 * by dm2_v1_creature_reset_death_observer. */
int dm2_v1_creature_death_observer_count(void);

/* dm2_v1_creature_reset_death_observer — clear the last-death observer
 * and zero the monotonic count.  Tests must call this before exercising
 * a fresh death sequence so previous observations don't leak in. */
void dm2_v1_creature_reset_death_observer(void);

#endif /* FIRESTAFF_DM2_V1_CREATURE_H */
