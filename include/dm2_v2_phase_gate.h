/*
 * dm2_v2_phase_gate.h — DM2 V2 Phase Gate Definitions
 *
 * Phase 1: Launch/Profile Separation
 *
 * DM2 V2 Phase 1 separates the V2 launch path from the V2 profile path:
 *
 *   DM2_V2_PHASE_DOMAIN_LAUNCH — V2 boot launch path
 *     - Must be able to start DM2 without requiring any other game's catalog
 *     - Uses DM2-specific hash-verified asset discovery
 *     - Initializes DM2 runtime with minimal V2 presentation scaffolding
 *     - Does NOT load champion portraits or dungeon graphics yet
 *     - Safe to run headless (no rendering required for asset verification)
 *
 *   DM2_V2_PHASE_DOMAIN_PROFILE — V2 profile / asset loading path
 *     - Loads DM2-specific assets: champion portraits, dungeon graphics
 *     - Reads from hash-verified DM2 data files
 *       (GRAPHICS.DAT: 25247ede4dabb6a71e5dabdfbcd5907d PC English,
 *        DUNGEON.DAT:  6caccd7875009e82fe2e28e7f6d6adc0 PC English + variants)
 *     - Binds DM2 V2 presentation pipeline to loaded assets
 *     - Initializes smooth movement, outdoor enhanced, HUD, lighting
 *
 * Phase gates follow the CSB V2 phase gate pattern:
 *   - LAUNCH is a prerequisite for PROFILE
 *   - Both default to V1 behavior (disabled) to preserve source lock
 *   - Explicit enable flag required for V2 presentation work
 *
 * Source references:
 *   SKULL.ASM T520  — party/movement tick
 *   SKULL.ASM T560  — dungeon viewport rendering
 *   SKULL.ASM T600  — outdoor viewport rendering
 *   SKULL.ASM T580  — load dungeon (entrance → dungeon load)
 *   SKULLWIN CSB.cpp CSBData::Initialize (boot/asset pattern)
 *   ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution
 *   ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)
 */

#ifndef FIRESTAFF_DM2_V2_PHASE_GATE_H
#define FIRESTAFF_DM2_V2_PHASE_GATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ── Phase domains ─────────────────────────────────────────────────────── */

typedef enum {
    DM2_V2_PHASE_DOMAIN_LAUNCH = 0,  /* V2 boot launch: minimal asset scan */
    DM2_V2_PHASE_DOMAIN_PROFILE = 1   /* V2 profile: full DM2 asset load    */
} DM2_V2_PhaseDomain;

/* ── Gate configuration ─────────────────────────────────────────────────── */

typedef struct {
    int v2LaunchEnabled;      /* DM2_V2_PHASE_DOMAIN_LAUNCH toggle     */
    int v2ProfileEnabled;     /* DM2_V2_PHASE_DOMAIN_PROFILE toggle    */
} DM2_V2_PhaseGateConfig;

/* ── Gate decision ─────────────────────────────────────────────────────── */

typedef struct {
    int v1SourceLocked;           /* domain stays on V1 source path       */
    int v2Allowed;                /* V2 presentation/behavior allowed     */
    const char *sourceAnchor;     /* SKULL.ASM source reference          */
    const char *rule;             /* human-readable gate rule             */
} DM2_V2_PhaseGateDecision;

/* ── API ───────────────────────────────────────────────────────────────── */

void dm2_v2_phase_gate_defaults(DM2_V2_PhaseGateConfig *config);
int  dm2_v2_phase_gate_is_launch_domain(DM2_V2_PhaseDomain domain);
int  dm2_v2_phase_gate_is_profile_domain(DM2_V2_PhaseDomain domain);
DM2_V2_PhaseGateDecision dm2_v2_phase_gate_decide(
    const DM2_V2_PhaseGateConfig *config,
    DM2_V2_PhaseDomain domain);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_PHASE_GATE_H */
