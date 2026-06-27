/*
 * dm2_v1_projectile_step_pc34_compat.c
 *
 * Phase 5 narrow source-faithful slice — DM2 V1 per-tick missile-step
 * helper that closes the runtime/drain-cache boundary.
 *
 * The implementation applies only the energy-decay + despawn half of
 * STEP_MISSILE (skproject/SKULLWIN/c_tim_proc.cpp:442-563, the m_7CE0
 * branch and the post-despawn energy decrement at m_7D2A).  The map
 * change, teleporter rotation, and CellContentDigest-based collision
 * resolution are explicitly out of scope here — those belong to the
 * next larger slice.
 *
 * ── Module-private list accessor ─────────────────────────────────
 * The dispatch module (dm2_v1_projectile_pc34_compat.c) owns the
 * s_projectile_list.  We talk to it via two narrow APIs:
 *
 *   dm2_v1_projectile_get_slot(slot, &snap)   — read-only snapshot,
 *       extended in this slice with kineticEnergy / stepEnergy /
 *       firstMoveGraceFlag fields so the step helper can decide.
 *
 *   dm2_v1_projectile_consume_step_energy(slot, &post) — applies the
 *       energy-decay / despawn boundary to one slot.  Returns 1 if
 *       the slot survived (with the post-step energy in *out_post),
 *       or 0 if the slot was F0813-despawned.
 *
 * Why this decomposition?  Matches the SKULLWIN.c_tim_proc.cpp split
 * (the STEP_* routines own the per-tick mutation, while c_creature
 * owns CCM dispatch).  Avoids exposing s_projectile_list directly
 * outside the dispatch module.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_tim_proc.cpp:442-563   (DM2_STEP_MISSILE)
 *       m_7CE0:  kineticEnergy <= stepEnergy → CUT_RECORD_FROM + DELETE_MISSILE_RECORD
 *       m_7D2A:  RG4Blo -= RG2Blo; mov8(location(RG71p + 4), RG4Blo);  (else branch)
 *
 *   skproject/SKWIN/SkWinCore.cpp:60920-61116   (STEP_MISSILE wrapper)
 *       ^075F:1349:  despawn on energy floor
 *       ^075F:1375:  bp08->EnergyRemaining(...) -= stepEnergy
 *
 *   ReDMCSB PROJEXPL.C:76-92                    (F0212 live +1 tick)
 *   ReDMCSB PROJEXPL.C:689-690                  (F0219 C48 grace)
 *   ReDMCSB GROUP.C:1695-1770                   (F0207 creature attack)
 *   memory_projectile_pc34_compat.h             (F0813 despawn + F0810 slot)
 */

#include "dm2_v1_projectile_step_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <string.h>

/* ── Forward decl for the dispatch helper (defined in dispatch module) ── */
extern int dm2_v1_projectile_consume_step_energy(int slot_index,
                                                  int *out_post_energy,
                                                  int *out_was_graced);

/* ── Observability counters ──────────────────────────────────────
 * Monotonic; reset by dm2_v1_projectile_step_reset_counters(). */
static int s_step_total = 0;
static int s_step_despawned_total = 0;
static int s_step_survived_total = 0;
static int s_step_graced_total = 0;

/* ── Step pass ─────────────────────────────────────────────────── */

DM2_V1_ProjectileStepResult dm2_v1_projectile_step_tick(void) {
    DM2_V1_ProjectileStepResult r;
    memset(&r, 0, sizeof(r));

    const int cap = PROJECTILE_LIST_CAPACITY;
    int i;

    for (i = 0; i < cap; i++) {
        DM2_V1_ProjectileSlotSnapshot before;
        if (!dm2_v1_projectile_get_slot(i, &before)) continue;
        r.slots_alive_before++;
        r.energy_total_before += before.kineticEnergy;

        /* Apply the energy-decay boundary for this slot. */
        int post_energy = 0;
        int was_graced = 0;
        int survived = dm2_v1_projectile_consume_step_energy(
                            i, &post_energy, &was_graced);
        if (!survived) {
            r.slots_despawned++;
            continue;
        }

        /* Slot survived — record post-step energy. */
        r.slots_survived++;
        r.slots_alive_after++;
        r.energy_total_after += post_energy;
        if (was_graced) {
            r.slots_graced++;
        }
    }

    s_step_total++;
    s_step_despawned_total += r.slots_despawned;
    s_step_survived_total += r.slots_survived;
    s_step_graced_total += r.slots_graced;
    return r;
}

/* ── Step + drain (atomic M11-boundary call) ──────────────────── */

int dm2_v1_projectile_step_and_drain(DM2_V1_DrainedProjectile *out_list,
                                       int max_count,
                                       DM2_V1_ProjectileStepResult *out_step) {
    DM2_V1_ProjectileStepResult local;
    DM2_V1_ProjectileStepResult *rs = out_step ? out_step : &local;
    *rs = dm2_v1_projectile_step_tick();
    return dm2_v1_projectile_drain_to_m11(out_list, max_count);
}

/* ── Reset / observability ─────────────────────────────────────── */

void dm2_v1_projectile_step_reset_counters(void) {
    s_step_total = 0;
    s_step_despawned_total = 0;
    s_step_survived_total = 0;
    s_step_graced_total = 0;
}

int dm2_v1_projectile_step_total(void)           { return s_step_total; }
int dm2_v1_projectile_step_despawned_total(void) { return s_step_despawned_total; }
int dm2_v1_projectile_step_survived_total(void)  { return s_step_survived_total; }
int dm2_v1_projectile_step_graced_total(void)    { return s_step_graced_total; }

/* ── Source evidence ───────────────────────────────────────────── */

const char *dm2_v1_projectile_step_source_evidence(void) {
    return
        "DM2 V1 Projectile Drain Runtime / M11 Cache Boundary — Phase 5 source-lock\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp:442-563   (DM2_STEP_MISSILE)\n"
        "        m_7CE0:  kineticEnergy <= stepEnergy → CUT_RECORD_FROM + DELETE_MISSILE_RECORD\n"
        "        m_7D2A:  RG4Blo -= RG2Blo; mov8(location(RG71p + 4), RG4Blo); (else branch)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:60920-61116   (STEP_MISSILE wrapper)\n"
        "        ^075F:1349:  despawn on energy floor\n"
        "        ^075F:1375:  bp08->EnergyRemaining(bp08->EnergyRemaining() - stepEnergy)\n"
        "Source: ReDMCSB PROJEXPL.C:76-92                    (F0212 live +1 tick)\n"
        "Source: ReDMCSB PROJEXPL.C:689-690                  (F0219 C48 first-move grace)\n"
        "Source: ReDMCSB GROUP.C:1695-1770                   (F0207 creature attack)\n"
        "Source: memory_projectile_pc34_compat.h             (F0813 despawn + F0810 slot)\n"
        "Module role: closes the runtime/drain-cache boundary so the per-tick M11\n"
        "            projectile drain reflects post-step survivors only.\n"
        "V1 invariant: no RNG consumed; same input list → same step result.\n"
        "Out of scope (next slice): full F0811 cell-content-digest advance,\n"
        "        CHANGE_CURRENT_MAP_TO routing, F0228 teleporter rotation.\n";
}
