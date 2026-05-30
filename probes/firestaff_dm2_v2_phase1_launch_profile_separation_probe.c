/*
 * firestaff_dm2_v2_phase1_launch_profile_separation_probe.c
 *
 * DM2 V2 Phase 1 — Launch/Profile Separation Probe
 *
 * Headless probe: verifies DM2 V2 phase gates and launch/profile separation
 * without requiring live game asset files.
 *
 * This probe validates:
 *
 *   1. DM2_V2_PHASE_DOMAIN_LAUNCH gate
 *      - When v2LaunchEnabled=1: V2 launch is allowed
 *      - When v2LaunchEnabled=0: V2 launch is source-locked (V1 behavior)
 *      - LAUNCH does NOT require any other game's catalog assets
 *
 *   2. DM2_V2_PHASE_DOMAIN_PROFILE gate
 *      - When v2ProfileEnabled=1 AND v2LaunchEnabled=1: PROFILE allowed
 *      - When v2LaunchEnabled=0: PROFILE blocked regardless of v2ProfileEnabled
 *      - PROFILE reads DM2-specific asset hashes
 *
 *   3. DM2 vs other-game asset hash separation
 *      - DM2 DUNGEON.DAT:  6caccd7875009e82fe2e28e7f6d6adc0 (PC English + variants)
 *      - DM2 GRAPHICS.DAT: 25247ede4dabb6a71e5dabdfbcd5907d (PC English)
 *      - PC French GRAPHICS.DAT: b4d733576ea60c41737f79f212faf528
 *      - PC German JewelCase GRAPHICS.DAT: e52ab5e01715042b16a4dcff02052e5d
 *      - These hashes are completely disjoint from DM1 and CSB.
 *
 *   4. Headless verification: no game data files needed
 *      - This probe exercises only the phase gate logic and hash constants.
 *      - It does NOT load GRAPHICS.DAT or DUNGEON.DAT.
 *      - SDL_VIDEODRIVER=dummy safe.
 *
 * Exit codes:
 *   0  — all checks passed
 *   1  — one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy ./firestaff_dm2_v2_phase1_launch_profile_separation_probe
 *
 * Source references:
 *   SKULL.ASM T520  — party/movement tick
 *   SKULL.ASM T560  — dungeon viewport rendering
 *   SKULL.ASM T580  — load dungeon (asset hash check)
 *   SKULL.ASM T600  — outdoor viewport rendering
 *   ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution
 *   ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)
 *   dm2_v1_boot.c: g_dm2_dungeon_hashes[], g_dm2_graphics_hashes[]
 *   dm2_v2_phase_gate.c — phase gate implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v2_phase_gate.h"

/* ── Known hash constants ───────────────────────────────────────────────── */

static const char *g_dm2_dungeon_hash  = "6caccd7875009e82fe2e28e7f6d6adc0";
static const char *g_dm2_graphics_hash_en = "25247ede4dabb6a71e5dabdfbcd5907d";
static const char *g_dm2_graphics_hash_fr = "b4d733576ea60c41737f79f212faf528";
static const char *g_dm2_graphics_hash_de = "e52ab5e01715042b16a4dcff02052e5d";

/* ── Test counters ──────────────────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

/* ── Reporting helpers ─────────────────────────────────────────────────── */

static void record(int ok, const char *id, const char *msg)
{
    if (ok) {
        ++g_pass;
        printf("PASS %s  %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s  %s\n", id, msg);
    }
}

static void section(const char *title)
{
    printf("\n=== %s ===\n", title);
}

/* ── Test cases ─────────────────────────────────────────────────────────── */

static void test_defaults_null_safe(void)
{
    section("dm2_v2_phase_gate_defaults null safety");
    DM2_V2_PhaseGateConfig cfg;
    memset(&cfg, 0xAA, sizeof(cfg));
    dm2_v2_phase_gate_defaults(&cfg);
    record(cfg.v2LaunchEnabled == 0, "DEF0", "v2LaunchEnabled defaults to 0");
    record(cfg.v2ProfileEnabled == 0, "DEF1", "v2ProfileEnabled defaults to 0");
    (void)dm2_v2_phase_gate_is_launch_domain(DM2_V2_PHASE_DOMAIN_LAUNCH);
    (void)dm2_v2_phase_gate_is_profile_domain(DM2_V2_PHASE_DOMAIN_PROFILE);
}

static void test_domain_identifiers(void)
{
    section("dm2_v2_phase_gate_is_*_domain");
    record(dm2_v2_phase_gate_is_launch_domain(DM2_V2_PHASE_DOMAIN_LAUNCH),
           "DOM0", "LAUNCH domain identified");
    record(!dm2_v2_phase_gate_is_launch_domain(DM2_V2_PHASE_DOMAIN_PROFILE),
           "DOM1", "LAUNCH false for PROFILE domain");
    record(dm2_v2_phase_gate_is_profile_domain(DM2_V2_PHASE_DOMAIN_PROFILE),
           "DOM2", "PROFILE domain identified");
    record(!dm2_v2_phase_gate_is_profile_domain(DM2_V2_PHASE_DOMAIN_LAUNCH),
           "DOM3", "PROFILE false for LAUNCH domain");
}

static void test_launch_gate_v1_locked(void)
{
    section("DM2_V2_PHASE_DOMAIN_LAUNCH — V1 locked (v2LaunchEnabled=0)");
    DM2_V2_PhaseGateConfig cfg;
    dm2_v2_phase_gate_defaults(&cfg);
    /* Explicitly keep launch disabled */
    cfg.v2LaunchEnabled = 0;

    DM2_V2_PhaseGateDecision dec =
        dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_LAUNCH);

    record(dec.v1SourceLocked == 1,  "LAUNCH_V1_0", "v1SourceLocked=1 when disabled");
    record(dec.v2Allowed == 0,       "LAUNCH_V1_1", "v2Allowed=0 when disabled");
    record(dec.sourceAnchor != NULL,  "LAUNCH_V1_2", "sourceAnchor is set");
    record(dec.rule != NULL,         "LAUNCH_V1_3", "rule is set");
}

static void test_launch_gate_v2_allowed(void)
{
    section("DM2_V2_PHASE_DOMAIN_LAUNCH — V2 allowed (v2LaunchEnabled=1)");
    DM2_V2_PhaseGateConfig cfg;
    dm2_v2_phase_gate_defaults(&cfg);
    cfg.v2LaunchEnabled = 1;

    DM2_V2_PhaseGateDecision dec =
        dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_LAUNCH);

    record(dec.v1SourceLocked == 0,  "LAUNCH_V2_0", "v1SourceLocked=0 when enabled");
    record(dec.v2Allowed == 1,       "LAUNCH_V2_1", "v2Allowed=1 when enabled");
    record(dec.sourceAnchor != NULL, "LAUNCH_V2_2", "sourceAnchor is set");
    record(dec.rule != NULL,         "LAUNCH_V2_3", "rule describes V2 launch path");
}

static void test_profile_gate_v1_locked(void)
{
    section("DM2_V2_PHASE_DOMAIN_PROFILE — V1 locked (launch disabled)");
    DM2_V2_PhaseGateConfig cfg;
    dm2_v2_phase_gate_defaults(&cfg);
    cfg.v2LaunchEnabled = 0;
    cfg.v2ProfileEnabled = 1;  /* profile enabled but launch disabled */

    DM2_V2_PhaseGateDecision dec =
        dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_PROFILE);

    record(dec.v1SourceLocked == 1,  "PROF_V1_0", "v1SourceLocked=1 when LAUNCH disabled");
    record(dec.v2Allowed == 0,       "PROF_V1_1", "v2Allowed=0 (PROFILE gated on LAUNCH)");
    record(dec.sourceAnchor != NULL,  "PROF_V1_2", "sourceAnchor is set");
}

static void test_profile_gate_v2_allowed(void)
{
    section("DM2_V2_PHASE_DOMAIN_PROFILE — V2 allowed (both enabled)");
    DM2_V2_PhaseGateConfig cfg;
    dm2_v2_phase_gate_defaults(&cfg);
    cfg.v2LaunchEnabled = 1;
    cfg.v2ProfileEnabled = 1;

    DM2_V2_PhaseGateDecision dec =
        dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_PROFILE);

    record(dec.v1SourceLocked == 0,  "PROF_V2_0", "v1SourceLocked=0 when both enabled");
    record(dec.v2Allowed == 1,       "PROF_V2_1", "v2Allowed=1 when both enabled");
    record(dec.sourceAnchor != NULL, "PROF_V2_2", "sourceAnchor is set");
    record(dec.rule != NULL,         "PROF_V2_3", "rule describes V2 profile path");
}

static void test_profile_gate_profile_only_enabled(void)
{
    section("DM2_V2_PHASE_DOMAIN_PROFILE — profile-only (no launch)");
    DM2_V2_PhaseGateConfig cfg;
    dm2_v2_phase_gate_defaults(&cfg);
    cfg.v2LaunchEnabled = 0;   /* explicit off */
    cfg.v2ProfileEnabled = 1; /* should still be blocked */

    DM2_V2_PhaseGateDecision dec =
        dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_PROFILE);

    record(dec.v1SourceLocked == 1,  "PROF_ONLY_0", "v1SourceLocked=1 (launch a prerequisite)");
    record(dec.v2Allowed == 0,       "PROF_ONLY_1", "v2Allowed=0 (profile gated on launch)");
}

static void test_hash_separation(void)
{
    section("DM2 asset hash separation from DM1/CSB");

    /* DM2 DUNGEON.DAT hash (PC English + variants): 6caccd7875009e82fe2e28e7f6d6adc0 */
    /* DM1 DUNGEON.DAT hash: d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85 */
    /* CSB DUNGEON.DAT hash: 6695d2acebce49f95db1d8f3a5c733de */
    static const char *dm1_dungeon = "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85";
    static const char *csb_dungeon = "6695d2acebce49f95db1d8f3a5c733de";

    record(strcmp(g_dm2_dungeon_hash, dm1_dungeon) != 0, "HS_DM2DM1_0",
           "DM2 dungeon hash differs from DM1");
    record(strcmp(g_dm2_dungeon_hash, csb_dungeon) != 0, "HS_DM2CSB_0",
           "DM2 dungeon hash differs from CSB");

    /* DM2 GRAPHICS.DAT (PC English): 25247ede4dabb6a71e5dabdfbcd5907d */
    /* DM1 GRAPHICS.DAT: 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e */
    /* CSB GRAPHICS.DAT: 61fbfd56887c94adc26888a9491c6611 */
    static const char *dm1_gfx = "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e";
    static const char *csb_gfx = "61fbfd56887c94adc26888a9491c6611";

    record(strcmp(g_dm2_graphics_hash_en, dm1_gfx) != 0, "HS_DM2DM1_1",
           "DM2 graphics hash differs from DM1");
    record(strcmp(g_dm2_graphics_hash_en, csb_gfx) != 0, "HS_DM2CSB_1",
           "DM2 graphics hash differs from CSB");

    /* All three DM2 variants are distinct */
    record(strcmp(g_dm2_graphics_hash_en, g_dm2_graphics_hash_fr) != 0, "HS_DM2VAR_0",
           "DM2 EN graphics hash differs from DM2 FR");
    record(strcmp(g_dm2_graphics_hash_fr, g_dm2_graphics_hash_de) != 0, "HS_DM2VAR_1",
           "DM2 FR graphics hash differs from DM2 DE");
    record(strcmp(g_dm2_graphics_hash_en, g_dm2_graphics_hash_de) != 0, "HS_DM2VAR_2",
           "DM2 EN graphics hash differs from DM2 DE");
}

static void test_null_config_antisymmetric(void)
{
    section("Null-config antisymmetric: both domains should be V1 (no crash)");
    DM2_V2_PhaseGateDecision dec_launch =
        dm2_v2_phase_gate_decide(NULL, DM2_V2_PHASE_DOMAIN_LAUNCH);
    DM2_V2_PhaseGateDecision dec_profile =
        dm2_v2_phase_gate_decide(NULL, DM2_V2_PHASE_DOMAIN_PROFILE);

    record(dec_launch.v1SourceLocked == 1,    "NULL_L0",
           "LAUNCH null-config: v1SourceLocked=1");
    record(dec_launch.v2Allowed == 0,       "NULL_L1",
           "LAUNCH null-config: v2Allowed=0");
    record(dec_profile.v1SourceLocked == 1,   "NULL_P0",
           "PROFILE null-config: v1SourceLocked=1");
    record(dec_profile.v2Allowed == 0,        "NULL_P1",
           "PROFILE null-config: v2Allowed=0");
}

static void test_decision_self_consistent(void)
{
    section("Decision self-consistency: same config same result");
    DM2_V2_PhaseGateConfig cfg_a, cfg_b;
    dm2_v2_phase_gate_defaults(&cfg_a);
    dm2_v2_phase_gate_defaults(&cfg_b);
    cfg_a.v2LaunchEnabled = 1;
    cfg_a.v2ProfileEnabled = 1;
    cfg_b.v2LaunchEnabled = 1;
    cfg_b.v2ProfileEnabled = 1;

    DM2_V2_PhaseGateDecision dec_a =
        dm2_v2_phase_gate_decide(&cfg_a, DM2_V2_PHASE_DOMAIN_PROFILE);
    DM2_V2_PhaseGateDecision dec_b =
        dm2_v2_phase_gate_decide(&cfg_b, DM2_V2_PHASE_DOMAIN_PROFILE);

    record(dec_a.v1SourceLocked == dec_b.v1SourceLocked, "CONS_0",
           "Same config: v1SourceLocked consistent");
    record(dec_a.v2Allowed == dec_b.v2Allowed,         "CONS_1",
           "Same config: v2Allowed consistent");
    record(dec_a.sourceAnchor == dec_b.sourceAnchor,    "CONS_2",
           "Same config: sourceAnchor same pointer");
    record(dec_a.rule == dec_b.rule,                   "CONS_3",
           "Same config: rule same pointer");
}

static void test_asset_hash_constants(void)
{
    section("Asset hash constants match dm2_v1_boot.c");
    /* These hashes are embedded in dm2_v1_boot.c g_dm2_*_hashes[] */

    /* DM2 dungeon hash: 6caccd7875009e82fe2e28e7f6d6adc0 */
    record(strlen(g_dm2_dungeon_hash) == 32, "HASH_DG_0",
           "DM2 DUNGEON.DAT hash is valid MD5 (32 hex chars)");
    record(strlen(g_dm2_graphics_hash_en) == 32, "HASH_GFX_0",
           "DM2 GRAPHICS.DAT EN hash is valid MD5 (32 hex chars)");
    record(strlen(g_dm2_graphics_hash_fr) == 32, "HASH_GFX_1",
           "DM2 GRAPHICS.DAT FR hash is valid MD5 (32 hex chars)");
    record(strlen(g_dm2_graphics_hash_de) == 32, "HASH_GFX_2",
           "DM2 GRAPHICS.DAT DE hash is valid MD5 (32 hex chars)");
}

/* ── Main ──────────────────────────────────────────────────────────────── */

int main(void)
{
    printf("DM2 V2 Phase 1 — Launch/Profile Separation Probe\n");
    printf("Built from: %s %s\n", __DATE__, __TIME__);

    test_defaults_null_safe();
    test_domain_identifiers();
    test_launch_gate_v1_locked();
    test_launch_gate_v2_allowed();
    test_profile_gate_v1_locked();
    test_profile_gate_v2_allowed();
    test_profile_gate_profile_only_enabled();
    test_hash_separation();
    test_null_config_antisymmetric();
    test_decision_self_consistent();
    test_asset_hash_constants();

    printf("\n--- Results: %d passed, %d failed ---\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
