#ifndef FIRESTAFF_DM2_V1_PROJECTILE_STEP_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_PROJECTILE_STEP_PC34_COMPAT_H
/*
 * dm2_v1_projectile_step_pc34_compat.h
 *
 * Phase 5 narrow source-faithful slice — DM2 V1 per-tick missile-step
 * helper that closes the runtime/drain-cache boundary.  Companion to
 * dm2_v1_projectile_pc34_compat.h (which owns the launch/dispatch
 * path and the module-owned projectile list).
 *
 * The runtime tick path that lives here is the missing bridge between:
 *   - The dispatch module (s_projectile_list[] is populated by
 *     dm2_v1_projectile_dispatch_* and dm2_v1_projectile_dispatch_synthetic)
 *   - The M11 drain cache (g_dm2_projectile_drain[] populated by
 *     dm2_v1_projectile_drain_to_m11 each render frame)
 *
 * In SKULL.ASM / skproject, between launch and draw, every V1 tick walks
 * the missile-step path (DM2_STEP_MISSILE in c_tim_proc.cpp:442-563,
 * STEP_MISSILE in SkWinCore.cpp:60920-61116).  That path:
 *   1. Honors the first-move C48 grace event.
 *   2. Checks party / creature impacts before the energy floor.
 *   3. Reads the per-slot stepEnergy and current kineticEnergy.
 *   4. If kineticEnergy <= stepEnergy → despawn the projectile
 *      (CUT_RECORD_FROM + DELETE_MISSILE_RECORD in skproject, F0813 in
 *      our M10 engine).
 *   5. Else → kineticEnergy -= stepEnergy (survivor continues).
 *   6. After the pass, only survivors should be visible to M11.
 *
 * The "drain" cache without this step is a snapshot of all live
 * projectiles; it never expires.  With this step, the cache reflects
 * the post-step survivors of the most recent tick — which is exactly
 * what M11 should draw this frame.
 *
 * This module is intentionally narrow:
 *   - It wires only the creature impact gate already owned by
 *     dm2_v1_projectile_creature_collision_pc34_compat.c.
 *   - It does NOT build a full CellContentDigest_Compat (wall / door /
 *     tile state remains a separate, larger slice).
 *   - It does NOT handle map-change / CHANGE_CURRENT_MAP_TO (the
 *     next larger slice would do that).
 *   - It does NOT handle teleporter rotation (F0228_GetDirectionsWhere
 *     DestinationIsVisibleFromSource is a separate slice).
 *   - It only implements the creature-impact + energy-decay / despawn
 *     boundary, which is the deterministic, source-lockable first half
 *     of STEP_MISSILE.
 *
 * That boundary is enough to prove the runtime/drain-cache handoff:
 * after dm2_v1_projectile_step_tick() runs, the next
 * dm2_v1_projectile_drain_to_m11() call sees exactly the survivors
 * (slots with slotIndex >= 0 and decremented kineticEnergy).
 *
 * ── Source-lock anchors ────────────────────────────────────────────
 *   skproject/SKULLWIN/c_tim_proc.cpp:442-563   (DM2_STEP_MISSILE)
 *       m_7CE0:   RG2W = RG51timp->getB(); RG2UW >>= 0xc;
 *                 RG1L = unsignedlong(RG2W);
 *                 RG4L = unsignedlong(byte_at(RG71p, lcon(0x4)));  (kineticEnergy)
 *                 if (RG4L <= RG1L) → despawn via CUT_RECORD_FROM + DELETE_MISSILE_RECORD
 *       m_7D2A:   RG4Blo -= RG2Blo; mov8(location(RG71p + 4), RG4Blo);
 *                 (else: kineticEnergy -= stepEnergy)
 *
 *   skproject/SKWIN/SkWinCore.cpp:60920-61116   (STEP_MISSILE wrapper)
 *       ^075F:1349: CUT_RECORD_FROM + DELETE_MISSILE_RECORD on depletion
 *       ^075F:1375: bp08->EnergyRemaining(bp08->EnergyRemaining() - stepEnergy)
 *
 *   ReDMCSB PROJEXPL.C:76-92                    (F0212: live +1 tick)
 *   ReDMCSB PROJEXPL.C:515-539                  (F0217: creature impact)
 *   ReDMCSB PROJEXPL.C:692-698                  (F0219: impact before energy)
 *   ReDMCSB GROUP.C:1695-1770                   (F0207 creature attack payload)
 *   memory_projectile_pc34_compat.h             (F0813 despawn + F0810 slot)
 *
 * ── Determinism ────────────────────────────────────────────────────
 *   No RNG is consumed by the step path.  Given the same input list,
 *   the post-step survivor set is bit-identical.  This is critical for
 *   the CTest regression gate.
 *
 * ── DM2 vs DM1 difference captured here ────────────────────────────
 *   DM1's PROJEXPL.C:689-690 uses C48_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS
 *   for the first movement of a champion/creature-launched projectile,
 *   so the first step is impact-free.  DM2 follows the same convention
 *   (PROJEXPL.C:76-92 first move +1 tick).  This module respects
 *   firstMoveGraceFlag: a slot with firstMoveGraceFlag == 1 does NOT
 *   decay energy on its first step (matches DM1's C48 grace).
 */

#include <stdint.h>
#include "dm2_v1_projectile_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Step result ──────────────────────────────────────────────────
 * Mirrors what the drain cache should see after one tick's pass:
 *   - slots_alive_before: count of occupied slots before the step
 *   - slots_alive_after:  count of occupied slots after the step
 *   - slots_despawned:    count of slots that were F0813-despawned
 *                         by creature impact or the energy floor
 *   - slots_survived:     count of slots whose kineticEnergy was
 *                         decremented (kineticEnergy -= stepEnergy)
 *   - slots_creature_collisions: accepted creature impact resolutions
 *                         before energy decay
 *   - energy_total_before:sum of kineticEnergy across all live slots
 *                          before the step
 *   - energy_total_after: sum of kineticEnergy across survivors after
 *                          the step
 *
 * Identity invariant: slots_alive_before == slots_alive_after
 *                                            + slots_despawned
 * Energy invariant:    energy_total_before >= energy_total_after
 *                     (equal when no slot had enough energy to step)
 * First-move grace:    a slot with firstMoveGraceFlag == 1 is counted
 *                     in slots_survived but does NOT decrement its
 *                     kineticEnergy (matches C48 grace rule). */
typedef struct {
    int slots_alive_before;
    int slots_alive_after;
    int slots_despawned;
    int slots_survived;
    int slots_graced;             /* firstMoveGraceFlag honored */
    int slots_creature_collisions; /* creature impact checks accepted */
    int energy_total_before;
    int energy_total_after;
} DM2_V1_ProjectileStepResult;

/* ── Public API: step the DM2 projectile list one tick ───────────
 *
 * dm2_v1_projectile_step_tick — walk the DM2 module-owned projectile
 * list once and apply the energy-decay / despawn boundary.
 *
 * Returns DM2_V1_ProjectileStepResult with the bookkeeping numbers.
 * After this call returns, the next dm2_v1_projectile_drain_to_m11()
 * will see only the survivors (slots with kineticEnergy > 0, or
 * graced slots).
 *
 * The function is idempotent for a single call: re-running it with
 * the same input list reproduces the same result (no RNG, no global
 * state mutation outside the list).
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:442-563 (DM2_STEP_MISSILE)
 *         skproject/SKWIN/SkWinCore.cpp:60920-61116 (STEP_MISSILE) */
DM2_V1_ProjectileStepResult dm2_v1_projectile_step_tick(void);

/* ── Public API: step + drain in one call ────────────────────────
 *
 * dm2_v1_projectile_step_and_drain — atomic step-then-drain.  Walks
 * the projectile list once, applies the energy-decay boundary, then
 * copies the survivors into the caller's drained array.
 *
 * This is the canonical M11-boundary call: tick → step → drain.
 *
 * Returns the number of survivors drained into out_list (0..max_count).
 * *out_step (optional) receives the step bookkeeping numbers.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:442-563 step path
 *         skproject/SKULLWIN/c_render.cpp drain path */
int dm2_v1_projectile_step_and_drain(DM2_V1_DrainedProjectile *out_list,
                                       int max_count,
                                       DM2_V1_ProjectileStepResult *out_step);

/* ── Public API: reset helper ────────────────────────────────────
 * Resets the per-step observability counters (step_total,
 * step_despawned_total).  Does NOT touch the projectile list itself.
 * Source: matches dm2_v1_projectile_reset_counters() pattern. */
void dm2_v1_projectile_step_reset_counters(void);

/* ── Public API: monotonic observability counters ──────────────── */
int dm2_v1_projectile_step_total(void);          /* total step ticks invoked */
int dm2_v1_projectile_step_despawned_total(void); /* total slots despawned across all ticks */
int dm2_v1_projectile_step_survived_total(void);  /* total slots that survived across all ticks */
int dm2_v1_projectile_step_graced_total(void);    /* total slots whose firstMoveGraceFlag was honored */
int dm2_v1_projectile_step_creature_collision_total(void); /* accepted creature impacts across ticks */

/* ── Source evidence citation ──────────────────────────────────── */
const char *dm2_v1_projectile_step_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_PROJECTILE_STEP_PC34_COMPAT_H */
