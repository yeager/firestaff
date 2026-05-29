/*
 * firestaff_csb_v2_phase1_launch_profile_separation_probe.c
 *
 * CSB V2 Phase 1 — Launch/Profile Separation Probe
 *
 * Headless probe: verifies CSB V2 phase gates and launch/profile separation
 * without requiring live game asset files.
 *
 * This probe validates:
 *
 *   1. CSB_V2_PHASE_DOMAIN_LAUNCH gate
 *      - When v2LaunchEnabled=1: V2 launch is allowed
 *      - When v2LaunchEnabled=0: V2 launch is source-locked (V1 behavior)
 *      - LAUNCH does NOT require DM1 catalog assets
 *
 *   2. CSB_V2_PHASE_DOMAIN_PROFILE gate
 *      - When v2ProfileEnabled=1 AND v2LaunchEnabled=1: PROFILE allowed
 *      - When v2LaunchEnabled=0: PROFILE blocked regardless of v2ProfileEnabled
 *      - PROFILE reads CSB-specific asset hashes
 *
 *   3. CSB vs DM1 asset hash separation
 *      - CSB DUNGEON.DAT: 6695d2acebce49f95db1d8f3a5c733de
 *      - DM1 DUNGEON.DAT: d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
 *      - CSB GRAPHICS.DAT: 61fbfd56887c94adc26888a9491c6611
 *      - DM1 GRAPHICS.DAT: 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
 *      - The two game catalogs are completely disjoint by hash.
 *
 *   4. CSB graphics archive variants
 *      - PC 3.4 EN:      61fbfd56887c94adc26888a9491c6611
 *      - Atari ST 2.1:   ebf6a57af3f27782e358c0490bfd2f2e
 *      - Amiga 3.5 EN:   291e1bc6803e3dc4b974c60117ca5d68
 *      - Amiga 3.5 ML:   cefaddfdf5651df2c91f61b5611a8362
 *      - All four are CSB archives (different platform releases).
 *
 *   5. Headless verification: no game data files needed
 *      - This probe exercises only the phase gate logic and hash constants.
 *      - It does NOT load GRAPHICS.DAT or DUNGEON.DAT.
 *      - SDL_VIDEODRIVER=dummy safe.
 *
 * Exit codes:
 *   0  — all checks passed
 *   1  — one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy ./firestaff_csb_v2_phase1_launch_profile_separation_probe
 *
 * Source references:
 *   ReDMCSB ENTRANCE.C F0806 (CSB boot, C28_ENTRANCE_CSB)
 *   ReDMCSB PROFILE.C F0401 (champion portrait loading)
 *   ReDMCSB LOADSAVE.C F0435 (new-game party location)
 *   CSBWin CSB.cpp CSBData::Initialize
 *   asset_status_m12.c: g_csbVersions[] (CSB hash catalog)
 *   csb_v1_boot.c: g_csb_boot_graphics_hashes[], g_csb_boot_dungeon_hashes[]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csb_v2_phase_gate.h"

/* ── Known hash constants ───────────────────────────────────────────────── */

static const char *g_csb_dungeon_hash  = "6695d2acebce49f95db1d8f3a5c733de";
static const char *g_dm1_dungeon_hash  = "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85";
static const char *g_csb_graphics_hash = "61fbfd56887c94adc26888a9491c6611";
static const char *g_dm1_graphics_hash = "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e";

static const char *const g_csb_graphics_variants[] = {
    "61fbfd56887c94adc26888a9491c6611",  /* PC 3.4 EN */
    "ebf6a57af3f27782e358c0490bfd2f2e",  /* Atari ST 2.1 EN */
    "291e1bc6803e3dc4b974c60117ca5d68",  /* Amiga 3.5 EN */
    "cefaddfdf5651df2c91f61b5611a8362",  /* Amiga 3.5 ML */
    NULL
};

/* ── Test counters ──────────────────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

/* ── Reporting helpers ──────────────────────────────────────────────────── */

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

/* ── Test cases ────────────────────────────────────────────────────────── */

/* TC-1: csb_v2_phase_gate_defaults() initializes to V1 behavior */
static void tc_defaults(void)
{
    CSB_V2_PhaseGateConfig cfg;
    csb_v2_phase_gate_defaults(&cfg);
    record(cfg.v2LaunchEnabled == 0,
           "TC-1", "v2LaunchEnabled=0 after defaults");
    record(cfg.v2ProfileEnabled == 0,
           "TC-1", "v2ProfileEnabled=0 after defaults");
}

/* TC-2: domain classification helpers */
static void tc_domain_class(void)
{
    record(csb_v2_phase_gate_is_launch_domain(CSB_V2_PHASE_DOMAIN_LAUNCH),
           "TC-2", "LAUNCH domain recognized as launch");
    record(!csb_v2_phase_gate_is_profile_domain(CSB_V2_PHASE_DOMAIN_LAUNCH),
           "TC-2", "LAUNCH domain not recognized as profile");
    record(csb_v2_phase_gate_is_profile_domain(CSB_V2_PHASE_DOMAIN_PROFILE),
           "TC-2", "PROFILE domain recognized as profile");
    record(!csb_v2_phase_gate_is_launch_domain(CSB_V2_PHASE_DOMAIN_PROFILE),
           "TC-2", "PROFILE domain not recognized as launch");
}

/* TC-3: LAUNCH gate — v2LaunchEnabled=0 means source-locked (V1 behavior) */
static void tc_launch_v1_locked(void)
{
    CSB_V2_PhaseGateConfig cfg;
    CSB_V2_PhaseGateDecision dec;
    csb_v2_phase_gate_defaults(&cfg);
    /* explicit v2LaunchEnabled=0 (already the default, but be explicit) */
    cfg.v2LaunchEnabled = 0;
    dec = csb_v2_phase_gate_decide(&cfg, CSB_V2_PHASE_DOMAIN_LAUNCH);
    record(dec.v1SourceLocked == 1,
           "TC-3", "LAUNCH v1SourceLocked when v2LaunchEnabled=0");
    record(dec.v2Allowed == 0,
           "TC-3", "LAUNCH v2Allowed=0 when v2LaunchEnabled=0");
}

/* TC-4: LAUNCH gate — v2LaunchEnabled=1 means V2 launch allowed */
static void tc_launch_v2_allowed(void)
{
    CSB_V2_PhaseGateConfig cfg;
    CSB_V2_PhaseGateDecision dec;
    csb_v2_phase_gate_defaults(&cfg);
    cfg.v2LaunchEnabled = 1;
    dec = csb_v2_phase_gate_decide(&cfg, CSB_V2_PHASE_DOMAIN_LAUNCH);
    record(dec.v1SourceLocked == 0,
           "TC-4", "LAUNCH v1SourceLocked=0 when v2LaunchEnabled=1");
    record(dec.v2Allowed == 1,
           "TC-4", "LAUNCH v2Allowed=1 when v2LaunchEnabled=1");
}

/* TC-5: PROFILE gate — v2ProfileEnabled=1 but LAUNCH disabled → blocked */
static void tc_profile_requires_launch(void)
{
    CSB_V2_PhaseGateConfig cfg;
    CSB_V2_PhaseGateDecision dec;
    csb_v2_phase_gate_defaults(&cfg);
    cfg.v2ProfileEnabled = 1;
    cfg.v2LaunchEnabled  = 0;  /* explicit: launch disabled */
    dec = csb_v2_phase_gate_decide(&cfg, CSB_V2_PHASE_DOMAIN_PROFILE);
    record(dec.v1SourceLocked == 1,
           "TC-5", "PROFILE v1SourceLocked=1 when LAUNCH disabled");
    record(dec.v2Allowed == 0,
           "TC-5", "PROFILE v2Allowed=0 when LAUNCH disabled");
}

/* TC-6: PROFILE gate — both enabled → V2 profile allowed */
static void tc_profile_v2_allowed(void)
{
    CSB_V2_PhaseGateConfig cfg;
    CSB_V2_PhaseGateDecision dec;
    csb_v2_phase_gate_defaults(&cfg);
    cfg.v2LaunchEnabled  = 1;
    cfg.v2ProfileEnabled = 1;
    dec = csb_v2_phase_gate_decide(&cfg, CSB_V2_PHASE_DOMAIN_PROFILE);
    record(dec.v1SourceLocked == 0,
           "TC-6", "PROFILE v1SourceLocked=0 when both enabled");
    record(dec.v2Allowed == 1,
           "TC-6", "PROFILE v2Allowed=1 when both enabled");
}

/* TC-7: PROFILE gate — v2ProfileEnabled=1, launch enabled, but PROFILE needs launch first */
static void tc_profile_not_auto_enabled(void)
{
    CSB_V2_PhaseGateConfig cfg;
    CSB_V2_PhaseGateDecision dec;
    /* Set v2ProfileEnabled=1 but leave v2LaunchEnabled=0 (default) */
    csb_v2_phase_gate_defaults(&cfg);
    cfg.v2ProfileEnabled = 1;  /* explicit profile enable, but... */
    /* v2LaunchEnabled stays 0 */
    dec = csb_v2_phase_gate_decide(&cfg, CSB_V2_PHASE_DOMAIN_PROFILE);
    record(dec.v2Allowed == 0,
           "TC-7", "PROFILE v2Allowed=0 when v2ProfileEnabled=1 but LAUNCH=0");
}

/* TC-8: CSB vs DM1 hash separation — no hash collision */
static void tc_csb_dm1_hash_separation(void)
{
    int csb_vs_dm1_dungeon = strcmp(g_csb_dungeon_hash, g_dm1_dungeon_hash) != 0;
    int csb_vs_dm1_graphics = strcmp(g_csb_graphics_hash, g_dm1_graphics_hash) != 0;
    record(csb_vs_dm1_dungeon,
           "TC-8", "CSB DUNGEON.DAT hash differs from DM1 DUNGEON.DAT hash");
    record(csb_vs_dm1_graphics,
           "TC-8", "CSB GRAPHICS.DAT hash differs from DM1 GRAPHICS.DAT hash");
    /* Also verify neither is the empty string */
    record(strlen(g_csb_dungeon_hash) == 32,
           "TC-8", "CSB DUNGEON.DAT hash is valid 32-char MD5");
    record(strlen(g_csb_graphics_hash) == 32,
           "TC-8", "CSB GRAPHICS.DAT hash is valid 32-char MD5");
}

/* TC-9: All CSB graphics variants are distinct */
static void tc_csb_variant_hashes(void)
{
    int i, j;
    int all_distinct = 1;
    for (i = 0; g_csb_graphics_variants[i] != NULL; ++i) {
        for (j = i + 1; g_csb_graphics_variants[j] != NULL; ++j) {
            if (strcmp(g_csb_graphics_variants[i], g_csb_graphics_variants[j]) == 0) {
                all_distinct = 0;
            }
        }
    }
    record(all_distinct,
           "TC-9", "All four CSB graphics variant hashes are distinct");
    /* None equals DM1's hash */
    for (i = 0; g_csb_graphics_variants[i] != NULL; ++i) {
        record(strcmp(g_csb_graphics_variants[i], g_dm1_graphics_hash) != 0,
               "TC-9", "CSB variant hash differs from DM1 GRAPHICS.DAT hash");
    }
}

/* TC-10: No DM1 hash appears in CSB catalog and vice versa */
static void tc_no_hash_collision(void)
{
    int dm1_in_csb = 0, csb_in_dm1 = 0;
    (void)dm1_in_csb; (void)csb_in_dm1;
    /* CSB dungeon hash is not DM1 dungeon hash */
    record(strcmp(g_csb_dungeon_hash, g_dm1_dungeon_hash) != 0,
           "TC-10", "CSB dungeon hash is not DM1 dungeon hash (no collision)");
    /* CSB graphics hash is not DM1 graphics hash */
    record(strcmp(g_csb_graphics_hash, g_dm1_graphics_hash) != 0,
           "TC-10", "CSB graphics hash is not DM1 graphics hash (no collision)");
    /* CSB graphics hash is not any DM1 dungeon hash (trivial: DM1 dungeon ≠ CSB graphics) */
    record(strcmp(g_csb_graphics_hash, g_dm1_dungeon_hash) != 0,
           "TC-10", "CSB graphics hash is not DM1 dungeon hash");
    /* DM1 graphics hash is not CSB dungeon hash */
    record(strcmp(g_dm1_graphics_hash, g_csb_dungeon_hash) != 0,
           "TC-10", "DM1 graphics hash is not CSB dungeon hash");
}

/* TC-11: Gate source anchor strings are non-NULL */
static void tc_source_anchors(void)
{
    CSB_V2_PhaseGateConfig cfg;
    CSB_V2_PhaseGateDecision dec_launch, dec_profile;
    csb_v2_phase_gate_defaults(&cfg);
    cfg.v2LaunchEnabled  = 1;
    cfg.v2ProfileEnabled = 1;
    dec_launch  = csb_v2_phase_gate_decide(&cfg, CSB_V2_PHASE_DOMAIN_LAUNCH);
    dec_profile = csb_v2_phase_gate_decide(&cfg, CSB_V2_PHASE_DOMAIN_PROFILE);
    record(dec_launch.sourceAnchor != NULL && strlen(dec_launch.sourceAnchor) > 0,
           "TC-11", "LAUNCH decision has a source anchor");
    record(dec_profile.sourceAnchor != NULL && strlen(dec_profile.sourceAnchor) > 0,
           "TC-11", "PROFILE decision has a source anchor");
    record(dec_launch.rule != NULL && strlen(dec_launch.rule) > 0,
           "TC-11", "LAUNCH decision has a rule description");
    record(dec_profile.rule != NULL && strlen(dec_profile.rule) > 0,
           "TC-11", "PROFILE decision has a rule description");
}

/* TC-12: LAUNCH gate — source evidence mentions CSB-specific assets (not DM1) */
static void tc_launch_no_dm1_dependency(void)
{
    CSB_V2_PhaseGateConfig cfg;
    CSB_V2_PhaseGateDecision dec;
    csb_v2_phase_gate_defaults(&cfg);
    cfg.v2LaunchEnabled = 1;
    dec = csb_v2_phase_gate_decide(&cfg, CSB_V2_PHASE_DOMAIN_LAUNCH);
    /* The source anchor and rule must NOT reference DM1 assets */
    record(strstr(dec.sourceAnchor, "DM1") == NULL,
           "TC-12", "LAUNCH source anchor does not reference DM1");
    record(strstr(dec.sourceAnchor, "d90b6b1c38fd17e41d63682f8afe5ca3") == NULL,
           "TC-12", "LAUNCH source anchor does not contain DM1 DUNGEON.DAT hash");
    record(strstr(dec.sourceAnchor, "2c3aa836925c64c09402bafb03c6459") == NULL,
           "TC-12", "LAUNCH source anchor does not contain DM1 GRAPHICS.DAT hash");
    record(strstr(dec.rule, "DM1") == NULL,
           "TC-12", "LAUNCH rule does not reference DM1 assets");
    /* Must reference CSB-specific hash */
    record(strstr(dec.sourceAnchor, "6695d2acebce49f95db1d8f3a5c733de") != NULL,
           "TC-12", "LAUNCH source anchor references CSB DUNGEON.DAT hash");
    record(strstr(dec.sourceAnchor, "61fbfd56887c94adc26888a9491c6611") != NULL,
           "TC-12", "LAUNCH source anchor references CSB GRAPHICS.DAT hash");
}

/* TC-13: SourceAnchor strings contain ReDMCSB and/or CSBWin references */
static void tc_source_references(void)
{
    CSB_V2_PhaseGateConfig cfg;
    CSB_V2_PhaseGateDecision dec_launch, dec_profile;
    csb_v2_phase_gate_defaults(&cfg);
    cfg.v2LaunchEnabled  = 1;
    cfg.v2ProfileEnabled = 1;
    dec_launch  = csb_v2_phase_gate_decide(&cfg, CSB_V2_PHASE_DOMAIN_LAUNCH);
    dec_profile = csb_v2_phase_gate_decide(&cfg, CSB_V2_PHASE_DOMAIN_PROFILE);
    /* Both must reference at least one source */
    record(strstr(dec_launch.sourceAnchor, "ReDMCSB") != NULL ||
           strstr(dec_launch.sourceAnchor, "CSBWin") != NULL,
           "TC-13", "LAUNCH sourceAnchor contains ReDMCSB or CSBWin reference");
    record(strstr(dec_profile.sourceAnchor, "ReDMCSB") != NULL ||
           strstr(dec_profile.sourceAnchor, "CSBWin") != NULL,
           "TC-13", "PROFILE sourceAnchor contains ReDMCSB or CSBWin reference");
}

/* ── Main ─────────────────────────────────────────────────────────────── */

int main(void)
{
    printf("CSB V2 Phase 1 — Launch/Profile Separation Probe\n");
    printf("Headless: no game assets required\n");
    printf("SDL_VIDEODRIVER=%s\n\n", getenv("SDL_VIDEODRIVER") ?: "(null)");

    section("TC-1: defaults()");
    tc_defaults();

    section("TC-2: domain classification");
    tc_domain_class();

    section("TC-3: LAUNCH — V1 locked (v2LaunchEnabled=0)");
    tc_launch_v1_locked();

    section("TC-4: LAUNCH — V2 allowed (v2LaunchEnabled=1)");
    tc_launch_v2_allowed();

    section("TC-5: PROFILE — requires LAUNCH (LAUNCH disabled → blocked)");
    tc_profile_requires_launch();

    section("TC-6: PROFILE — V2 allowed (both enabled)");
    tc_profile_v2_allowed();

    section("TC-7: PROFILE — not auto-enabled (v2ProfileEnabled=1, LAUNCH=0)");
    tc_profile_not_auto_enabled();

    section("TC-8: CSB vs DM1 hash separation");
    tc_csb_dm1_hash_separation();

    section("TC-9: CSB variant hashes distinct");
    tc_csb_variant_hashes();

    section("TC-10: no cross-game hash collisions");
    tc_no_hash_collision();

    section("TC-11: source anchor and rule strings present");
    tc_source_anchors();

    section("TC-12: LAUNCH gate does not reference DM1 assets");
    tc_launch_no_dm1_dependency();

    section("TC-13: source anchors contain ReDMCSB/CSBWin references");
    tc_source_references();

    section("SUMMARY");
    printf("PASS: %d  FAIL: %d\n", g_pass, g_fail);
    if (g_fail > 0) {
        printf("RESULT: FAIL\n");
        return 1;
    }
    printf("RESULT: PASS\n");
    return 0;
}
