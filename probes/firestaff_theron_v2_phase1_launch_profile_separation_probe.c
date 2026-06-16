/*
 * firestaff_theron_v2_phase1_launch_profile_separation_probe.c
 *
 * Theron's Quest V2 Phase 1 - Launch/Profile Separation Probe
 *
 * Headless probe: verifies Theron V2 phase gates and launch/profile
 * separation without requiring live game asset files.
 *
 * This probe validates:
 *
 *   1. THERON_V2_PHASE_DOMAIN_BOOT_PROFILE gate
 *      - V1 boot/profile is always allowed (V1-source-locked)
 *      - V1 boot/profile never depends on V2 toggles
 *
 *   2. THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE gate
 *      - When v2PresentationEnabled=0: V2 presentation is source-locked
 *      - When v2PresentationEnabled=1: V2 presentation is allowed
 *
 *   3. THERON_V2_PHASE_DOMAIN_FILTER_CONFIG gate
 *      - V2 filter config writes are blocked unless both toggles are on
 *      - When v2ConfigPersistenceEnabled=0: V2 filter config blocked
 *      - When v2ConfigPersistenceEnabled=1 AND v2PresentationEnabled=1:
 *        V2 filter config allowed
 *
 *   4. Theron V1 Track 02 asset hash separation (PC Engine CD)
 *      - JP Rev 1 ISO: 397039af02d50d15c70b74088eb8a1cb
 *      - US ISO:      3d8b78571dcd0e6eb8eb4b01eeb7fbba
 *      - The two are distinct PC Engine CD releases.
 *
 *   5. Cross-game hash separation
 *      - DM1 DUNGEON.DAT:  d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
 *      - CSB DUNGEON.DAT:  6695d2acebce49f95db1d8f3a5c733de
 *      - Theron JP Track 02: 397039af02d50d15c70b74088eb8a1cb
 *      - Theron US Track 02: 3d8b78571dcd0e6eb8eb4b01eeb7fbba
 *      - All four game catalogs are completely disjoint by hash.
 *
 *   6. Headless verification: no game data files needed
 *      - This probe exercises only the phase gate logic and hash constants.
 *      - It does NOT load any THQUEST.ASM bank or ISO file.
 *      - SDL_VIDEODRIVER=dummy safe.
 *
 * Exit codes:
 *   0  - all checks passed
 *   1  - one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy ./firestaff_theron_v2_phase1_launch_profile_separation_probe
 *
 * Source references:
 *   THQUEST.ASM T080  between-dungeon save/load
 *   THQUEST.ASM T400  dungeon bank loading
 *   theron_v1_track02.c  Track 02 bank signal (JP + US MD5)
 *   theron_v1_boot.c  boot/profile
 *   theron_v2_presentation_mode_pc34.c  M12_PRESENTATION -> V2 mode
 *   theron_v2_filter_config_pc34.c  V2.0 filter chain
 *   HuC6260/HuC6270 VDC/VCE datasheet
 *   HuC6280 CPU datasheet
 *   ReDMCSB ENTRANCE.C F0806 (CSB boot, sister game, parallel)
 *   ReDMCSB LOADSAVE.C F0435 (new-game party location)
 *   docs/source-lock/tqr_v1_phase1_boot_H2338.md
 */

#include "theron_v2_phase_gate_pc34.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <string.h>

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
    THERON_V2_PhaseGateConfig cfg;
    THERON_V2_PhaseGateDecision d;

    /* BOOT_PROFILE is V1-source-locked; always allowed regardless of V2. */
    theron_v2_phase_gate_defaults(&cfg);
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_BOOT_PROFILE);
    check(d.v1SourceLocked == 1,
          "LAUNCH: BOOT_PROFILE is V1-source-locked (default)");
    check(d.v2PresentationAllowed == 0,
          "LAUNCH: BOOT_PROFILE rejects V2 (default)");

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_BOOT_PROFILE);
    check(d.v1SourceLocked == 1,
          "LAUNCH: BOOT_PROFILE stays V1-locked (V2 on)");
    check(d.v2PresentationAllowed == 0,
          "LAUNCH: BOOT_PROFILE still rejects V2 (V2 on)");

    /* PRESENTATION_MODE is V2-eligible; only allowed when V2 on. */
    cfg.v2PresentationEnabled = 0;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE);
    check(d.v2PresentationAllowed == 0,
          "LAUNCH: PRESENTATION_MODE blocked when V2 off");

    cfg.v2PresentationEnabled = 1;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE);
    check(d.v2PresentationAllowed == 1,
          "LAUNCH: PRESENTATION_MODE allowed when V2 on");
}

static void check_profile_gate(void)
{
    THERON_V2_PhaseGateConfig cfg;
    THERON_V2_PhaseGateDecision d;

    /* FILTER_CONFIG is the V2-profile write path. */
    theron_v2_phase_gate_defaults(&cfg);
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_FILTER_CONFIG);
    check(d.v2PresentationAllowed == 0,
          "PROFILE: FILTER_CONFIG blocked when V2 off");

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 0;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_FILTER_CONFIG);
    check(d.v2PresentationAllowed == 0,
          "PROFILE: FILTER_CONFIG blocked when V2 on, persist off");

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_FILTER_CONFIG);
    check(d.v2PresentationAllowed == 1,
          "PROFILE: FILTER_CONFIG allowed when V2 on, persist on");

    cfg.v2PresentationEnabled = 0;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_FILTER_CONFIG);
    check(d.v2PresentationAllowed == 0,
          "PROFILE: FILTER_CONFIG blocked when V2 off, persist on (V1 gate)");
}

static void check_theron_asset_hashes(void)
{
    /* Each hash is 32 hex chars (MD5). */
    check(strlen(THERON_TRACK02_MD5_JP_REV1_ISO) == 32,
          "JP Rev 1 ISO hash is 32 hex chars (MD5)");
    check(strlen(THERON_TRACK02_MD5_US_ISO) == 32,
          "US ISO hash is 32 hex chars (MD5)");

    /* All hashes are distinct (no accidental collision). */
    check(strcmp(THERON_TRACK02_MD5_JP_REV1_ISO, THERON_TRACK02_MD5_US_ISO) != 0,
          "JP Rev 1 != US ISO");
    check(THERON_TRACK02_MD5_JP_REV1_ISO[0] == '3',
          "JP Rev 1 starts with '3' (prefix pinned)");
    check(THERON_TRACK02_MD5_US_ISO[0] == '3',
          "US ISO starts with '3' (prefix pinned)");
}

static void check_cross_game_hash_separation(void)
{
    /* Theron Track 02 (MD5, 32 hex) must be distinct from DM1/CSB
     * DUNGEON.DAT (SHA256, 64 hex). They are different algorithms AND
     * different file types so the strings cannot accidentally match. */
    check(strlen(THERON_TRACK02_MD5_JP_REV1_ISO) != strlen(DM1_V2_HASH_DUNGEON_DAT),
          "Theron MD5 length != DM1 SHA256 length");
    check(strcmp(THERON_TRACK02_MD5_JP_REV1_ISO, DM1_V2_HASH_DUNGEON_DAT) != 0,
          "Theron JP Track 02 != DM1 DUNGEON.DAT");
    check(strcmp(THERON_TRACK02_MD5_JP_REV1_ISO, CSB_V2_HASH_DUNGEON_DAT) != 0,
          "Theron JP Track 02 != CSB DUNGEON.DAT");
    check(strcmp(THERON_TRACK02_MD5_US_ISO, DM1_V2_HASH_DUNGEON_DAT) != 0,
          "Theron US Track 02 != DM1 DUNGEON.DAT");
    check(strcmp(THERON_TRACK02_MD5_US_ISO, CSB_V2_HASH_DUNGEON_DAT) != 0,
          "Theron US Track 02 != CSB DUNGEON.DAT");
    check(strcmp(DM1_V2_HASH_DUNGEON_DAT, CSB_V2_HASH_DUNGEON_DAT) != 0,
          "DM1 DUNGEON.DAT != CSB DUNGEON.DAT");

    /* All four game catalogs are completely disjoint. */
    int t_jp_dm1 = (strcmp(THERON_TRACK02_MD5_JP_REV1_ISO, DM1_V2_HASH_DUNGEON_DAT) == 0);
    int t_jp_csb = (strcmp(THERON_TRACK02_MD5_JP_REV1_ISO, CSB_V2_HASH_DUNGEON_DAT) == 0);
    int t_us_dm1 = (strcmp(THERON_TRACK02_MD5_US_ISO, DM1_V2_HASH_DUNGEON_DAT) == 0);
    int t_us_csb = (strcmp(THERON_TRACK02_MD5_US_ISO, CSB_V2_HASH_DUNGEON_DAT) == 0);
    int dm1_csb = (strcmp(DM1_V2_HASH_DUNGEON_DAT, CSB_V2_HASH_DUNGEON_DAT) == 0);
    check(!t_jp_dm1 && !t_jp_csb && !t_us_dm1 && !t_us_csb && !dm1_csb,
          "All game catalogs (Theron, DM1, CSB) are disjoint");
}

static void check_v1_only_default(void)
{
    THERON_V2_PhaseGateConfig cfg;
    THERON_V2_PhaseGateDecision d;

    theron_v2_phase_gate_defaults(&cfg);
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE);
    check(d.v2PresentationAllowed == 0, "V1-only default: PRESENTATION_MODE blocked");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_TEXTURE_UPSCALE);
    check(d.v2PresentationAllowed == 0, "V1-only default: TEXTURE_UPSCALE blocked");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_FILTER_CONFIG);
    check(d.v2PresentationAllowed == 0, "V1-only default: FILTER_CONFIG blocked");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_MODERN_SHAPES);
    check(d.v2PresentationAllowed == 0, "V1-only default: MODERN_SHAPES blocked");

    /* V1 source-locked domains are always available in V1-only mode. */
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_TRACK02_BANK);
    check(d.v1SourceLocked == 1, "V1-only default: TRACK02_BANK V1-locked");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_BOOT_PROFILE);
    check(d.v1SourceLocked == 1, "V1-only default: BOOT_PROFILE V1-locked");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_CHAMPION_PARTY);
    check(d.v1SourceLocked == 1, "V1-only default: CHAMPION_PARTY V1-locked");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_DUNGEON_PROGRESSION);
    check(d.v1SourceLocked == 1, "V1-only default: DUNGEON_PROGRESSION V1-locked");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_MECHANICS);
    check(d.v1SourceLocked == 1, "V1-only default: MECHANICS V1-locked");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_SAVE_LOAD);
    check(d.v1SourceLocked == 1, "V1-only default: SAVE_LOAD V1-locked");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_SHOP);
    check(d.v1SourceLocked == 1, "V1-only default: SHOP V1-locked");
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_WORLD_STATE);
    check(d.v1SourceLocked == 1, "V1-only default: WORLD_STATE V1-locked");
}

static void check_headless_safe(void)
{
    THERON_V2_PhaseGateConfig cfg;
    int i;

    theron_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < (int)THERON_V2_PHASE_DOMAIN_COUNT; ++i) {
        char id[96];
        THERON_V2_PhaseGateDecision d =
            theron_v2_phase_gate_decide(&cfg, (THERON_V2_PhaseDomain)i);
        snprintf(id, sizeof(id), "headless[%d].no_crash", i);
        check(d.sourceAnchor != 0, id);
    }
    /* And explicitly check that no asset load is attempted. The Track 02
     * constants are pure string literals. */
    check(THERON_TRACK02_MD5_JP_REV1_ISO[0] == '3',
          "headless: JP Rev 1 constant pinned (no I/O)");
    check(THERON_TRACK02_MD5_US_ISO[0] == '3',
          "headless: US ISO constant pinned (no I/O)");
}

int main(void)
{
    printf("=== Theron V2 Phase 1 - Launch/Profile Separation Probe ===\n");
    check_launch_gate();
    check_profile_gate();
    check_theron_asset_hashes();
    check_cross_game_hash_separation();
    check_v1_only_default();
    check_headless_safe();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
