#ifndef FIRESTAFF_DM2_V1_PROJECTILE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_PROJECTILE_PC34_COMPAT_H
#include <stdint.h>
#include "memory_projectile_pc34_compat.h"
#include "dm2_v1_creature.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * dm2_v1_projectile_pc34_compat.h — DM2 V1 Projectile Routing
 *
 * Phase 5 (creature/combat parity) source-lock.
 *
 * DM2's creature attack projectiles are routed through the DM1
 * projectile engine (memory_projectile_pc34_compat.c F0810-F0820).
 * This module is the DM2->DM1 bridge: it maps DM2 creature AI attack
 * flags + world coordinates to a DM1 ProjectileCreateInput_Compat and
 * invokes F0810_PROJECTILE_Create_Compat.
 *
 * Why reuse DM1's engine?  SKULL.ASM projectile routing is byte-for-byte
 * compatible with DM1's PROJECT.C / PROJEXPL.C routines; the only
 * differences are:
 *   1. DM2 creatures have more attack-flag bits (AI_ATTACK_FLAGS__*
 *      with 12 bits vs DM1's 8)
 *   2. DM2 spells can produce projectiles (CCM 0x15 CAST_SPELL)
 *   3. DM2 has outdoor combat where bow projectiles take -50% penalty
 *      (handled by dm2_v1_combat_apply_outdoor_modifier before launch)
 *
 * Architecture:
 *   dm2_v1_creature_attack_ranged(instance_id, target_world_x, target_world_y)
 *     -> looks up AI attack flags
 *     -> picks PROJECTILE_CATEGORY_KINETIC (bow/gun) or
 *        PROJECTILE_CATEGORY_MAGICAL (spell)
 *     -> picks projectile subtype from attack_flags
 *     -> calls F0810_PROJECTILE_Create_Compat via dm2_v1_projectile_dispatch()
 *
 * Source-lock anchors:
 *   SKULL.ASM:10620-10710  (SKULL_COMBAT_ResolveRanged)
 *   SKULL.ASM:11100-11200  (projectile routing — SKWin SKULL.ASM source-locked region)
 *   ReDMCSB PROJEXPL.C:76-92       (F0212: projectile is linked live, first move +1 tick)
 *   ReDMCSB PROJEXPL.C:689-690     (F0219: C48 ignore-impacts-first-movement)
 *   ReDMCSB GROUP.C:2376-2387      (F0209: visible same row/column triggers F0207 attack)
 *   ReDMCSB GROUP.C:1695-1770      (F0207: creature attack projectile payload)
 *   skproject/SKWIN/SkWinCore.cpp:10479-10561  (AI_W30_TURNS_MISSILE check)
 *   skproject/SKULLWIN/c_creature.cpp           (DM2_PROCEED_CCM, CCM 0x0d/0x15)
 *   memory_projectile_pc34_compat.h             (F0810-F0820 contract)
 * ================================================================ */

/* ── DM2 attack-flag → projectile-category mapping ──────────────────
 * Maps AI_ATTACK_FLAGS__* to PROJECTILE_CATEGORY_* + subtype.
 * Source: skproject/SKULLWIN/c_creature.cpp + memory_projectile_pc34_compat.h */

/* Projectile subtypes — DM2 uses DM1's PROJECTILE_SUBTYPE_* constants.
 * DM2 has no new projectile subtypes; it just has more AI_ATTACK_FLAGS__*
 * that route to the existing DM1 subtypes.  Subtype mapping:
 *   AI_ATTACK_FLAGS__SHOOT        → PROJECTILE_SUBTYPE_KINETIC_ARROW
 *   AI_ATTACK_FLAGS__FIREBALL     → PROJECTILE_SUBTYPE_FIREBALL
 *   AI_ATTACK_FLAGS__LIGHTNING    → PROJECTILE_SUBTYPE_LIGHTNING_BOLT
 *   AI_ATTACK_FLAGS__DISPELL      → PROJECTILE_SUBTYPE_HARM_NON_MATERIAL
 *   AI_ATTACK_FLAGS__POISON_CLOUD → PROJECTILE_SUBTYPE_POISON_CLOUD
 *   AI_ATTACK_FLAGS__POISON_BOLT  → PROJECTILE_SUBTYPE_POISON_BOLT
 *   AI_ATTACK_FLAGS__POISON_BLOB  → PROJECTILE_SUBTYPE_SLIME
 * Source: skproject/SKWIN/SkWinCore.cpp:27038-27096 (creature AI spell dispatch). */
#define DM2_PROJ_SUBTYPE_KINETIC_ARROW        PROJECTILE_SUBTYPE_KINETIC_ARROW
#define DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL     PROJECTILE_SUBTYPE_FIREBALL
#define DM2_PROJ_SUBTYPE_MAGICAL_LIGHTNING    PROJECTILE_SUBTYPE_LIGHTNING_BOLT
#define DM2_PROJ_SUBTYPE_MAGICAL_DISPELL      PROJECTILE_SUBTYPE_HARM_NON_MATERIAL
#define DM2_PROJ_SUBTYPE_MAGICAL_POISON_CLOUD PROJECTILE_SUBTYPE_POISON_CLOUD
#define DM2_PROJ_SUBTYPE_MAGICAL_POISON_BOLT  PROJECTILE_SUBTYPE_POISON_BOLT
#define DM2_PROJ_SUBTYPE_MAGICAL_POISON_BLOB  PROJECTILE_SUBTYPE_SLIME
/* DM2 bombs (CCM 0x0d SHOOT_ITEM for bomb AI types) use FIREBALL subtype
 * with area-effect damage scaling in F0821 explosion create. */
#define DM2_PROJ_SUBTYPE_BOMB                 PROJECTILE_SUBTYPE_FIREBALL

/* ── Result of a projectile dispatch attempt ──────────────────────
 * Returned by dm2_v1_projectile_dispatch().  Mirrors the F0810 contract:
 *   - slot_index = -1 means no projectile was created (input invalid or
 *     attack flag not ranged/spell)
 *   - first_move_event fields are 0 when no event was scheduled
 *   - projectile_dispatch_count is monotonic and increments on accepted
 *     dispatches (used by the wire-up probe to verify data flow) */
typedef struct {
    int accepted;               /* 1 if a projectile was created */
    int slot_index;             /* F0810 out slot (-1 = no slot) */
    int category;               /* PROJECTILE_CATEGORY_KINETIC/MAGICAL */
    int subtype;                /* DM2_PROJ_SUBTYPE_* */
    int owner_kind;             /* PROJECTILE_OWNER_CREATURE */
    int owner_index;            /* creature instance id */
    int first_move_event_type;  /* TimelineEvent type for first move */
    int first_move_event_tick;  /* tick at which first move is scheduled */
} DM2_V1_ProjectileDispatchResult;

/* ── Phase 5 expansion: pick projectile category+subtype from attack
 *     flags (called by dm2_v1_projectile_dispatch) ────────────────────
 * Source: skproject/SKULLWIN/c_creature.cpp projectile dispatch.
 * Returns 1 if the attack flags map to a projectile (KINETIC/MAGICAL),
 * 0 if the attack is melee-only (no projectile).  Caller can then call
 * dm2_v1_projectile_dispatch with the resolved category+subtype. */
int dm2_v1_projectile_pick_category(uint16_t attack_flags,
    int *out_category, int *out_subtype);

/* ── Phase 5 expansion: dispatch one creature attack ──────────────
 * Maps creature instance_id + target coords to a ProjectileCreateInput
 * and calls F0810_PROJECTILE_Create_Compat.  The result struct
 * describes what happened (slot index, category, subtype). */
DM2_V1_ProjectileDispatchResult dm2_v1_projectile_dispatch(
    int creature_instance_id,
    int target_world_x, int target_world_y,
    int target_map_index);

/* ── Phase 5 expansion: dispatch a creature spell (CCM 0x15) ─────
 * Source: skproject/SKULLWIN/c_creature.cpp CAST_SPELL dispatch.
 * Spells are MAGICAL projectiles routed through
 * F0812_PROJECTILE_CreateFromSpellEffect_Compat. */
DM2_V1_ProjectileDispatchResult dm2_v1_projectile_dispatch_spell(
    int creature_instance_id,
    int spell_subtype,  /* DM2_PROJ_SUBTYPE_MAGICAL_* */
    int target_world_x, int target_world_y,
    int target_map_index);

/* ── Phase 5 expansion: dispatch a creature bomb throw (DM2 new) ─
 * Source: skproject/SKULLWIN/c_creature.cpp 0x0d SHOOT_ITEM for bombs.
 * Bombs are area-effect KINETIC projectiles. */
DM2_V1_ProjectileDispatchResult dm2_v1_projectile_dispatch_bomb(
    int creature_instance_id,
    int target_world_x, int target_world_y,
    int target_map_index);

/* ── Phase 5 observability ──────────────────────────────────────── */
int dm2_v1_projectile_dispatch_count(void);  /* monotonic counter */
int dm2_v1_projectile_spell_dispatch_count(void);
int dm2_v1_projectile_bomb_dispatch_count(void);
void dm2_v1_projectile_reset_counters(void);

/* ── Phase 5 expansion: projectile drain to M11 ──────────────────
 * Copies the DM2 projectile list into a caller-provided array so the
 * M11 render path (firestaff_game_loop.c, m11_game_view.c) can iterate
 * over them and draw fireballs/lightning/arrows in the V1 viewport.
 *
 * Each drained entry is a small DM2_V1_DrainedProjectile with screen-
 * ready framebuffer coordinates (pixel_x, pixel_y) computed from the
 * world (map_x, map_y) + direction.
 *
 * Source-lock:
 *   skproject/SKULLWIN/c_render.cpp   - projectile draw routine
 *   ReDMCSB DUNGEON.C:2362-2387       - F0209 visible row/column
 *   ReDMCSB PROJEXPL.C:76-92          - F0212 projectile live
 *
 * Returns the number of projectiles drained (0..max_count). */
#define DM2_DRAIN_MAX_PROJECTILES  60
typedef struct {
    int  slot_index;        /* slot in DM2 projectile list (-1 = empty) */
    int  category;          /* PROJECTILE_CATEGORY_KINETIC / MAGICAL */
    int  subtype;           /* DM2_PROJ_SUBTYPE_* */
    int  owner_kind;        /* PROJECTILE_OWNER_* */
    int  owner_index;       /* creature instance id or champion idx */
    int  map_x, map_y;      /* world coords */
    int  direction;         /* 0=N, 1=E, 2=S, 3=W */
    int  pixel_x, pixel_y;  /* framebuffer pixel coords (for M11 draw) */
    int  frame;             /* animation frame (0..7) */
    int  active;            /* 1 if visible, 0 if ended */
} DM2_V1_DrainedProjectile;

int dm2_v1_projectile_drain_to_m11(DM2_V1_DrainedProjectile *out_list,
                                    int max_count);

/* ── Phase 5 expansion: synthetic dispatch for tests ─────────────
 * Allows tests/probes to inject projectiles directly without going
 * through the creature attack pipeline.  Returns the slot index or -1. */
int dm2_v1_projectile_dispatch_synthetic(int category, int subtype,
                                          int map_x, int map_y,
                                          int map_index, int direction);

/* ── Phase 5 expansion: count active projectiles ───────────────── */
int dm2_v1_projectile_active_count(void);

/* Source evidence citation */
const char *dm2_v1_projectile_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_PROJECTILE_PC34_COMPAT_H */