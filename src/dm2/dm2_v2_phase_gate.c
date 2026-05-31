/*
 * dm2_v2_phase_gate.c — DM2 V2 Phase Gate Implementation
 *
 * Phase 1: Launch/Profile Separation
 *
 * Source-lock anchors:
 *   SKULL.ASM T520  — party/movement tick (V1 game state per 55ms tick)
 *   SKULL.ASM T560  — dungeon viewport rendering (V1 pipeline)
 *   SKULL.ASM T580  — load dungeon: entrance check, then F0805 dungeon load
 *   SKULL.ASM T600  — outdoor viewport rendering (alternate entry point)
 *   SKULLWIN CSB.cpp CSBData::Initialize — boot/asset pattern for reference
 *   ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution
 *   ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)
 *
 * Phase 1 rules:
 *   LAUNCH domain: V2 launch must verify DM2 assets independently.
 *     DM2 DUNGEON.DAT MD5:  6caccd7875009e82fe2e28e7f6d6adc0 (PC English + variants)
 *     DM2 GRAPHICS.DAT MD5: 25247ede4dabb6a71e5dabdfbcd5907d (PC English)
 *     DM2 also accepts the PC French (b4d733576ea60c41737f79f212faf528) and
 *     PC German JewelCase (e52ab5e01715042b16a4dcff02052e5d) graphics hashes.
 *     V2 launch must NOT require any other game's GRAPHICS.DAT or DUNGEON.DAT.
 *
 *   PROFILE domain: V2 profile loads DM2-specific champion portraits and
 *     dungeon graphics from the correct hash-verified DM2 archive.
 *     Profile is LAUNCH-after: launch establishes asset paths, profile
 *     binds the presentation pipeline to those paths.
 *     The following V2 modules are bound in profile domain:
 *       dm2_v2_asset_pipeline  (Phase 2 — enhanced asset pipeline)
 *       dm2_v2_hud_overlay     (Phase 3 — enhanced HUD)
 *       dm2_v2_lighting        (Phase 4 — enhanced lighting)
 *       dm2_v2_viewport_renderer / dm2_v2_smooth_movement (Phase 5 — smooth movement)
 *       dm2_v2_outdoor_enhanced (Phase 4 outdoor — enhanced sky/terrain)
 *
 *   Both domains default to V1 behavior (source-locked) until explicitly
 *     enabled via dm2_v2_phase_gate_defaults → user configuration.
 */

#include "dm2_v2_phase_gate.h"

/* ── Internal helpers ──────────────────────────────────────────────────── */

static DM2_V2_PhaseGateDecision decision(int sourceLocked,
                                         int v2Allowed,
                                         const char *anchor,
                                         const char *rule) {
    DM2_V2_PhaseGateDecision out;
    out.v1SourceLocked = sourceLocked ? 1 : 0;
    out.v2Allowed = v2Allowed ? 1 : 0;
    out.sourceAnchor = anchor;
    out.rule = rule;
    return out;
}

/* ── Public API ────────────────────────────────────────────────────────── */

void dm2_v2_phase_gate_defaults(DM2_V2_PhaseGateConfig *config)
{
    /* Default: V2 launch/profile disabled — all domains route to V1 source.
     * Caller must explicitly set v2LaunchEnabled / v2ProfileEnabled. */
    if (!config) return;
    config->v2LaunchEnabled = 0;
    config->v2ProfileEnabled = 0;
}

int dm2_v2_phase_gate_is_launch_domain(DM2_V2_PhaseDomain domain)
{
    return domain == DM2_V2_PHASE_DOMAIN_LAUNCH;
}

int dm2_v2_phase_gate_is_profile_domain(DM2_V2_PhaseDomain domain)
{
    return domain == DM2_V2_PHASE_DOMAIN_PROFILE;
}

int dm2_v2_phase_gate_is_hud_domain(DM2_V2_PhaseDomain domain)
{
    return domain == DM2_V2_PHASE_DOMAIN_HUD;
}

DM2_V2_PhaseGateDecision dm2_v2_phase_gate_decide(
    const DM2_V2_PhaseGateConfig *config,
    DM2_V2_PhaseDomain domain)
{
    int launchEnabled  = config && config->v2LaunchEnabled;
    int profileEnabled = config && config->v2ProfileEnabled;

    switch (domain) {

        case DM2_V2_PHASE_DOMAIN_LAUNCH:
            /* V2 launch: enables DM2-specific asset discovery.
             * LAUNCH is a prerequisite for PROFILE but is independently useful
             * for headless build verification (no rendering required).
             *
             * Source-lock: SKULL.ASM T580 loads dungeon after entrance check,
             * verifying DM2 DUNGEON.DAT hash before any game state is set.
             * V2 launch must verify DM2 hashes ONLY and must not fall back
             * to or chain-load other game's hashes if DM2 assets are absent.
             *
             * DM2 DUNGEON.DAT hash:  6caccd7875009e82fe2e28e7f6d6adc0 (PC EN + variants)
             * DM2 GRAPHICS.DAT hashes:
             *   25247ede4dabb6a71e5dabdfbcd5907d  PC English
             *   b4d733576ea60c41737f79f212faf528 PC French
             *   e52ab5e01715042b16a4dcff02052e5d PC German/English JewelCase
             */
            return decision(!launchEnabled, launchEnabled,
                            "SKULL.ASM T580 (load dungeon / asset hash check); "
                            "SKULL.ASM T520 (party tick V1 invariant); "
                            "ReDMCSB DUNGEON.C:1371-1421",
                            launchEnabled
                                ? "DM2 V2 LAUNCH: DM2 hash-verified asset path active"
                                : "DM2 V2 LAUNCH: V1 source-locked (v2LaunchEnabled=0)");

        case DM2_V2_PHASE_DOMAIN_PROFILE:
            /* V2 profile: full DM2 asset load and V2 presentation pipeline bind.
             * PROFILE requires LAUNCH to first establish the asset paths.
             * If launch is disabled, profile is also disabled (source-locked).
             *
             * Phase 2 (asset pipeline), Phase 3 (HUD), Phase 4 (lighting/outdoor),
             * Phase 5 (smooth movement) are all PROFILE-domain features.
             * They must not activate unless LAUNCH is also enabled.
             *
             * DM2 DUNGEON.DAT hash:  6caccd7875009e82fe2e28e7f6d6adc0 (PC EN + variants)
             * DM2 GRAPHICS.DAT hashes as above (PC EN/FR/DE).
             */
            if (!launchEnabled) {
                return decision(1, 0,
                               "SKULL.ASM T560 (viewport render V1); "
                               "SKULL.ASM T520; "
                               "PROFILE gated on LAUNCH (launch disabled)",
                               "DM2 V2 PROFILE: V1 source-locked (LAUNCH not enabled)");
            }
            return decision(!profileEnabled, profileEnabled,
                            "SKULL.ASM T560 (viewport render / profile bind); "
                            "SKULL.ASM T520 (party tick V1 invariant); "
                            "ReDMCSB DUNGEON.C:1371-1421; "
                            "SKULLWIN CSB.cpp CSBData::Initialize (boot pattern)",
                            profileEnabled
                                ? "DM2 V2 PROFILE: DM2 V2 pipeline bound to hash-verified assets"
                                : "DM2 V2 PROFILE: V1 source-locked (v2ProfileEnabled=0)");

        case DM2_V2_PHASE_DOMAIN_HUD:
            /* V2 HUD: Phase 3 enhanced UI overlay rendering (compass, depth,
             * gold counter, champion mini-bars, action strip).
             * HUD is a PROFILE-domain feature: it activates only when both
             * LAUNCH and PROFILE are enabled (full V2 presentation pipeline).
             *
             * Source: SKULL.ASM T560 HUD rendering (compass/depth/gold);
             *   SKULLWIN/SKWIN/c_gui_vp.cpp (DM2 UI chrome layout);
             *   ReDMCSB PANEL.C F0354 (champion status-box drawing);
             *   ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing).
             *
             * This module is presentation-only: it draws optional overlay
             * elements into the supplied framebuffer and does NOT mutate
             * dungeon, champion, or command runtime state.
             */
            if (!launchEnabled || !profileEnabled) {
                return decision(1, 0,
                               "SKULL.ASM T560 (HUD V1 source-locked); "
                               "HUD gated on LAUNCH+PROFILE (domain not enabled)",
                               "DM2 V2 HUD: V1 source-locked (LAUNCH or PROFILE not enabled)");
            }
            return decision(0, 1,
                            "SKULL.ASM T560 (HUD rendering); "
                            "SKULLWIN/SKWIN/c_gui_vp.cpp (DM2 UI chrome layout); "
                            "ReDMCSB PANEL.C F0354; "
                            "ReDMCSB DUNGEON.C F0260",
                            profileEnabled
                                ? "DM2 V2 HUD: Phase 3 overlay active (LAUNCH+PROFILE enabled)"
                                : "DM2 V2 HUD: V1 source-locked (v2ProfileEnabled=0)");

        default:
            return decision(1, 0, "unknown phase domain", "invalid domain");
    }
}
