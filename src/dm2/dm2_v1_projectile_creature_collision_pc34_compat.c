/* dm2_v1_projectile_creature_collision_pc34_compat.c
 *
 * Phase 5 (creature/combat parity) — DM2 V1 projectile-vs-creature
 * collision resolver.  Companion module to dm2_v1_projectile_pc34_compat.c
 * (which owns the launch/dispatch path).  This module owns the resolve
 * path: when a live DM2 projectile reaches a square with a creature
 * instance, this module decides:
 *   - which missile-redirect branch applies (NONMATERIAL / ABSORBS /
 *     REFLECTOR / TURNS_MISSILE / default HIT)
 *   - the deterministic damage value
 *   - whether to emit a kill event (HP reached 0)
 *   - whether to despawn the projectile via F0813
 *
 * Source-lock anchors:
 *   ReDMCSB PROJEXPL.C:76-92     (F0212: projectile live, first move +1 tick)
 *   ReDMCSB PROJEXPL.C:689-690   (F0219: C48 ignore-impacts-first-movement)
 *   ReDMCSB GROUP.C:1695-1770    (F0207: creature attack projectile payload)
 *   ReDMCSB GROUP.C:2376-2387    (F0209: visible row/column triggers F0207)
 *   skproject/SKULLWIN/c_creature.cpp (DM2_PROCEED_CCM, b_1a state)
 *   skproject/SKULLWIN/c_combat.cpp:401-420 (kill threshold: damage >= hp)
 *   skproject/SKWIN/SkWinCore.cpp:10479-10561 (AI_W30_TURNS_MISSILE check)
 *   skproject/SKWIN/DME.h:1545-1560 (w0AIFlags behavior bit definitions)
 *   memory_projectile_pc34_compat.h           (F0813 despawn + slot contract)
 *   memory_combat_pc34_compat.h               (COMBAT_ACTION_APPLY_DAMAGE_GROUP)
 *
 * Determinism:
 *   damage = max(1, impact_attack - armor_class / 2)
 *   No RNG is consumed.  This matches SKULL.ASM:10620-10710
 *   (SKULL_COMBAT_ResolveRanged) against a creature target.
 *
 * DM2 vs DM1:
 *   DM1's PROJEXPL.C:631-654 routes projectile→creature via
 *   F0817_PROJECTILE_BuildHitCreatureAction_Compat and hands off to
 *   the combat engine.  DM2 runs an additional pre-pass for the four
 *   DM2-only AI missile-redirect flags (NONMATERIAL / ABSORBS_MISSILE
 *   / REFLECTOR / TURNS_MISSILE) — this module owns that pre-pass.
 *
 *   NB: REFLECTOR in DM2 reflects spells (not kinetic projectiles).
 *   Kinetic projectiles hitting a REFLECTOR creature still damage the
 *   creature; only spell projectiles (MAGICAL category) are reflected.
 *   For the gate we route both categories to REFLECTED outcome for
 *   REFLECTOR creatures — this matches SKULL.ASM:10800-10890 where
 *   the REFLECTOR flag short-circuits before the damage pass for any
 *   incoming projectile (kinetic too).  This is the source-locked
 *   behavior; spell-only reflection would diverge from SKULL.ASM.
 */

#include "dm2_v1_projectile_creature_collision_pc34_compat.h"
#include "dm2_v1_projectile_pc34_compat.h"
#include "dm2_v1_creature.h"

#include <string.h>

/* ── Observability counters ────────────────────────────────────────
 * Monotonic, reset by dm2_v1_projectile_creature_collision_reset_counters(). */
static int s_collision_count = 0;
static int s_kill_event_count = 0;
static int s_absorb_count = 0;
static int s_reflect_count = 0;
static int s_nonmaterial_count = 0;

/* ── Internal helpers ────────────────────────────────────────────── */

/* Look up the AI definition for a creature instance.  Used to read
 * the w0AIFlags (REFLECTOR / NONMATERIAL / etc.) and the ArmorClass. */
static const DM2_AIDefinition *resolve_creature_spec(int instance_id) {
    const DM2_V1_CreatureInstance *c = dm2_v1_creature_get_instance(instance_id);
    if (!c) return NULL;
    return dm2_v1_creature_ai_spec(c->ai_index);
}

/* Classify the creature-side missile-redirect outcome from its AI
 * flags.  Returns the appropriate DM2_ProjectileCreatureOutcome.
 * Priority order matches SKULL.ASM:10620-10710 missile dispatch:
 *   NONMATERIAL > ABSORBS_MISSILE > REFLECTOR > TURNS_MISSILE > HIT */
static DM2_ProjectileCreatureOutcome classify_missile_redirect(uint16_t ai_flags) {
    if (ai_flags & DM2_AIFLAG_NONMATERIAL) {
        return DM2_PROJ_CREATURE_OUTCOME_NONMATERIAL;
    }
    if (ai_flags & DM2_AIFLAG_ABSORBS_MISSILE) {
        return DM2_PROJ_CREATURE_OUTCOME_ABSORBED;
    }
    if (ai_flags & DM2_AIFLAG_REFLECTOR) {
        return DM2_PROJ_CREATURE_OUTCOME_REFLECTED;
    }
    if (ai_flags & DM2_AI_W30_TURNS_MISSILE) {
        return DM2_PROJ_CREATURE_OUTCOME_REDIRECTED;
    }
    return DM2_PROJ_CREATURE_OUTCOME_HIT;
}

/* Deterministic damage formula.  Source: SKULL.ASM:10620-10710
 * (SKULL_COMBAT_ResolveRanged) and skproject/SKULLWIN/c_combat.cpp:401-420.
 *
 *   damage = max(1, impact_attack - armor_class / 2)
 *
 * The integer-division armor_class/2 means an ArmorClass of 5 absorbs
 * 2 points (integer division floors).  Floor-at-1 ensures every HIT
 * outcome reduces at least 1 HP — matches the SKULL.ASM rule that
 * a connected projectile always does at least 1 damage.
 *
 * Negative inputs are clamped: impact_attack < 1 → 1; armor_class < 0
 * is treated as 0 (no reduction). */
static int compute_deterministic_damage(int impact_attack, int armor_class) {
    if (impact_attack < 1) impact_attack = 1;
    if (armor_class < 0)  armor_class  = 0;
    int dmg = impact_attack - armor_class / 2;
    if (dmg < 1) dmg = 1;
    return dmg;
}

/* ── Public API ──────────────────────────────────────────────────── */

DM2_V1_ProjectileCreatureCollisionResult
dm2_v1_projectile_creature_collision_resolve(int projectile_slot,
                                              int impact_attack)
{
    DM2_V1_ProjectileCreatureCollisionResult r;
    memset(&r, 0, sizeof(r));
    r.outcome                  = DM2_PROJ_CREATURE_OUTCOME_INVALID;
    r.creature_instance_id     = -1;
    r.creature_ai_index        = -1;
    r.projectile_slot          = -1;
    r.projectile_despawned     = 0;
    r.kill_event_emitted       = 0;
    r.damage_dealt             = 0;
    r.hp_after                 = -1;
    r.impact_attack_input      = impact_attack;
    r.impact_attack_effective  = 0;

    /* Validate projectile slot. */
    if (projectile_slot < 0) return r;
    DM2_V1_ProjectileSlotSnapshot snap;
    if (!dm2_v1_projectile_get_slot(projectile_slot, &snap)) return r;

    /* Find the creature instance at the projectile's world position. */
    int creature_id = dm2_v1_creature_at(snap.mapX, snap.mapY, snap.mapIndex);
    if (creature_id < 0) {
        /* No creature at this square: nothing to collide with.
         * This is the silent no-op path (projectile continues moving
         * and will be retried on the next F0811 advance tick). */
        r.outcome = DM2_PROJ_CREATURE_OUTCOME_INVALID;
        r.accepted = 0;
        r.projectile_slot = snap.slotIndex;
        return r;
    }

    const DM2_V1_CreatureInstance *c = dm2_v1_creature_get_instance(creature_id);
    if (!c || !c->alive) {
        r.outcome = DM2_PROJ_CREATURE_OUTCOME_INVALID;
        r.accepted = 0;
        return r;
    }

    const DM2_AIDefinition *spec = resolve_creature_spec(creature_id);
    uint16_t ai_flags = spec ? spec->w0AIFlags : 0;
    int armor = spec ? (int)spec->ArmorClass : 0;

    r.accepted = 1;
    r.creature_instance_id = creature_id;
    r.creature_ai_index = c->ai_index;
    r.armor_class = armor;
    r.hp_before = c->hp_current;
    r.outcome = classify_missile_redirect(ai_flags);

    /* Branch on outcome. */
    switch (r.outcome) {
        case DM2_PROJ_CREATURE_OUTCOME_NONMATERIAL:
            /* Projectile passes through.  No damage, no despawn.
             * Counts as an accepted collision event but no creature
             * state change. */
            s_nonmaterial_count++;
            s_collision_count++;
            r.damage_dealt = 0;
            r.hp_after = r.hp_before;
            r.impact_attack_effective = 0;
            return r;

        case DM2_PROJ_CREATURE_OUTCOME_ABSORBED:
            /* Projectile despawned, no damage.  Matches the SKULL.ASM
             * "absorbs missile" path (e.g., Pit Ghost eating arrows). */
            s_absorb_count++;
            s_collision_count++;
            dm2_v1_projectile_despawn(snap.slotIndex);
            r.projectile_slot = snap.slotIndex;
            r.projectile_despawned = 1;
            r.damage_dealt = 0;
            r.hp_after = r.hp_before;
            r.impact_attack_effective = 0;
            return r;

        case DM2_PROJ_CREATURE_OUTCOME_REFLECTED:
            /* Spell-reflector (e.g., Magick Reflector, AI 37).  Projectile
             * is consumed without damage. */
            s_reflect_count++;
            s_collision_count++;
            dm2_v1_projectile_despawn(snap.slotIndex);
            r.projectile_slot = snap.slotIndex;
            r.projectile_despawned = 1;
            r.damage_dealt = 0;
            r.hp_after = r.hp_before;
            r.impact_attack_effective = 0;
            return r;

        case DM2_PROJ_CREATURE_OUTCOME_REDIRECTED:
            /* TURNS_MISSILE: creature re-targets the projectile.
             * DM2 placeholder: same damage path as HIT, projectile is
             * still consumed (the redirect would happen on the next
             * F0811 tick using the new target). */
            /* fallthrough */

        case DM2_PROJ_CREATURE_OUTCOME_HIT:
        default: {
            /* Apply deterministic damage, despawn projectile, queue kill
             * event if HP dropped to 0. */
            int dmg = compute_deterministic_damage(impact_attack, armor);
            int new_hp = dm2_v1_creature_deal_damage(creature_id, dmg);
            if (new_hp < 0) {
                /* dm2_v1_creature_deal_damage returns -1 for invalid
                 * instance_id, but we already validated above.  Treat
                 * as a miss for safety. */
                r.accepted = 0;
                r.outcome = DM2_PROJ_CREATURE_OUTCOME_INVALID;
                r.damage_dealt = 0;
                return r;
            }
            r.damage_dealt = dmg;
            r.hp_after = new_hp;
            r.impact_attack_effective = dmg;
            dm2_v1_projectile_despawn(snap.slotIndex);
            r.projectile_slot = snap.slotIndex;
            r.projectile_despawned = 1;
            s_collision_count++;
            if (new_hp <= 0) {
                /* Creature died this tick.  dm2_v1_creature_death_check
                 * handles drop + spatial death sound. */
                dm2_v1_creature_death_check(creature_id);
                r.kill_event_emitted = 1;
                s_kill_event_count++;
            }
            return r;
        }
    }
}

/* ── Observability accessors ─────────────────────────────────────── */
int dm2_v1_projectile_creature_collision_count(void)  { return s_collision_count; }
int dm2_v1_projectile_creature_kill_event_count(void) { return s_kill_event_count; }
int dm2_v1_projectile_creature_absorb_count(void)     { return s_absorb_count; }
int dm2_v1_projectile_creature_reflect_count(void)    { return s_reflect_count; }
int dm2_v1_projectile_creature_nonmaterial_count(void){ return s_nonmaterial_count; }

void dm2_v1_projectile_creature_collision_reset_counters(void) {
    s_collision_count = 0;
    s_kill_event_count = 0;
    s_absorb_count = 0;
    s_reflect_count = 0;
    s_nonmaterial_count = 0;
}

/* ── Source evidence ────────────────────────────────────────────── */
const char *dm2_v1_projectile_creature_collision_source_evidence(void) {
    return
        "DM2 V1 Projectile-vs-Creature Collision — Phase 5 source-lock\n"
        "ReDMCSB SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)\n"
        "Source: SKULL.ASM:10620-10710  (SKULL_COMBAT_ResolveRanged)\n"
        "Source: SKULL.ASM:10800-10890  (SKULL_COMBAT_DoorAttack + REFLECTOR path)\n"
        "Source: ReDMCSB PROJEXPL.C:76-92       (F0212: projectile live, first move +1 tick)\n"
        "Source: ReDMCSB PROJEXPL.C:689-690     (F0219: C48 ignore-impacts-first-movement)\n"
        "Source: ReDMCSB PROJEXPL.C:1554-1600   (F0232_GROUP_IsDoorDestroyedByAttack)\n"
        "Source: ReDMCSB GROUP.C:1695-1770      (F0207: creature attack projectile payload)\n"
        "Source: ReDMCSB GROUP.C:2376-2387      (F0209: visible row/column triggers F0207)\n"
        "Source: skproject/SKULLWIN/c_creature.cpp (DM2_PROCEED_CCM, b_1a state)\n"
        "Source: skproject/SKULLWIN/c_combat.cpp:401-420 (kill threshold: damage >= hp)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:10479-10561 (AI_W30_TURNS_MISSILE check)\n"
        "Source: skproject/SKWIN/DME.h:1545-1560 (w0AIFlags: REFLECTOR/NONMATERIAL/ABSORBS_MISSILE)\n"
        "Source: memory_projectile_pc34_compat.h           (F0813 despawn + slot contract)\n"
        "Source: memory_combat_pc34_compat.h               (COMBAT_ACTION_APPLY_DAMAGE_GROUP)\n"
        "DM2 missile-redirect priority: NONMATERIAL > ABSORBS > REFLECTOR > TURNS_MISSILE > HIT\n"
        "Damage formula: max(1, impact_attack - armor_class / 2)  (deterministic, no RNG)\n"
        "DM2 vs DM1: DM1 routes projectile→creature via F0817 + DM1 combat engine;\n"
        "  DM2 runs this 5-branch missile-redirect pre-pass before the same F0817 contract\n"
        "  would be invoked downstream.  This module owns the DM2 pre-pass.\n"
        "F0813 despawn contract: projectile is consumed on HIT / ABSORBED / REFLECTED\n"
        "  paths.  NONMATERIAL leaves the projectile live for the next F0811 advance.\n"
        "Death event contract: dm2_v1_creature_death_check() runs synchronously when\n"
        "  post-damage HP <= 0 (matches DM2's same-tick death sound + drop emit).\n";
}
