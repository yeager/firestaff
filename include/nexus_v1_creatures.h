
#ifndef NEXUS_V1_CREATURES_H
#define NEXUS_V1_CREATURES_H
#include <stdint.h>
#include "nexus_v1_dungeon.h"

/* Nexus creature types — loaded from MNS files.
 * Each creature has a DMDF 3D model + stats. */

#define NEXUS_MAX_CREATURE_TYPES 64
#define NEXUS_MAX_ACTIVE_CREATURES 128
#define NEXUS_MAX_ACTOR_BINDINGS 64

typedef struct {
    char name[32];
    char model_file[32];  /* e.g. "SCORPION.MNS" */
    int health, attack, defense, speed;
    int experience_value;
    int detection_range;  /* CRET byte 14: how far creature detects party */
    int ai_type;          /* CRET byte 0: AI behavior index (0-15) */
    int attack_count;     /* CRET byte 1: number/type of attacks */
    int behav_flags;      /* CRET byte 3: movement/behavior bit flags */
    int ranged_type;      /* CRET byte 4: ranged attack type (0=melee) */
    int poison;           /* CRET byte 13: poison damage on hit */
    int model_index;      /* index into engine->models[] */
    int cret_bound;       /* 1 when CRET stats loaded from RLOWFIX.BIN */
    uint8_t cret_raw[96]; /* raw CRET record from RLOWFIX.BIN (96 bytes per creature) */
    /* Real *.MNS model metadata binding.  Set only after the named file has
     * been opened from the configured Nexus data directory and authenticated
     * as a DMDF container; stays 0 (fail-closed) when the file is absent or
     * malformed.  Source: docs/NEXUS_FILE_CLASSIFICATION.md MNS inventory,
     * nexus_v1_dmdf_model.c DMDF magic check. */
    int mns_bound;
    uint32_t mns_size;
    uint64_t mns_fnv1a64;
    uint32_t ai_func_ptr;     /* SH-2 function pointer from DM.BIN 0x0383A8 */
} Nexus_CreatureType;

typedef struct {
    int type_index;   /* -1 = unbound actor (stats not source-locked) */
    int health;
    int x, y;       /* map position */
    int facing;
    int alive;
    int state;       /* 0=idle, 1=patrol, 2=chase, 3=attack, 4=flee */
    int ai_timer;
    int level;       /* dungeon level this creature belongs to */
    /* Real DGN Structure1A actor-record provenance.  actor_ref_bound is 1
     * only when the creature was spawned from an authenticated LEV*.DGN
     * Structure1A model record with kind byte 01h/02h and a unique
     * Structure1B owner cell.  structure3_model_index retains the record's
     * documented Structure3 model reference; model_signature is the
     * FNV-1a64 of the extracted Structure3 mesh rows (0 when the retained
     * DGN buffer was unavailable).  hidden mirrors the Structure1B
     * invisible-by-default bit (DMWeb: set only for the Grey Lord on
     * LEV1.DGN); hidden actors stay dormant until a proven trigger exists.
     * Source: DMWeb DGN Structure1A/Structure1B cell format. */
    int actor_ref_bound;
    int hidden;
    uint8_t structure3_model_index;
    uint64_t model_signature;
    /* Wander target for patrol state.  When set to a non-negative value the
     * creature shuffles toward that square instead of standing still.
     * Source: DM1 CREATURE.C idle/wander AI (F0209_GROUP_ProcessEvents29to41). */
    int wander_target_x;
    int wander_target_y;
    int wander_timer; /* ticks until next wander target pick */
} Nexus_Creature;

/* Evidence-gated binding between a DGN actor model (Structure3 index +
 * mesh signature) and a roster creature type.  Bindings are registered
 * only where source evidence identifies the model; unbound actors spawn
 * with type_index -1 and keep AI/combat fail-closed. */
typedef struct {
    uint64_t model_signature;
    uint8_t structure3_model_index;
    int type_index;
} Nexus_V1_CreatureActorBinding;

typedef struct {
    Nexus_CreatureType types[NEXUS_MAX_CREATURE_TYPES];
    int type_count;
    Nexus_Creature active[NEXUS_MAX_ACTIVE_CREATURES];
    int active_count;
    int alarm_timer; /* ticks remaining while alarm keeps creatures alerted */
    Nexus_V1_CreatureActorBinding actor_bindings[NEXUS_MAX_ACTOR_BINDINGS];
    int actor_binding_count;
    /* Number of live creatures spawned from authenticated DGN actor records.
     * When this is non-zero, synthetic probe spawn fixtures stay blocked. */
    int real_actor_spawn_count;
} Nexus_V1_CreatureManager;

void nexus_v1_creatures_init(Nexus_V1_CreatureManager *mgr);

/* Production Nexus init.  The legacy type table contains fixture-only
 * combat values; live Nexus must not promote those values without a
 * Saturn-owned stat source. */
void nexus_v1_creatures_init_production(Nexus_V1_CreatureManager *mgr);
int nexus_v1_creature_spawn(Nexus_V1_CreatureManager *mgr, int type_idx, int x, int y, int dir);
int nexus_v1_creature_spawn_on_level(Nexus_V1_CreatureManager *mgr, int type_idx, int x, int y, int dir, int level);
void nexus_v1_creatures_tick(Nexus_V1_CreatureManager *mgr, int party_x, int party_y,
                              const uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE],
                              int map_index);

/* Alert all creatures on a level (e.g. on alarm square trigger).
 * Source: DM1 MOVESENS.C F0277 — ALARM sets creature alert=255. */
void nexus_v1_creatures_alert_all(Nexus_V1_CreatureManager *mgr, int level);

/* Manhattan distance between two map positions. */
int nexus_v1_creature_distance(int ax, int ay, int bx, int by);

/* Creature attacks adjacent party. Returns 1 if attack landed.
 * damage output set to damage amount dealt.
 * Source: DM1 CREATURE.C creature attack resolution, F0209 creature AI. */
int nexus_v1_creature_attack(Nexus_V1_CreatureManager *mgr, int creature_idx,
                               int champion_defense, int *out_damage);

/* ═══════════════════════════════════════════════════════════════════
 * Real DGN actor-record spawn and type binding
 * ═══════════════════════════════════════════════════════════════════ */

/* Clear the active creature pool (level transition) while keeping the
 * roster types and evidence-gated actor bindings.
 * Source: ReDMCSB GROUP.C F0183 active-group pool reset on map load. */
void nexus_v1_creatures_reset_active(Nexus_V1_CreatureManager *mgr);

/* Bind a roster creature type to its real *.MNS model metadata.  Opens
 * the file, requires the DMDF magic, and records size + FNV-1a64.
 * Returns 1 when bound, 0 when the file is absent or not a DMDF
 * container (fail-closed; type keeps mns_bound == 0).
 * Source: docs/NEXUS_FILE_CLASSIFICATION.md MNS inventory,
 *         nexus_v1_dmdf_model.c nexus_v1_dmdf_is_valid(). */
int nexus_v1_creature_bind_mns_metadata(Nexus_V1_CreatureManager *mgr,
                                        int type_idx, const char *path);

/* Register an evidence-gated actor-model binding.  Returns 0 on success,
 * -1 on bad arguments or a full binding table. */
int nexus_v1_creature_bind_actor_model(Nexus_V1_CreatureManager *mgr,
                                       uint64_t model_signature,
                                       uint8_t structure3_model_index,
                                       int type_idx);

/* Resolve an actor model to a roster type.  Exact signature+index match
 * first; a zero signature matches on the Structure3 index alone (the
 * retained DGN buffer was unavailable at spawn time).  Returns the type
 * index or -1 when no proven binding exists. */
int nexus_v1_creature_actor_type_for(const Nexus_V1_CreatureManager *mgr,
                                     uint64_t model_signature,
                                     uint8_t structure3_model_index);

/* Spawn a creature from an authenticated DGN Structure1A actor record.
 * The type is resolved through the evidence-gated binding registry; when
 * no proven binding exists the actor spawns with type_index -1 (fail-
 * closed: no AI movement, no combat) until a binding is registered and
 * nexus_v1_creature_rebind_unbound() runs.  Increments
 * real_actor_spawn_count, which blocks synthetic spawn fixtures.
 * Source: DMWeb DGN Structure1A model records (kind byte 01h/02h),
 *         ReDMCSB GROUP.C F0183 active-group slot bounds. */
int nexus_v1_creature_spawn_actor(Nexus_V1_CreatureManager *mgr,
                                  int x, int y, int dir, int level,
                                  int hidden, uint8_t structure3_model_index,
                                  uint64_t model_signature);

/* Re-resolve the type of every unbound actor through the binding
 * registry.  Returns the number of actors that gained a proven type. */
int nexus_v1_creature_rebind_unbound(Nexus_V1_CreatureManager *mgr);

/* Load creature stats from RLOWFIX.BIN CRET section.  Reads the 30-entry
 * CRET table (96 bytes per creature) and wires HP/attack/defense/speed
 * into the roster types.  Returns the number of creatures bound.
 * Source: RLOWFIX.BIN CRET section at offset 0xF2B4. */
int nexus_v1_creatures_load_cret(Nexus_V1_CreatureManager *mgr,
                                 const char *rlowfix_path);

/* CRET record field offsets (96-byte record from RLOWFIX.BIN). */
#define NEXUS_CRET_OFF_HP          7
#define NEXUS_CRET_OFF_ATTACK_BASE 8
#define NEXUS_CRET_OFF_DEXTERITY   9
#define NEXUS_CRET_OFF_STRENGTH   10
#define NEXUS_CRET_OFF_DAMAGE     11
#define NEXUS_CRET_OFF_ARMOR      12
#define NEXUS_CRET_OFF_POISON     13
#define NEXUS_CRET_OFF_INDEX      15
#define NEXUS_CRET_OFF_DETECTION  14
#define NEXUS_CRET_OFF_INDEX      15
#define NEXUS_CRET_OFF_ANIM_FRAMES 16
#define NEXUS_CRET_OFF_MOVE_TYPE  17
#define NEXUS_CRET_OFF_AI_PARAM   18
#define NEXUS_CRET_OFF_AI_RANGE   19
#define NEXUS_CRET_OFF_SPEED      49
#define NEXUS_CRET_RECORD_SIZE    96
#define NEXUS_CRET_COUNT          30

/* CRET byte 0: AI behavior type (0-15).
 * Source: RLOWFIX.BIN CRET records, cross-referenced with DM.BIN
 * yam\crenet.c AI function pointer table at 0x0383A8. */
#define NEXUS_CRET_OFF_AI_TYPE    0
/* CRET byte 1: attack count or projectile type (0-17). */
#define NEXUS_CRET_OFF_ATTACK_COUNT 1
/* CRET byte 3: movement/behavior flags (bit field: 0x10,0x30,0x40,0x50,0x80). */
#define NEXUS_CRET_OFF_BEHAV_FLAGS 3
/* CRET byte 4: ranged attack type (0=melee, 4/6/10=ranged variants). */
#define NEXUS_CRET_OFF_RANGED_TYPE 4

/* DM.BIN yam\cresub.c combat data tables (0x03B5A0-0x03B5F0).
 * These are extracted from the real Saturn binary; no synthetic values. */

/* Attack type permutation (8 entries, DM.BIN 0x03B5A0).
 * Maps creature attack category to stat lookup index. */
#define NEXUS_COMBAT_ATTACK_PERM_COUNT 8

/* Experience level thresholds (8 entries, DM.BIN 0x03B5D8).
 * Creature level boundaries: 40, 80, 120, 160, 200, 240, 277. */
#define NEXUS_COMBAT_XP_THRESHOLD_COUNT 8

/* Stat bitmask table (6 entries, DM.BIN 0x03B5C6).
 * Powers of 2: 1, 2, 4, 8, 16, 32. Used for wound/stat category flags. */
#define NEXUS_COMBAT_STAT_BITS_COUNT 6

/* Special combat item IDs (3 entries, DM.BIN 0x03B5D2). */
#define NEXUS_COMBAT_SPECIAL_ITEM_COUNT 3

/* Retrieve DM.BIN creature combat tables.  Returns pointers to static
 * tables extracted from the real Saturn binary.
 * Source: DM.BIN yam\cresub.c data region 0x03B5A0-0x03B5F0. */
const uint8_t *nexus_v1_combat_attack_perm(void);
const uint8_t *nexus_v1_combat_xp_thresholds(void);
const uint8_t *nexus_v1_combat_stat_bits(void);
const uint16_t *nexus_v1_combat_special_items(void);

#endif
