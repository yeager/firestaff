/*
 * csb_v2_phase_gate.c — CSB V2 Phase Gate Implementation
 *
 * Phase 1: Launch/Profile Separation
 *
 * Source-lock anchors:
 *   ReDMCSB ENTRANCE.C F0806 lines 409-441: builds entrance micro-dungeon,
 *     selects C28_ENTRANCE_CSB palette (CSB-specific, not DM1 C28_ENTRANCE)
 *   ReDMCSB ENTRANCE.C F0806 lines 857-883: entrance state machine waits
 *     then switches G0298_B_NewGame to C001_MODE_LOAD_DUNGEON
 *   ReDMCSB LOADSAVE.C F0435 lines 1940-1944: loads initial party location
 *     from DUNGEON.DAT and sets G0309_i_PartyMapIndex to map 0 for new games
 *   ReDMCSB PROFILE.C F0401: champion portrait loading (CSB portrait index
 *     differs from DM1; CSB uses extended portrait set for the expanded roster)
 *   CSBWin CSB.cpp: CSBData::Initialize / CSB::Run — startup initialization
 *
 * Phase 1 rules:
 *   LAUNCH domain: V2 launch must verify CSB assets independently from DM1.
 *     CSB DUNGEON.DAT MD5: 6695d2acebce49f95db1d8f3a5c733de (PC 3.4 EN)
 *     CSB GRAPHICS.DAT MD5: 61fbfd56887c94adc26888a9491c6611 (PC 3.4 EN)
 *     CSBGRAPH.DAT and CSB.DAT are also valid CSB archives (floppy variants).
 *     V2 launch must NOT require DM1's GRAPHICS.DAT or DUNGEON.DAT hashes.
 *
 *   PROFILE domain: V2 profile loads CSB-specific champion portraits and
 *     dungeon graphics from the correct hash-verified CSB archive.
 *     Profile is LAUNCH-after: launch establishes asset paths, profile
 *     binds the presentation pipeline to those paths.
 *
 *   Both domains default to V1 behavior (source-locked) until explicitly
 *     enabled via csb_v2_phase_gate_defaults → user configuration.
 */

#include "csb_v2_phase_gate.h"

/* ── Internal helpers ──────────────────────────────────────────────────── */

static CSB_V2_PhaseGateDecision decision(int sourceLocked,
                                          int v2Allowed,
                                          const char *anchor,
                                          const char *rule) {
    CSB_V2_PhaseGateDecision out;
    out.v1SourceLocked = sourceLocked ? 1 : 0;
    out.v2Allowed = v2Allowed ? 1 : 0;
    out.sourceAnchor = anchor;
    out.rule = rule;
    return out;
}

/* ── Public API ────────────────────────────────────────────────────────── */

void csb_v2_phase_gate_defaults(CSB_V2_PhaseGateConfig *config)
{
    /* Default: V2 launch/profile disabled — all domains route to V1 source.
     * Caller must explicitly set v2LaunchEnabled / v2ProfileEnabled. */
    if (!config) return;
    config->v2LaunchEnabled = 0;
    config->v2ProfileEnabled = 0;
}

int csb_v2_phase_gate_is_launch_domain(CSB_V2_PhaseDomain domain)
{
    return domain == CSB_V2_PHASE_DOMAIN_LAUNCH;
}

int csb_v2_phase_gate_is_profile_domain(CSB_V2_PhaseDomain domain)
{
    return domain == CSB_V2_PHASE_DOMAIN_PROFILE;
}

CSB_V2_PhaseGateDecision csb_v2_phase_gate_decide(
    const CSB_V2_PhaseGateConfig *config,
    CSB_V2_PhaseDomain domain)
{
    int launchEnabled  = config && config->v2LaunchEnabled;
    int profileEnabled = config && config->v2ProfileEnabled;

    switch (domain) {

        case CSB_V2_PHASE_DOMAIN_LAUNCH:
            /* V2 launch: enables CSB-specific asset discovery without DM1 deps.
             * LAUNCH is a prerequisite for PROFILE but is independently useful
             * for headless build verification (no rendering required).
             *
             * Source-lock: ENTRANCE.C F0806 sets up CSB entrance micro-dungeon
             * and C28_ENTRANCE_CSB palette before any asset binding.
             * V2 may extend the entrance animation but must not skip it.
             *
             * CSB DUNGEON.DAT hash: 6695d2acebce49f95db1d8f3a5c733de (PC 3.4 EN)
             * CSB GRAPHICS.DAT hash: 61fbfd56887c94adc26888a9491c6611 (PC 3.4 EN)
             * These hashes are distinct from DM1's:
             *   DM1 DUNGEON.DAT: d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
             *   DM1 GRAPHICS.DAT: 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
             *
             * The V2 launch path must verify CSB hashes ONLY and must not
             * fall back to or chain-load DM1 hashes if CSB assets are absent. */
            return decision(!launchEnabled, launchEnabled,
                            "ReDMCSB ENTRANCE.C F0806 lines 409-441 (hash 6695d2acebce49f95db1d8f3a5c733de); "
                            "ENTRANCE.C F0806 lines 857-883; "
                            "LOADSAVE.C F0435 lines 1940-1944; "
                            "CSBWin CSB.cpp CSBData::Initialize (hash 61fbfd56887c94adc26888a9491c6611)",
                            "V2 launch scans CSB-specific asset hashes independently; "
                            "V2 launch must use CSB hash catalog only (no external asset chains); "
                            "original source path is the fallback when v2LaunchEnabled=0");

        case CSB_V2_PHASE_DOMAIN_PROFILE:
            /* V2 profile: loads champion portraits and dungeon graphics from
             * CSB hash-verified archives (GRAPHICS.DAT / CSBGRAPH.DAT / CSB.DAT).
             *
             * Source-lock: PROFILE.C F0401 owns champion portrait loading.
             * CSB portrait roster extends DM1 with CSB-only champion classes.
             * Portrait index binding must use CSB palette entries, not DM1's.
             *
             * V2 profile must bind to assets discovered during LAUNCH.
             * Profile is LAUNCH-after: enabling PROFILE without LAUNCH is invalid. */
            return decision(!(profileEnabled && launchEnabled),
                            profileEnabled && launchEnabled,
                            "ReDMCSB PROFILE.C F0401; "
                            "ReDMCSB DUNGEON.C F0148-F0170 (hash 6695d2acebce49f95db1d8f3a5c733de); "
                            "CSBWin Character.cpp portrait loading",
                            "V2 profile loads CSB-specific assets only; "
                            "PROFILE requires LAUNCH to be enabled first; "
                            "original source path is the fallback when v2ProfileEnabled=0 or LAUNCH disabled");

        default:
            return decision(1, 0, "unknown domain",
                            "unknown domains are V1 source-locked by default");
    }
}
