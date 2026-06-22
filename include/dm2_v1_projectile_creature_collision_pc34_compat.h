#ifndef FIRESTAFF_DM2_V1_PROJECTILE_CREATURE_COLLISION_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_PROJECTILE_CREATURE_COLLISION_PC34_COMPAT_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * dm2_v1_projectile_creature_collision_pc34_compat.h
 *
 * Phase 5 (creature/combat parity) — narrow regression gate for the
 * DM2 V1 projectile-vs-creature collision.  This module is the
 * DM2→creature bridge that complements dm2_v1_projectile_pc34_compat.c
 * (the DM2→DM1 projectile *launch* bridge):
 *
 *   - dm2_v1_projectile_pc34_compat.c        dispatches a creature attack
 *     via F0810_PROJECTILE_Create_Compat, storing the projectile in the
 *     module-owned s_projectile_list.
 *   - dm2_v1_projectile_creature_collision_pc34_compat.c (this module)
 *     resolves what happens when one of those live projectiles reaches
 *     a square containing a creature instance: deterministic damage,
 *     projectile despawn, optional death event.
 *
 * Why a dedicated module (and not folded into the launch module)?
 *
 *   1. The launch module is hot-path: every creature ranged attack creates
 *      a projectile. The collision module runs on the F0811 advance path
 *      (per-tick as the projectile moves). Keeping them separate avoids
 *      coupling dispatch + resolve.
 *   2. DM2 has 4 missile-redirect flags (REFLECTOR / ABSORBS_MISSILE /
 *      TURNS_MISSILE / NONMATERIAL) that govern projectile outcome
 *      BEFORE the damage pass — those branches are easier to test in
 *      isolation than as a state machine inside the launch module.
 *   3. The deterministic damage formula (no RNG) lets CTest pin exact
 *      damage values for the regression gate.
 *
 * ── Source-lock anchors ────────────────────────────────────────────
 *   ReDMCSB PROJEXPL.C:76-92     (F0212: projectile live, first move +1 tick)
 *   ReDMCSB PROJEXPL.C:689-690   (F0219: C48 ignore-impacts-first-movement)
 *   ReDMCSB PROJEXPL.C:1554-1600 (F0232_GROUP_IsDoorDestroyedByAttack — door damage cap)
 *   ReDMCSB GROUP.C:1695-1770    (F0207: creature attack projectile payload)
 *   ReDMCSB GROUP.C:2376-2387    (F0209: visible row/column triggers F0207 attack)
 *   ReDMCSB DEFS.H:1039-1047     (M036/M037 door state macros)
 *   ReDMCSB DEFS.H:1567-1572     (DOOR_INFO {Attributes, Defense})
 *   skproject/SKULLWIN/c_creature.cpp (DM2_PROCEED_CCM, c_creature.h b_1a)
 *   skproject/SKULLWIN/c_combat.cpp:401-420 (kill threshold: damage >= hp)
 *   skproject/SKWIN/SkWinCore.cpp:10479-10561 (AI_W30_TURNS_MISSILE check)
 *   skproject/SKWIN/DME.h:1545-1560 (w0AIFlags behavior bit definitions)
 *   memory_projectile_pc34_compat.h           (F0813 despawn + list contract)
 *
 * ── DM2 vs DM1 difference captured here ────────────────────────────
 *   DM1: PROJEXPL.C:631-654 routes projectile-impact-to-creature through
 *        F0817_PROJECTILE_BuildHitCreatureAction_Compat, which builds a
 *        COMBAT_ACTION_APPLY_DAMAGE_GROUP payload handled by the DM1
 *        combat engine.  DM2 has the same F0817 contract (shared
 *        memory_projectile module), but the F0820_RESOLVE collision path
 *        in DM2 runs an additional missile-redirect pass for the four
 *        DM2-only AI flags (REFLECTOR / ABSORBS_MISSILE / TURNS_MISSILE
 *        / NONMATERIAL) before the F0817 damage call.  This module owns
 *        that DM2-specific pre-pass.
 *
 * ── Why deterministic damage?  ─────────────────────────────────────
 *   F0815_PROJECTILE_ComputeImpactAttack_Compat in DM1 already produces
 *   a deterministic raw attack value for the projectile.  DM2's formula
 *   for the *creature-side* damage applied to HP is also deterministic
 *   in SKULL.ASM (no per-impact RNG roll; only the F0822 explosion AoE
 *   uses an RNG roll for partial-radius ticks).  So we can pin exact
 *   damage values per (projectile_subtype, creature_armor_class) pair
 *   for CTest regression coverage.
 * ================================================================ */

/* ── Outcome enum ──────────────────────────────────────────────────
 * Mirrors the SKULL.ASM missile-redirect dispatch order:
 *   1. NONMATERIAL → projectile passes through without dealing damage.
 *   2. ABSORBS_MISSILE → projectile despawned, no damage, no event.
 *   3. REFLECTOR → projectile despawned, no damage (DM2 doesn't
 *                  reflect back; REFLECTOR creatures reflect spells,
 *                  see c_creature.cpp:401-420).
 *   4. TURNS_MISSILE → creature re-targets projectile (placeholder for
 *                      future routing); for now treated as HIT (same
 *                      damage path).
 *   5. Default → HIT: damage applied, projectile despawned, optional
 *                  kill event. */
typedef enum {
    DM2_PROJ_CREATURE_OUTCOME_INVALID        = 0,
    DM2_PROJ_CREATURE_OUTCOME_HIT            = 1,
    DM2_PROJ_CREATURE_OUTCOME_NONMATERIAL    = 2, /* projectile passes through */
    DM2_PROJ_CREATURE_OUTCOME_ABSORBED       = 3, /* projectile despawned, no damage */
    DM2_PROJ_CREATURE_OUTCOME_REFLECTED      = 4, /* projectile despawned (DM2 spell-reflector) */
    DM2_PROJ_CREATURE_OUTCOME_REDIRECTED     = 5, /* projectile would be turned (placeholder HIT path) */
} DM2_ProjectileCreatureOutcome;

/* ── Result struct ─────────────────────────────────────────────────
 * Returned by dm2_v1_projectile_creature_collision_resolve().
 *
 * All fields are deterministic given the same input state.  No RNG
 * is consumed.  The kill_event_emitted flag is 1 when the post-damage
 * HP reached 0 and dm2_v1_creature_death_check() is queued for the
 * next tick.  The death sound + drop are owned by dm2_v1_creature.c —
 * this module only flags the kill_event for downstream tick handling. */
typedef struct {
    DM2_ProjectileCreatureOutcome outcome;   /* which path was taken */
    int  accepted;                          /* 1 if collision was resolved */
    int  creature_instance_id;              /* index of the hit creature */
    int  creature_ai_index;                 /* AI index (snapshot) */
    int  hp_before;                         /* HP before damage */
    int  hp_after;                          /* HP after damage */
    int  damage_dealt;                      /* 0 for non-HIT outcomes */
    int  armor_class;                       /* creature's defense snapshot */
    int  projectile_slot;                   /* slot that was despawned (-1 if not despawned) */
    int  projectile_despawned;              /* 1 if F0813 was called */
    int  kill_event_emitted;                /* 1 if HP reached 0 → death_check queued */
    int  impact_attack_input;               /* raw attack input */
    int  impact_attack_effective;           /* post-armor attack value */
} DM2_V1_ProjectileCreatureCollisionResult;

/* ── Public API ────────────────────────────────────────────────────
 *
 * dm2_v1_projectile_creature_collision_resolve — resolve a collision
 * between a live projectile slot and the creature at the projectile's
 * world position.  If no creature is present at the slot's world coords,
 * the call returns an INVALID outcome with accepted=0.
 *
 * Inputs:
 *   projectile_slot — slot index in the dm2_v1_projectile_pc34_compat
 *                     module-owned s_projectile_list (must be >= 0).
 *   impact_attack   — deterministic raw attack value (typically the
 *                     F0815_PROJECTILE_ComputeImpactAttack output, or
 *                     a hand-set value for tests).  Must be >= 1.
 *
 * Side effects:
 *   - Mutates the target creature's HP via dm2_v1_creature_deal_damage().
 *   - Despawns the projectile via F0813_PROJECTILE_Despawn_Compat()
 *     (this is the canonical "creature took the hit, projectile is
 *     consumed" path).
 *   - If the post-damage HP <= 0, calls dm2_v1_creature_death_check()
 *     immediately (matches DM2's synchronous creature death handling
 *     — the death sound + drop emit in the same tick as the kill).
 *
 * Determinism:
 *   No RNG is consumed.  damage_dealt = max(1, impact_attack - armor/2).
 *   This matches the SKULL.ASM:10620-10710 ranged-damage contract
 *   against creatures (impactAttack - ArmorClass/2, floored at 1).
 *
 * Returns: DM2_V1_ProjectileCreatureCollisionResult with full
 *   snapshot of the resolved collision.  Tests should assert on:
 *     outcome, damage_dealt, hp_after, kill_event_emitted,
 *     projectile_despawned, creature_instance_id. */
DM2_V1_ProjectileCreatureCollisionResult
dm2_v1_projectile_creature_collision_resolve(int projectile_slot,
                                              int impact_attack);

/* ── Observability counters ────────────────────────────────────────
 * Monotonic counters for the M11 wire-up probe. */
int  dm2_v1_projectile_creature_collision_count(void);
int  dm2_v1_projectile_creature_kill_event_count(void);
int  dm2_v1_projectile_creature_absorb_count(void);
int  dm2_v1_projectile_creature_reflect_count(void);
int  dm2_v1_projectile_creature_nonmaterial_count(void);
void dm2_v1_projectile_creature_collision_reset_counters(void);

/* ── Source evidence citation ────────────────────────────────────── */
const char *dm2_v1_projectile_creature_collision_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_PROJECTILE_CREATURE_COLLISION_PC34_COMPAT_H */
