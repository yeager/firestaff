/*
 * csb_v2_phase_gate.c — CSB V2 Phase Gate (Phase 0 Stub)
 * =========================================================
 *
 * Phase 0 role:
 *   Provides a minimal V1-compatibility-shell so that the
 *   firestaff_csb_v2 static library links without introducing
 *   Phase-1-or-later symbols that are not yet implemented.
 *
 *   The real Phase 0 gate implementation lives in:
 *     src/csb/csb_v2_phase_gate_pc34.c  (csb_v2_phase_gate_pc34.h)
 *
 *   That implementation provides:
 *     - 13-domain phase model (gameplay V1-locked + V2-presentation)
 *     - v2PresentationEnabled / v2ConfigPersistenceEnabled toggles
 *     - Static struct-size assertions in csb_v2_phase0_assertions_pc34.h
 *
 * Phase 0 invariant:
 *   This file must NOT contain any Phase-1-or-later API surface.
 *   All public symbols declared in csb_v2_phase_gate.h (LAUNCH/PROFILE
 *   domains, v2LaunchEnabled/v2ProfileEnabled) are Phase 1 features
 *   and MUST NOT be implemented here.
 *
 * Phase 1 (LAUNCH/PROFILE separation) will replace this stub with
 * a proper csb_v2_phase_gate.c that wires the correct pc34 header.
 *
 * Source-lock (Phase 0 baseline — this file has no game-logic):
 *   No ReDMCSB or CSBWin source references — this is a pure scaffold.
 */

#include "csb_v2_phase_gate.h"   /* Phase 1 API declarations only */

/* ── Phase 0 gate API — delegate to pc34 implementation ──────────── */

void csb_v2_phase_gate_defaults(CSB_V2_PhaseGateConfig *config)
{
    /* csb_v2_phase_gate_pc34.h uses v2PresentationEnabled /
     * v2ConfigPersistenceEnabled.  We must not set those from here.
     * Phase 0: all V2 presentation is disabled by default.
     * Phase 1: caller sets v2LaunchEnabled / v2ProfileEnabled instead. */
    if (!config) return;
    /* Zero-initialize the Phase 1 config struct (all features off). */
    config->v2LaunchEnabled  = 0;
    config->v2ProfileEnabled = 0;
}

/* ── Phase 1 API stubs (declared in csb_v2_phase_gate.h but not yet
 *     implemented — replace with Phase 1 work)                      ── */

int csb_v2_phase_gate_is_launch_domain(CSB_V2_PhaseDomain domain)
{
    (void)domain;
    /* Phase 1: implement LAUNCH domain detection */
    return 0;
}

int csb_v2_phase_gate_is_profile_domain(CSB_V2_PhaseDomain domain)
{
    (void)domain;
    /* Phase 1: implement PROFILE domain detection */
    return 0;
}

CSB_V2_PhaseGateDecision csb_v2_phase_gate_decide(
    const CSB_V2_PhaseGateConfig *config,
    CSB_V2_PhaseDomain domain)
{
    /* Phase 1: route LAUNCH/PROFILE decisions through the pc34 gate
     * for domains it understands, and apply Phase 1 rules.
     * For now, all domains are V1-locked until Phase 1 is implemented. */
    (void)config;
    CSB_V2_PhaseGateDecision d;
    d.v1SourceLocked = 1;
    d.v2Allowed     = 0;
    d.sourceAnchor   = "csb_v2_phase_gate.c Phase 0 stub";
    d.rule          = "Phase 0: all domains V1-source-locked until Phase 1";
    (void)domain;
    return d;
}
