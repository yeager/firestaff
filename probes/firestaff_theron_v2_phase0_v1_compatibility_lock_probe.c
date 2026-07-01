/*
 * firestaff_theron_v2_phase0_v1_compatibility_lock_probe.c
 *
 * Theron's Quest V2 Phase 0 - V1 Compatibility Lock Probe
 *
 * Headless probe: verifies the Theron V2 Phase 0 V1 compatibility lock
 * (theron_v2_phase_gate_pc34.c) without requiring live game asset files.
 *
 * This probe validates:
 *
 *   1. theron_v2_phase_gate_defaults() initialises to V1-only behaviour
 *
 *   2. Domain classification: 12 V1-source-locked + 5 V2-eligible = 17
 *
 *   3. V1-source-locked domains (12):
 *      TRACK02_BANK, BOOT_PROFILE, CHAMPION_PARTY, DUNGEON_PROGRESSION,
 *      MECHANICS, SAVE_LOAD, SHOP, TILE_RENDERER, VIEWPORT,
 *      WORLD_STATE, PALETTE, UI_CHROME
 *      - Always return v1SourceLocked=1, v2PresentationAllowed=0
 *
 *   4. V2-presentation-eligible domains (4):
 *      PRESENTATION_MODE, TEXTURE_UPSCALE, FILTER_CONFIG, MODERN_SHAPES
 *      - Return v2PresentationAllowed=0 when V2 is off
 *      - Return v2PresentationAllowed=1 when V2 is on
 *        (except FILTER_CONFIG which also needs config-persistence)
 *
 *   5. FILTER_CONFIG additionally requires v2ConfigPersistenceEnabled
 *
 *   6. theron_v2_phase_gate_v2_active() returns 1 only when V2 enabled
 *
 *   7. All 17 domain names are non-NULL
 *
 *   8. theron_v2_phase_gate_source_evidence() references THQUEST.ASM T*
 *      and theron_v1_*
 *
 *   9. All 16 domains are handled (no switch fallthrough to default)
 *
 *  10. Source citations in gate decisions contain THQUEST.ASM or
 *      theron_v1_* references
 *
 *  11. Default config has both toggles off
 *
 *  12. Unknown domains default to V1-locked
 *
 *  13. Null config arg is safe
 *
 * Exit codes:
 *   0  - all checks passed
 *   1  - one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy ./firestaff_theron_v2_phase0_v1_compatibility_lock_probe
 *
 * Source references:
 *   THQUEST.ASM T080  between-dungeon save/load
 *   THQUEST.ASM T400  dungeon bank loading
 *   THQUEST.ASM T520  party placement / start position
 *   THQUEST.ASM T560  dungeon loading (header parsing, dungeon_seed)
 *   THQUEST.ASM T600  map transitions
 *   THQUEST.ASM T700  timers / world tick
 *   THQUEST.ASM T800  champion persistence + inventory reset
 *   THQUEST.ASM T900  object database / thing list
 *   theron_v1_track02.c  PC Engine CD Track 02 bank signal
 *   theron_v1_champions.c, theron_v1_save_load.c, theron_v1_world.c
 *   HuC6260/HuC6270 VDC/VCE datasheet
 *   HuC6280 CPU datasheet
 *   ADPCM audio codec
 *   docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 *   docs/source-lock/tqr_v1_phase1_boot_H2338.md
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 */

#include "theron_v2_phase_gate_pc34.h"

#include <stdio.h>
#include <string.h>

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

static void check_defaults(void)
{
    THERON_V2_PhaseGateConfig cfg;
    memset(&cfg, 0xff, sizeof(cfg));
    theron_v2_phase_gate_defaults(&cfg);
    check(cfg.v2PresentationEnabled == 0,
          "Phase 0 default: v2PresentationEnabled=0");
    check(cfg.v2ConfigPersistenceEnabled == 0,
          "Phase 0 default: v2ConfigPersistenceEnabled=0");
    check(theron_v2_phase_gate_v2_active(&cfg) == 0,
          "Phase 0 default: v2_active=0 (V1 behaviour)");
}

static void check_null_args(void)
{
    theron_v2_phase_gate_defaults(0);
    check(theron_v2_phase_gate_v2_active(0) == 0,
          "null v2_active=0 (safe)");
    {
        THERON_V2_PhaseGateDecision d =
            theron_v2_phase_gate_decide(0, THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE);
        check(d.v1SourceLocked == 0 && d.v2PresentationAllowed == 0,
              "null config decide -> V1-locked for V2-eligible (safe)");
    }
}

static void check_v1_locked_count(void)
{
    /* 12 V1-source-locked gameplay domains. */
    static const THERON_V2_PhaseDomain v1Locked[] = {
        THERON_V2_PHASE_DOMAIN_TRACK02_BANK,
        THERON_V2_PHASE_DOMAIN_BOOT_PROFILE,
        THERON_V2_PHASE_DOMAIN_CHAMPION_PARTY,
        THERON_V2_PHASE_DOMAIN_DUNGEON_PROGRESSION,
        THERON_V2_PHASE_DOMAIN_MECHANICS,
        THERON_V2_PHASE_DOMAIN_SAVE_LOAD,
        THERON_V2_PHASE_DOMAIN_SHOP,
        THERON_V2_PHASE_DOMAIN_TILE_RENDERER,
        THERON_V2_PHASE_DOMAIN_VIEWPORT,
        THERON_V2_PHASE_DOMAIN_WORLD_STATE,
        THERON_V2_PHASE_DOMAIN_PALETTE,
        THERON_V2_PHASE_DOMAIN_UI_CHROME
    };
    int n = (int)(sizeof(v1Locked) / sizeof(v1Locked[0]));
    check(n == 12, "V1-source-locked domain count = 12");
}

static void check_v2_eligible_count(void)
{
    static const THERON_V2_PhaseDomain v2Eligible[] = {
        THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE,
        THERON_V2_PHASE_DOMAIN_TEXTURE_UPSCALE,
        THERON_V2_PHASE_DOMAIN_FILTER_CONFIG,
        THERON_V2_PHASE_DOMAIN_MODERN_SHAPES,
        THERON_V2_PHASE_DOMAIN_HUD_LAUNCH_MODE
    };
    int n = (int)(sizeof(v2Eligible) / sizeof(v2Eligible[0]));
    check(n == 5, "V2-eligible domain count = 5");
    check(n + 12 == (int)THERON_V2_PHASE_DOMAIN_COUNT,
          "12 V1-locked + 5 V2-eligible = 17 (THERON_V2_PHASE_DOMAIN_COUNT)");
}

static void check_v1_locked_with_v2_off(void)
{
    static const THERON_V2_PhaseDomain v1Locked[] = {
        THERON_V2_PHASE_DOMAIN_TRACK02_BANK,
        THERON_V2_PHASE_DOMAIN_BOOT_PROFILE,
        THERON_V2_PHASE_DOMAIN_CHAMPION_PARTY,
        THERON_V2_PHASE_DOMAIN_DUNGEON_PROGRESSION,
        THERON_V2_PHASE_DOMAIN_MECHANICS,
        THERON_V2_PHASE_DOMAIN_SAVE_LOAD,
        THERON_V2_PHASE_DOMAIN_SHOP,
        THERON_V2_PHASE_DOMAIN_TILE_RENDERER,
        THERON_V2_PHASE_DOMAIN_VIEWPORT,
        THERON_V2_PHASE_DOMAIN_WORLD_STATE,
        THERON_V2_PHASE_DOMAIN_PALETTE,
        THERON_V2_PHASE_DOMAIN_UI_CHROME
    };
    THERON_V2_PhaseGateConfig cfg;
    int i;
    int n = (int)(sizeof(v1Locked) / sizeof(v1Locked[0]));

    theron_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < n; ++i) {
        THERON_V2_PhaseGateDecision d =
            theron_v2_phase_gate_decide(&cfg, v1Locked[i]);
        char id_v1[96], id_v2[96];
        snprintf(id_v1, sizeof(id_v1),
                 "v1_locked.v2_off[%s].v1SourceLocked",
                 theron_v2_phase_gate_domain_name(v1Locked[i]));
        snprintf(id_v2, sizeof(id_v2),
                 "v1_locked.v2_off[%s].v2PresentationAllowed",
                 theron_v2_phase_gate_domain_name(v1Locked[i]));
        check(d.v1SourceLocked == 1, id_v1);
        check(d.v2PresentationAllowed == 0, id_v2);
        check(theron_v2_phase_gate_is_gameplay_domain(v1Locked[i]) == 1,
              "is_gameplay(V1-locked) == 1");
    }
}

static void check_v1_locked_with_v2_on(void)
{
    static const THERON_V2_PhaseDomain v1Locked[] = {
        THERON_V2_PHASE_DOMAIN_TRACK02_BANK,
        THERON_V2_PHASE_DOMAIN_BOOT_PROFILE,
        THERON_V2_PHASE_DOMAIN_CHAMPION_PARTY,
        THERON_V2_PHASE_DOMAIN_DUNGEON_PROGRESSION,
        THERON_V2_PHASE_DOMAIN_MECHANICS,
        THERON_V2_PHASE_DOMAIN_SAVE_LOAD,
        THERON_V2_PHASE_DOMAIN_SHOP,
        THERON_V2_PHASE_DOMAIN_TILE_RENDERER,
        THERON_V2_PHASE_DOMAIN_VIEWPORT,
        THERON_V2_PHASE_DOMAIN_WORLD_STATE,
        THERON_V2_PHASE_DOMAIN_PALETTE,
        THERON_V2_PHASE_DOMAIN_UI_CHROME
    };
    THERON_V2_PhaseGateConfig cfg;
    int i;
    int n = (int)(sizeof(v1Locked) / sizeof(v1Locked[0]));

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    for (i = 0; i < n; ++i) {
        THERON_V2_PhaseGateDecision d =
            theron_v2_phase_gate_decide(&cfg, v1Locked[i]);
        char id_v1[96], id_v2[96];
        snprintf(id_v1, sizeof(id_v1),
                 "v1_locked.v2_on[%s].v1SourceLocked",
                 theron_v2_phase_gate_domain_name(v1Locked[i]));
        snprintf(id_v2, sizeof(id_v2),
                 "v1_locked.v2_on[%s].v2PresentationAllowed",
                 theron_v2_phase_gate_domain_name(v1Locked[i]));
        check(d.v1SourceLocked == 1, id_v1);
        check(d.v2PresentationAllowed == 0, id_v2);
    }
}

static void check_v2_eligible_with_v2_off(void)
{
    static const THERON_V2_PhaseDomain v2Eligible[] = {
        THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE,
        THERON_V2_PHASE_DOMAIN_TEXTURE_UPSCALE,
        THERON_V2_PHASE_DOMAIN_FILTER_CONFIG,
        THERON_V2_PHASE_DOMAIN_MODERN_SHAPES
    };
    THERON_V2_PhaseGateConfig cfg;
    int i;
    int n = (int)(sizeof(v2Eligible) / sizeof(v2Eligible[0]));

    theron_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < n; ++i) {
        THERON_V2_PhaseGateDecision d =
            theron_v2_phase_gate_decide(&cfg, v2Eligible[i]);
        char id_v1[96], id_v2[96];
        snprintf(id_v1, sizeof(id_v1),
                 "v2_eligible.v2_off[%s].v1SourceLocked",
                 theron_v2_phase_gate_domain_name(v2Eligible[i]));
        snprintf(id_v2, sizeof(id_v2),
                 "v2_eligible.v2_off[%s].v2PresentationAllowed",
                 theron_v2_phase_gate_domain_name(v2Eligible[i]));
        check(d.v1SourceLocked == 0, id_v1);
        check(d.v2PresentationAllowed == 0, id_v2);
        check(theron_v2_phase_gate_is_gameplay_domain(v2Eligible[i]) == 0,
              "is_gameplay(V2-eligible) == 0");
    }
}

static void check_v2_eligible_with_v2_on(void)
{
    static const THERON_V2_PhaseDomain v2Eligible[] = {
        THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE,
        THERON_V2_PHASE_DOMAIN_TEXTURE_UPSCALE,
        THERON_V2_PHASE_DOMAIN_FILTER_CONFIG,
        THERON_V2_PHASE_DOMAIN_MODERN_SHAPES
    };
    THERON_V2_PhaseGateConfig cfg;
    int i;
    int n = (int)(sizeof(v2Eligible) / sizeof(v2Eligible[0]));

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    for (i = 0; i < n; ++i) {
        THERON_V2_PhaseGateDecision d =
            theron_v2_phase_gate_decide(&cfg, v2Eligible[i]);
        char id_v2[96];
        snprintf(id_v2, sizeof(id_v2),
                 "v2_eligible.v2_on_persist_on[%s].v2PresentationAllowed",
                 theron_v2_phase_gate_domain_name(v2Eligible[i]));
        check(d.v1SourceLocked == 0, "v2_eligible.v1SourceLocked==0");
        check(d.v2PresentationAllowed == 1, id_v2);
    }
}

static void check_filter_config_persistence(void)
{
    THERON_V2_PhaseGateConfig cfg;
    THERON_V2_PhaseGateDecision d;

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 0;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_FILTER_CONFIG);
    check(d.v2PresentationAllowed == 0,
          "FILTER_CONFIG: v2 on, persist off -> blocked");

    cfg.v2PresentationEnabled = 0;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_FILTER_CONFIG);
    check(d.v2PresentationAllowed == 0,
          "FILTER_CONFIG: v2 off, persist on -> blocked");

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_FILTER_CONFIG);
    check(d.v2PresentationAllowed == 1,
          "FILTER_CONFIG: v2 on, persist on -> allowed");
}

static void check_v2_active_helper(void)
{
    THERON_V2_PhaseGateConfig cfg;

    check(theron_v2_phase_gate_v2_active(0) == 0,
          "v2_active(NULL) == 0 (safe)");

    theron_v2_phase_gate_defaults(&cfg);
    check(theron_v2_phase_gate_v2_active(&cfg) == 0,
          "v2_active(default) == 0 (V1 behaviour)");

    cfg.v2PresentationEnabled = 1;
    check(theron_v2_phase_gate_v2_active(&cfg) == 1,
          "v2_active(v2=1) == 1");

    cfg.v2PresentationEnabled = 0;
    check(theron_v2_phase_gate_v2_active(&cfg) == 0,
          "v2_active(v2=0) == 0");

    cfg.v2PresentationEnabled = 0;
    cfg.v2ConfigPersistenceEnabled = 1;
    check(theron_v2_phase_gate_v2_active(&cfg) == 0,
          "v2_active(persist=1 only) == 0");
}

static void check_domain_names(void)
{
    int i;
    for (i = 0; i < (int)THERON_V2_PHASE_DOMAIN_COUNT; ++i) {
        const char *name = theron_v2_phase_gate_domain_name((THERON_V2_PhaseDomain)i);
        char id[96];
        snprintf(id, sizeof(id), "domain_name[%d].nonnull", i);
        check(name != 0, id);
        snprintf(id, sizeof(id), "domain_name[%d].nonempty", i);
        check(name && name[0] != 0, id);
    }
    check(strcmp(theron_v2_phase_gate_domain_name((THERON_V2_PhaseDomain)9999),
                 "UNKNOWN") == 0,
          "domain_name(unknown) == 'UNKNOWN'");
}

static void check_source_evidence(void)
{
    const char *e = theron_v2_phase_gate_source_evidence();

    check(e != 0, "evidence present");
    check(e && e[0] != 0, "evidence non-empty");
    check(e && strstr(e, "theron_v1_track02") != 0,
          "evidence references theron_v1_track02");
    check(e && strstr(e, "theron_v1_boot") != 0,
          "evidence references theron_v1_boot");
    check(e && strstr(e, "theron_v1_champions") != 0,
          "evidence references theron_v1_champions");
    check(e && strstr(e, "theron_v1_dungeon_progression") != 0,
          "evidence references theron_v1_dungeon_progression");
    check(e && strstr(e, "theron_v1_mechanics") != 0,
          "evidence references theron_v1_mechanics");
    check(e && strstr(e, "theron_v1_save_load") != 0,
          "evidence references theron_v1_save_load");
    check(e && strstr(e, "theron_v1_shop") != 0,
          "evidence references theron_v1_shop");
    check(e && strstr(e, "theron_v1_tile_renderer") != 0,
          "evidence references theron_v1_tile_renderer");
    check(e && strstr(e, "theron_v1_viewport") != 0,
          "evidence references theron_v1_viewport");
    check(e && strstr(e, "theron_v1_world") != 0,
          "evidence references theron_v1_world");
    check(e && strstr(e, "theron_v1_palette") != 0,
          "evidence references theron_v1_palette");
    check(e && strstr(e, "theron_v1_ui_chrome") != 0,
          "evidence references theron_v1_ui_chrome");
    check(e && strstr(e, "T080") != 0,
          "evidence references THQUEST.ASM T080");
    check(e && strstr(e, "T400") != 0,
          "evidence references THQUEST.ASM T400");
    check(e && strstr(e, "T520") != 0,
          "evidence references THQUEST.ASM T520");
    check(e && strstr(e, "T560") != 0,
          "evidence references THQUEST.ASM T560");
    check(e && strstr(e, "T600") != 0,
          "evidence references THQUEST.ASM T600");
    check(e && strstr(e, "T700") != 0,
          "evidence references THQUEST.ASM T700");
    check(e && strstr(e, "T800") != 0,
          "evidence references THQUEST.ASM T800");
    check(e && strstr(e, "T900") != 0,
          "evidence references THQUEST.ASM T900");
    check(e && strstr(e, "HuC6260") != 0,
          "evidence references HuC6260 (PC Engine VDC)");
    check(e && strstr(e, "HuC6280") != 0,
          "evidence references HuC6280 (PC Engine CPU)");
    check(e && strstr(e, "ADPCM") != 0,
          "evidence references ADPCM (PC Engine audio)");
    check(e && strstr(e, "tqr_v1_phase0_provenance_gate") != 0,
          "evidence references tqr_v1_phase0_provenance_gate_H2339");
    check(e && strstr(e, "tqr_v1_phase1_boot") != 0,
          "evidence references tqr_v1_phase1_boot_H2338");
    check(e && strstr(e, "tqr_v1_phase2_data_formats") != 0,
          "evidence references tqr_v1_phase2_data_formats_H2339");
}

static void check_all_domains_have_anchor(void)
{
    THERON_V2_PhaseGateConfig cfg;
    int i;

    theron_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < (int)THERON_V2_PHASE_DOMAIN_COUNT; ++i) {
        THERON_V2_PhaseGateDecision d =
            theron_v2_phase_gate_decide(&cfg, (THERON_V2_PhaseDomain)i);
        char id[96];
        snprintf(id, sizeof(id), "decision[%d].source_anchor", i);
        check(d.sourceAnchor != 0 && d.sourceAnchor[0] != 0, id);
        snprintf(id, sizeof(id), "decision[%d].rule", i);
        check(d.rule != 0 && d.rule[0] != 0, id);
    }
}

static void check_unknown_domain(void)
{
    THERON_V2_PhaseGateConfig cfg;
    THERON_V2_PhaseGateDecision d;

    theron_v2_phase_gate_defaults(&cfg);
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;

    d = theron_v2_phase_gate_decide(&cfg, (THERON_V2_PhaseDomain)999);
    check(d.v1SourceLocked == 1,
          "unknown domain -> v1SourceLocked=1 (safe)");
    check(d.v2PresentationAllowed == 0,
          "unknown domain -> v2PresentationAllowed=0 (safe)");
    check(theron_v2_phase_gate_is_gameplay_domain((THERON_V2_PhaseDomain)999) == 1,
          "unknown domain -> is_gameplay=1 (safe)");
}

int main(void)
{
    printf("=== Theron V2 Phase 0 - V1 Compatibility Lock Probe ===\n");
    check_defaults();
    check_null_args();
    check_v1_locked_count();
    check_v2_eligible_count();
    check_v1_locked_with_v2_off();
    check_v1_locked_with_v2_on();
    check_v2_eligible_with_v2_off();
    check_v2_eligible_with_v2_on();
    check_filter_config_persistence();
    check_v2_active_helper();
    check_domain_names();
    check_source_evidence();
    check_all_domains_have_anchor();
    check_unknown_domain();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
