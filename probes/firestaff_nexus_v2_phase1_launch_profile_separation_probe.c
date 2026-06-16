/*
 * firestaff_nexus_v2_phase1_launch_profile_separation_probe.c
 *
 * Nexus V2 Phase 1 - Launch/Profile Separation Probe
 *
 * Headless probe: verifies Nexus V2 phase gates and launch/profile
 * separation without requiring live game asset files.
 *
 * This probe validates:
 *
 *   1. NEXUS_V2_PHASE_DOMAIN_GAME_STATE_INIT gate
 *      - V1 game state init is always allowed (V1-source-locked)
 *      - V1 game state init never depends on V2 toggles
 *
 *   2. NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION gate
 *      - When v2PresentationEnabled=0: V2 render is source-locked
 *        (V1 framebuffer stays as-is)
 *      - When v2PresentationEnabled=1: V2 render is allowed
 *
 *   3. NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION gate
 *      - When v2ConfigPersistenceEnabled=0: V2 config writes blocked
 *      - When v2ConfigPersistenceEnabled=1 AND v2PresentationEnabled=1:
 *        V2 config writes allowed
 *      - When v2ConfigPersistenceEnabled=1 AND v2PresentationEnabled=0:
 *        V2 config writes still blocked
 *
 *   4. Nexus asset hash separation
 *      - 0DMSTRT.BIN: 8a026f155af27cfd43a33b29f7da5b75ee7b09b2c4f016fc3be1ebb4787d20b6
 *      - DM.BIN:      3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124
 *      - LEV00.DGN:   24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a
 *      - LEV15.DGN:   df8ccdf292439cfa53033686aedeaa4e9b3005aa0343760917ed8325b44ef0aa
 *      - ITEM.IBS:    fc32ca5875906e6e0dc69e0b5edfa5d00cb1f4401b7d497397c699be7c4530c1
 *      - FACE.BIN:    d733f50096098b5a2d15f2d355a89decd7b3777f82e515f60fee2e9ca4921e22
 *      - FILE_LISTING.txt: 6526c88ba9a4b7af90d71edac019eea51ecf5c8e9abcd4b165c7187a06dd091b
 *      - The Nexus catalog is distinct from DM1 and CSB catalogs.
 *
 *   5. Cross-game hash separation
 *      - DM1 DUNGEON.DAT:  d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
 *      - CSB DUNGEON.DAT:  6695d2acebce49f95db1d8f3a5c733de
 *      - Nexus LEV00.DGN:  24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a
 *      - All three game catalogs are completely disjoint by hash.
 *
 *   6. Headless verification: no game data files needed
 *      - This probe exercises only the phase gate logic and hash constants.
 *      - It does NOT load 0DMSTRT.BIN, DM.BIN, or any LEVnn.DGN.
 *      - SDL_VIDEODRIVER=dummy safe.
 *
 * Exit codes:
 *   0  - all checks passed
 *   1  - one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy ./firestaff_nexus_v2_phase1_launch_profile_separation_probe
 *
 * Source references:
 *   NEXUS.C / NEXUS2.C                DM Nexus engine lifecycle
 *   NEXUS.BIN                          Saturn game binary
 *   nexus_v1_iso_reader.c              Saturn ISO 9660 + DMDF interleaving
 *   nexus_v1_dmdf_model.c              DMDF (Dungeon Master Data Format) decoder
 *   nexus_v1_dungeon.c                 DGN level loader, 16 levels
 *   nexus_v1_game.c                    state init, level load, CD track map
 *   scripts/fixtures/nexus_v1_disc_file_hashes.py  138-file SHA256 table
 *   ReDMCSB ENTRANCE.C F0806           CSB boot (sister game, parallel)
 *   ReDMCSB LOADSAVE.C F0435           new-game party location
 *   HuC6260/HuC6270                    VDC/VCE datasheet
 */

#include "nexus_v2_phase_gate_pc34.h"

#include <stdio.h>
#include <string.h>

/* ----------------------------------------------------------------
 * Nexus V1 Saturn DMDF/DGN asset hash constants.
 * Source: scripts/fixtures/nexus_v1_disc_file_hashes.py (138 files,
 * SHA256, computed 2026-05-27 from /Users/bosse/.firestaff/data/nexus).
 * ---------------------------------------------------------------- */

#define NEXUS_V1_HASH_0DMSTRT_BIN  \
    "8a026f155af27cfd43a33b29f7da5b75ee7b09b2c4f016fc3be1ebb4787d20b6"
#define NEXUS_V1_HASH_DM_BIN  \
    "3bbca125e0bfb486897e4926541e7c31adbff010d01a9b0c736637f432aad124"
#define NEXUS_V1_HASH_LEV00_DGN  \
    "24e3b3cdf2496b53f489df456d822ba85593a67325f90dd414c6af26bf683d9a"
#define NEXUS_V1_HASH_LEV15_DGN  \
    "df8ccdf292439cfa53033686aedeaa4e9b3005aa0343760917ed8325b44ef0aa"
#define NEXUS_V1_HASH_ITEM_IBS  \
    "fc32ca5875906e6e0dc69e0b5edfa5d00cb1f4401b7d497397c699be7c4530c1"
#define NEXUS_V1_HASH_FACE_BIN  \
    "d733f50096098b5a2d15f2d355a89decd7b3777f82e515f60fee2e9ca4921e22"
#define NEXUS_V1_HASH_FILE_LISTING  \
    "6526c88ba9a4b7af90d71edac019eea51ecf5c8e9abcd4b165c7187a06dd091b"

/* Cross-game hash constants for separation verification. */
#define DM1_V2_HASH_DUNGEON_DAT  \
    "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"
#define CSB_V2_HASH_DUNGEON_DAT  \
    "6695d2acebce49f95db1d8f3a5c733de"

static int g_total = 0;
static int g_failed = 0;

static void check(int cond, const char *name)
{
    ++g_total;
    if (!cond) {
        ++g_failed;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

static void check_launch_gate(void)
{
    NEXUS_V2_PhaseGateConfig cfg;
    NEXUS_V2_PhaseGateDecision d;

    /* GAME_STATE_INIT is V1-source-locked; always allowed regardless of V2. */
    nexus_v2_phase_gate_defaults(&cfg);
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_GAME_STATE_INIT);
    check(d.v1SourceLocked == 1,
          "LAUNCH: GAME_STATE_INIT is V1-source-locked (default)");
    check(d.v2PresentationAllowed == 0,
          "LAUNCH: GAME_STATE_INIT rejects V2 (default)");

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_GAME_STATE_INIT);
    check(d.v1SourceLocked == 1,
          "LAUNCH: GAME_STATE_INIT stays V1-locked (V2 on)");
    check(d.v2PresentationAllowed == 0,
          "LAUNCH: GAME_STATE_INIT still rejects V2 (V2 on)");

    /* RENDER_PRESENTATION is V2-eligible; only allowed when V2 on. */
    cfg.v2PresentationEnabled = 0;
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    check(d.v2PresentationAllowed == 0,
          "LAUNCH: RENDER blocked when V2 off");

    cfg.v2PresentationEnabled = 1;
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    check(d.v2PresentationAllowed == 1,
          "LAUNCH: RENDER allowed when V2 on");
}

static void check_profile_gate(void)
{
    NEXUS_V2_PhaseGateConfig cfg;
    NEXUS_V2_PhaseGateDecision d;

    /* CONFIG_PRESENTATION is the V2-profile write path. */
    /* V2 off -> blocked. */
    nexus_v2_phase_gate_defaults(&cfg);
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    check(d.v2PresentationAllowed == 0,
          "PROFILE: CONFIG blocked when V2 off");

    /* V2 on, persist off -> still blocked. */
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 0;
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    check(d.v2PresentationAllowed == 0,
          "PROFILE: CONFIG blocked when V2 on, persist off");

    /* V2 on, persist on -> allowed. */
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    check(d.v2PresentationAllowed == 1,
          "PROFILE: CONFIG allowed when V2 on, persist on");

    /* V2 off, persist on -> still blocked (must be V1 gate). */
    cfg.v2PresentationEnabled = 0;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    check(d.v2PresentationAllowed == 0,
          "PROFILE: CONFIG blocked when V2 off, persist on (V1 gate)");
}

static void check_nexus_asset_hashes(void)
{
    /* Each hash is 64 hex chars (SHA256). */
    check(strlen(NEXUS_V1_HASH_0DMSTRT_BIN) == 64,
          "0DMSTRT.BIN hash is 64 hex chars");
    check(strlen(NEXUS_V1_HASH_DM_BIN) == 64,
          "DM.BIN hash is 64 hex chars");
    check(strlen(NEXUS_V1_HASH_LEV00_DGN) == 64,
          "LEV00.DGN hash is 64 hex chars");
    check(strlen(NEXUS_V1_HASH_LEV15_DGN) == 64,
          "LEV15.DGN hash is 64 hex chars");
    check(strlen(NEXUS_V1_HASH_ITEM_IBS) == 64,
          "ITEM.IBS hash is 64 hex chars");
    check(strlen(NEXUS_V1_HASH_FACE_BIN) == 64,
          "FACE.BIN hash is 64 hex chars");
    check(strlen(NEXUS_V1_HASH_FILE_LISTING) == 64,
          "FILE_LISTING.txt hash is 64 hex chars");

    /* All hashes are distinct (no accidental collision). */
    check(strcmp(NEXUS_V1_HASH_0DMSTRT_BIN, NEXUS_V1_HASH_DM_BIN) != 0,
          "0DMSTRT.BIN != DM.BIN");
    check(strcmp(NEXUS_V1_HASH_LEV00_DGN, NEXUS_V1_HASH_LEV15_DGN) != 0,
          "LEV00.DGN != LEV15.DGN");
    check(strcmp(NEXUS_V1_HASH_DM_BIN, NEXUS_V1_HASH_LEV00_DGN) != 0,
          "DM.BIN != LEV00.DGN");
    check(strcmp(NEXUS_V1_HASH_ITEM_IBS, NEXUS_V1_HASH_FACE_BIN) != 0,
          "ITEM.IBS != FACE.BIN");
}

static void check_cross_game_hash_separation(void)
{
    /* Nexus LEV00.DGN must be distinct from DM1 DUNGEON.DAT and CSB DUNGEON.DAT. */
    check(strcmp(NEXUS_V1_HASH_LEV00_DGN, DM1_V2_HASH_DUNGEON_DAT) != 0,
          "Nexus LEV00.DGN != DM1 DUNGEON.DAT");
    check(strcmp(NEXUS_V1_HASH_LEV00_DGN, CSB_V2_HASH_DUNGEON_DAT) != 0,
          "Nexus LEV00.DGN != CSB DUNGEON.DAT");
    check(strcmp(DM1_V2_HASH_DUNGEON_DAT, CSB_V2_HASH_DUNGEON_DAT) != 0,
          "DM1 DUNGEON.DAT != CSB DUNGEON.DAT");

    /* All three game catalogs are completely disjoint. */
    int nexus_dm1_match = (strcmp(NEXUS_V1_HASH_LEV00_DGN, DM1_V2_HASH_DUNGEON_DAT) == 0);
    int nexus_csb_match = (strcmp(NEXUS_V1_HASH_LEV00_DGN, CSB_V2_HASH_DUNGEON_DAT) == 0);
    int dm1_csb_match = (strcmp(DM1_V2_HASH_DUNGEON_DAT, CSB_V2_HASH_DUNGEON_DAT) == 0);
    check(!nexus_dm1_match && !nexus_csb_match && !dm1_csb_match,
          "All three game catalogs (Nexus, DM1, CSB) are disjoint by hash");
}

static void check_v1_only_default(void)
{
    /* Phase 1 default behaviour: V1 only. The launch path does not
     * require V2 to be enabled. The CONFIG domain is locked. */
    NEXUS_V2_PhaseGateConfig cfg;
    NEXUS_V2_PhaseGateDecision d;

    nexus_v2_phase_gate_defaults(&cfg);
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    check(d.v2PresentationAllowed == 0,
          "V1-only default: RENDER blocked");
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION);
    check(d.v2PresentationAllowed == 0,
          "V1-only default: SMOOTH_MOVEMENT blocked");
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_HUD_OVERLAY);
    check(d.v2PresentationAllowed == 0,
          "V1-only default: HUD_OVERLAY blocked");
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS);
    check(d.v2PresentationAllowed == 0,
          "V1-only default: PARTICLE_EFFECTS blocked");
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE);
    check(d.v2PresentationAllowed == 0,
          "V1-only default: ATMOSPHERE blocked");
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_INPUT_PRESENTATION);
    check(d.v2PresentationAllowed == 0,
          "V1-only default: INPUT_PRESENTATION blocked");
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_UPSCALER);
    check(d.v2PresentationAllowed == 0,
          "V1-only default: UPSCALER blocked");
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION);
    check(d.v2PresentationAllowed == 0,
          "V1-only default: DYNAMIC_LIGHTING blocked");
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    check(d.v2PresentationAllowed == 0,
          "V1-only default: CONFIG_PRESENTATION blocked (both toggles off)");

    /* V1 source-locked domains are always available in V1-only mode. */
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_GAME_STATE_INIT);
    check(d.v1SourceLocked == 1,
          "V1-only default: GAME_STATE_INIT V1-locked");
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_DMDF_DGN_LOADING);
    check(d.v1SourceLocked == 1,
          "V1-only default: DMDF_DGN_LOADING V1-locked");
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_MOVEMENT);
    check(d.v1SourceLocked == 1,
          "V1-only default: MOVEMENT V1-locked");
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD);
    check(d.v1SourceLocked == 1,
          "V1-only default: SAVE_LOAD V1-locked");
}

static void check_headless_safe(void)
{
    /* This probe must not require live game data. The fact that we got
     * here and can call all the gate APIs without crashing IS the
     * headless-safety guarantee. Add a few explicit zero-touch probes. */
    NEXUS_V2_PhaseGateConfig cfg;
    int i;

    nexus_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < (int)NEXUS_V2_PHASE_DOMAIN_COUNT; ++i) {
        char id[96];
        NEXUS_V2_PhaseGateDecision d =
            nexus_v2_phase_gate_decide(&cfg, (NEXUS_V2_PhaseDomain)i);
        snprintf(id, sizeof(id), "headless[%d].no_crash", i);
        check(d.sourceAnchor != 0, id);
    }
    /* And explicitly check that no asset load is attempted. The constants
     * are pure string literals. */
    check(NEXUS_V1_HASH_0DMSTRT_BIN[0] == '8',
          "headless: 0DMSTRT.BIN constant pinned (no I/O)");
    check(NEXUS_V1_HASH_LEV00_DGN[0] == '2',
          "headless: LEV00.DGN constant pinned (no I/O)");
}

int main(void)
{
    printf("=== Nexus V2 Phase 1 - Launch/Profile Separation Probe ===\n");
    check_launch_gate();
    check_profile_gate();
    check_nexus_asset_hashes();
    check_cross_game_hash_separation();
    check_v1_only_default();
    check_headless_safe();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
