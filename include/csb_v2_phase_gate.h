/*
 * csb_v2_phase_gate.h — CSB V2 Phase Gate Definitions
 *
 * Phase 1: Launch/Profile Separation
 *
 * CSB V2 Phase 1 separates the V2 launch path from the V2 profile path:
 *
 *   CSB_V2_PHASE_DOMAIN_LAUNCH — V2 boot launch path
 *     - Must be able to start CSB without requiring DM1 catalog assets
 *     - Uses CSB-specific hash-verified asset discovery
 *     - Initializes CSB runtime with minimal V2 presentation scaffolding
 *     - Does NOT load champion portraits or dungeon graphics yet
 *     - Safe to run headless (no rendering required for asset verification)
 *
 *   CSB_V2_PHASE_DOMAIN_PROFILE — V2 profile / asset loading path
 *     - Loads CSB-specific assets: champion portraits, dungeon graphics
 *     - Reads from hash-verified CSB data files (GRAPHICS.DAT/CSBGRAPH.DAT
 *       and DUNGEON.DAT with CSB-specific MD5 hashes)
 *     - Binds CSB V2 presentation pipeline to loaded assets
 *     - Initializes chaos magic, smooth movement, dynamic lighting
 *
 * Phase gates follow the DM1 V2 phase gate pattern:
 *   - LAUNCH is a prerequisite for PROFILE
 *   - Both default to V1 behavior (disabled) to preserve source lock
 *   - Explicit enable flag required for V2 presentation work
 *
 * Source references:
 *   ReDMCSB ENTRANCE.C F0806 (CSB boot, C28_ENTRANCE_CSB palette)
 *   ReDMCSB ENTRANCE.C F0807 (intro animation step)
 *   ReDMCSB PROFILE.C F0401 (champion portrait loading)
 *   ReDMCSB DUNGEON.C F0148-F0170 (dungeon data format)
 *   CSBWin CSB.cpp startup / CSBData::Initialize
 */

#ifndef FIRESTAFF_CSB_V2_PHASE_GATE_H
#define FIRESTAFF_CSB_V2_PHASE_GATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ── Phase domains ─────────────────────────────────────────────────────── */

typedef enum {
    CSB_V2_PHASE_DOMAIN_LAUNCH = 0,   /* V2 boot launch: minimal asset scan */
    CSB_V2_PHASE_DOMAIN_PROFILE = 1   /* V2 profile: full CSB asset load    */
} CSB_V2_PhaseDomain;

/* ── Gate configuration ─────────────────────────────────────────────────── */

typedef struct {
    int v2LaunchEnabled;     /* CSB_V2_PHASE_DOMAIN_LAUNCH toggle      */
    int v2ProfileEnabled;    /* CSB_V2_PHASE_DOMAIN_PROFILE toggle     */
} CSB_V2_PhaseGateConfig;

/* ── Gate decision ─────────────────────────────────────────────────────── */

typedef struct {
    int v1SourceLocked;           /* domain stays on V1 source path       */
    int v2Allowed;                /* V2 presentation/behavior allowed     */
    const char *sourceAnchor;     /* ReDMCSB / CSBWin source reference    */
    const char *rule;             /* human-readable gate rule             */
} CSB_V2_PhaseGateDecision;

/* ── API ───────────────────────────────────────────────────────────────── */

void csb_v2_phase_gate_defaults(CSB_V2_PhaseGateConfig *config);
int  csb_v2_phase_gate_is_launch_domain(CSB_V2_PhaseDomain domain);
int  csb_v2_phase_gate_is_profile_domain(CSB_V2_PhaseDomain domain);
CSB_V2_PhaseGateDecision csb_v2_phase_gate_decide(
    const CSB_V2_PhaseGateConfig *config,
    CSB_V2_PhaseDomain domain);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_PHASE_GATE_H */
