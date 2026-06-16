/*
 * test_theron_v2_phase_gate_pc34.c
 *
 * Unit test for Theron V2 Phase 0 V1 compatibility lock and
 * Phase 1 launch/profile separation. Validates:
 *
 *  - theron_v2_phase_gate_defaults() initialises to V1-only behaviour
 *  - Domain classification: 12 V1-source-locked + 4 V2-eligible = 16
 *  - V1-source-locked domains always return v1SourceLocked=1
 *  - V2-eligible domains return v2PresentationAllowed=0 when V2 is off
 *  - V2-eligible domains return v2PresentationAllowed=1 when V2 is on
 *  - FILTER_CONFIG additionally requires v2ConfigPersistenceEnabled
 *  - theron_v2_phase_gate_v2_active() returns 1 only when v2PresentationEnabled=1
 *  - All 17 domain names are non-NULL
 *  - Source evidence references THQUEST.ASM T* / theron_v1_*
 *  - Unknown domains default to V1-locked
 *  - Default config has both toggles off
 *  - Source evidence is non-empty
 *  - Track 02 asset hashes are pinned
 *
 * Source-lock:
 *  - include/theron_v2_phase_gate_pc34.h
 *  - src/theron/theron_v2_phase_gate_pc34.c
 */

#include "theron_v2_phase_gate_pc34.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_passed = 0;

static void check_int(const char *id, int got, int want)
{
    ++g_assertions;
    if (got == want) {
        ++g_passed;
    } else {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
    }
}

static void check_true(const char *id, int condition)
{
    ++g_assertions;
    if (condition) {
        ++g_passed;
    } else {
        printf("FAIL %s\n", id);
    }
}

static void check_default_config(void)
{
    THERON_V2_PhaseGateConfig cfg;
    memset(&cfg, 0xff, sizeof(cfg));
    theron_v2_phase_gate_defaults(&cfg);
    check_int("default.v2PresentationEnabled", cfg.v2PresentationEnabled, 0);
    check_int("default.v2ConfigPersistenceEnabled", cfg.v2ConfigPersistenceEnabled, 0);
    check_int("default.v2_active", theron_v2_phase_gate_v2_active(&cfg), 0);
}

static void check_null_args(void)
{
    theron_v2_phase_gate_defaults(0);
    check_int("null.v2_active", theron_v2_phase_gate_v2_active(0), 0);
    {
        THERON_V2_PhaseGateDecision d =
            theron_v2_phase_gate_decide(0, THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE);
        check_int("null.decision.v1SourceLocked", d.v1SourceLocked, 0);
        check_int("null.decision.v2PresentationAllowed", d.v2PresentationAllowed, 0);
    }
}

static void check_v1_locked_domains(void)
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
    THERON_V2_PhaseGateDecision d;
    size_t i;
    int n = (int)(sizeof(v1Locked) / sizeof(v1Locked[0]));

    check_int("v1_locked.count", n, 12);

    theron_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < (size_t)n; ++i) {
        char id[96];
        d = theron_v2_phase_gate_decide(&cfg, v1Locked[i]);
        snprintf(id, sizeof(id), "v1_locked.v2_off[%zu].v1SourceLocked", i);
        check_int(id, d.v1SourceLocked, 1);
        snprintf(id, sizeof(id), "v1_locked.v2_off[%zu].v2PresentationAllowed", i);
        check_int(id, d.v2PresentationAllowed, 0);
        snprintf(id, sizeof(id), "v1_locked.v2_off[%zu].is_gameplay", i);
        check_int(id, theron_v2_phase_gate_is_gameplay_domain(v1Locked[i]), 1);
        snprintf(id, sizeof(id), "v1_locked.v2_off[%zu].name_nonnull", i);
        check_true(id, theron_v2_phase_gate_domain_name(v1Locked[i]) != 0);
    }

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    for (i = 0; i < (size_t)n; ++i) {
        char id[96];
        d = theron_v2_phase_gate_decide(&cfg, v1Locked[i]);
        snprintf(id, sizeof(id), "v1_locked.v2_on[%zu].v1SourceLocked", i);
        check_int(id, d.v1SourceLocked, 1);
        snprintf(id, sizeof(id), "v1_locked.v2_on[%zu].v2PresentationAllowed", i);
        check_int(id, d.v2PresentationAllowed, 0);
    }
}

static void check_v2_eligible_domains(void)
{
    static const THERON_V2_PhaseDomain v2Eligible[] = {
        THERON_V2_PHASE_DOMAIN_PRESENTATION_MODE,
        THERON_V2_PHASE_DOMAIN_TEXTURE_UPSCALE,
        THERON_V2_PHASE_DOMAIN_FILTER_CONFIG,
        THERON_V2_PHASE_DOMAIN_MODERN_SHAPES
    };
    THERON_V2_PhaseGateConfig cfg;
    THERON_V2_PhaseGateDecision d;
    size_t i;
    int n = (int)(sizeof(v2Eligible) / sizeof(v2Eligible[0]));

    check_int("v2_eligible.count", n, 4);
    check_int("total", n + 12, (int)THERON_V2_PHASE_DOMAIN_COUNT);
    check_int("domain_count", (int)THERON_V2_PHASE_DOMAIN_COUNT, 16);

    theron_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < (size_t)n; ++i) {
        char id[96];
        d = theron_v2_phase_gate_decide(&cfg, v2Eligible[i]);
        snprintf(id, sizeof(id), "v2_eligible.v2_off[%zu].v1SourceLocked", i);
        check_int(id, d.v1SourceLocked, 0);
        snprintf(id, sizeof(id), "v2_eligible.v2_off[%zu].v2PresentationAllowed", i);
        check_int(id, d.v2PresentationAllowed, 0);
        snprintf(id, sizeof(id), "v2_eligible.v2_off[%zu].is_not_gameplay", i);
        check_int(id, theron_v2_phase_gate_is_gameplay_domain(v2Eligible[i]), 0);
    }

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 0;
    for (i = 0; i < (size_t)n; ++i) {
        char id[96];
        d = theron_v2_phase_gate_decide(&cfg, v2Eligible[i]);
        snprintf(id, sizeof(id), "v2_eligible.v2_on_persist_off[%zu].v1SourceLocked", i);
        check_int(id, d.v1SourceLocked, 0);
        if (v2Eligible[i] == THERON_V2_PHASE_DOMAIN_FILTER_CONFIG) {
            snprintf(id, sizeof(id), "v2_eligible.v2_on_persist_off[%zu].filter_blocked", i);
            check_int(id, d.v2PresentationAllowed, 0);
        } else {
            snprintf(id, sizeof(id), "v2_eligible.v2_on_persist_off[%zu].v2PresentationAllowed", i);
            check_int(id, d.v2PresentationAllowed, 1);
        }
    }

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    for (i = 0; i < (size_t)n; ++i) {
        char id[96];
        d = theron_v2_phase_gate_decide(&cfg, v2Eligible[i]);
        snprintf(id, sizeof(id), "v2_eligible.v2_on_persist_on[%zu].v2PresentationAllowed", i);
        check_int(id, d.v2PresentationAllowed, 1);
    }
}

static void check_filter_config_gate(void)
{
    THERON_V2_PhaseGateConfig cfg;
    THERON_V2_PhaseGateDecision d;

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 0;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_FILTER_CONFIG);
    check_int("filter.v2_on_persist_off.v1SourceLocked", d.v1SourceLocked, 0);
    check_int("filter.v2_on_persist_off.v2PresentationAllowed", d.v2PresentationAllowed, 0);

    cfg.v2PresentationEnabled = 0;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_FILTER_CONFIG);
    check_int("filter.v2_off_persist_on.v2PresentationAllowed", d.v2PresentationAllowed, 0);

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = theron_v2_phase_gate_decide(&cfg, THERON_V2_PHASE_DOMAIN_FILTER_CONFIG);
    check_int("filter.v2_on_persist_on.v2PresentationAllowed", d.v2PresentationAllowed, 1);
}

static void check_v2_active_helper(void)
{
    THERON_V2_PhaseGateConfig cfg;

    check_int("v2_active.null", theron_v2_phase_gate_v2_active(0), 0);
    theron_v2_phase_gate_defaults(&cfg);
    check_int("v2_active.default", theron_v2_phase_gate_v2_active(&cfg), 0);
    cfg.v2PresentationEnabled = 1;
    check_int("v2_active.v2_on", theron_v2_phase_gate_v2_active(&cfg), 1);
    cfg.v2PresentationEnabled = 0;
    check_int("v2_active.v2_off", theron_v2_phase_gate_v2_active(&cfg), 0);
    cfg.v2PresentationEnabled = 0;
    cfg.v2ConfigPersistenceEnabled = 1;
    check_int("v2_active.persist_only", theron_v2_phase_gate_v2_active(&cfg), 0);
}

static void check_domain_names(void)
{
    int i;
    for (i = 0; i < (int)THERON_V2_PHASE_DOMAIN_COUNT; ++i) {
        char id[96];
        const char *name = theron_v2_phase_gate_domain_name((THERON_V2_PhaseDomain)i);
        snprintf(id, sizeof(id), "domain_name[%d].nonnull", i);
        check_true(id, name != 0);
        snprintf(id, sizeof(id), "domain_name[%d].nonempty", i);
        check_true(id, name && name[0] != 0);
    }
    check_true("domain_name.unknown",
               strcmp(theron_v2_phase_gate_domain_name((THERON_V2_PhaseDomain)9999),
                      "UNKNOWN") == 0);
}

static void check_source_evidence(void)
{
    const char *e = theron_v2_phase_gate_source_evidence();

    check_true("evidence.present", e != 0);
    check_true("evidence.nonempty", e && e[0] != 0);
    check_true("evidence.theron_v1_track02", strstr(e, "theron_v1_track02") != 0);
    check_true("evidence.theron_v1_boot", strstr(e, "theron_v1_boot") != 0);
    check_true("evidence.theron_v1_champions", strstr(e, "theron_v1_champions") != 0);
    check_true("evidence.theron_v1_dungeon_progression", strstr(e, "theron_v1_dungeon_progression") != 0);
    check_true("evidence.theron_v1_mechanics", strstr(e, "theron_v1_mechanics") != 0);
    check_true("evidence.theron_v1_save_load", strstr(e, "theron_v1_save_load") != 0);
    check_true("evidence.theron_v1_shop", strstr(e, "theron_v1_shop") != 0);
    check_true("evidence.theron_v1_tile_renderer", strstr(e, "theron_v1_tile_renderer") != 0);
    check_true("evidence.theron_v1_viewport", strstr(e, "theron_v1_viewport") != 0);
    check_true("evidence.theron_v1_world", strstr(e, "theron_v1_world") != 0);
    check_true("evidence.theron_v1_palette", strstr(e, "theron_v1_palette") != 0);
    check_true("evidence.theron_v1_ui_chrome", strstr(e, "theron_v1_ui_chrome") != 0);
    check_true("evidence.THQUEST_T080", strstr(e, "T080") != 0);
    check_true("evidence.THQUEST_T400", strstr(e, "T400") != 0);
    check_true("evidence.THQUEST_T520", strstr(e, "T520") != 0);
    check_true("evidence.THQUEST_T560", strstr(e, "T560") != 0);
    check_true("evidence.THQUEST_T600", strstr(e, "T600") != 0);
    check_true("evidence.THQUEST_T700", strstr(e, "T700") != 0);
    check_true("evidence.THQUEST_T800", strstr(e, "T800") != 0);
    check_true("evidence.THQUEST_T900", strstr(e, "T900") != 0);
    check_true("evidence.HuC6260", strstr(e, "HuC6260") != 0);
    check_true("evidence.HuC6280", strstr(e, "HuC6280") != 0);
    check_true("evidence.ADPCM", strstr(e, "ADPCM") != 0);
    check_true("evidence.doc_phase0", strstr(e, "tqr_v1_phase0_provenance_gate_H2339") != 0);
    check_true("evidence.doc_phase1", strstr(e, "tqr_v1_phase1_boot_H2338") != 0);
    check_true("evidence.doc_phase2", strstr(e, "tqr_v1_phase2_data_formats_H2339") != 0);
}

static void check_decision_metadata(void)
{
    THERON_V2_PhaseGateConfig cfg;
    THERON_V2_PhaseGateDecision d;
    size_t i;

    theron_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < (size_t)THERON_V2_PHASE_DOMAIN_COUNT; ++i) {
        char id[96];
        d = theron_v2_phase_gate_decide(&cfg, (THERON_V2_PhaseDomain)i);
        snprintf(id, sizeof(id), "decision[%zu].source_anchor", i);
        check_true(id, d.sourceAnchor != 0 && d.sourceAnchor[0] != 0);
        snprintf(id, sizeof(id), "decision[%zu].rule", i);
        check_true(id, d.rule != 0 && d.rule[0] != 0);
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
    check_int("unknown.v1SourceLocked", d.v1SourceLocked, 1);
    check_int("unknown.v2PresentationAllowed", d.v2PresentationAllowed, 0);
    check_int("unknown.is_gameplay",
              theron_v2_phase_gate_is_gameplay_domain((THERON_V2_PhaseDomain)999), 1);
}

static void check_track02_hashes(void)
{
    /* Theron V1 Track 02 asset hashes (PC Engine CD).
     * "397039af02d50d15c70b74088eb8a1cb" is 32 hex chars, so the
     * last char is at index 31 and the null-terminator is at index 32. */
    const char *jp = THERON_TRACK02_MD5_JP_REV1_ISO;
    const char *us = THERON_TRACK02_MD5_US_ISO;
    check_true("track02.jp_rev1_length",    strlen(jp) == 32);
    check_true("track02.jp_rev1_first",     jp[0] == '3');
    check_true("track02.jp_rev1_last",      jp[31] == 'b');
    check_true("track02.jp_rev1_nulterm",   jp[32] == 0);
    check_true("track02.us_iso_length",     strlen(us) == 32);
    check_true("track02.us_iso_first",      us[0] == '3');
    check_true("track02.us_iso_last",       us[31] == 'a');
    check_true("track02.us_iso_nulterm",    us[32] == 0);
    check_true("track02.hashes_distinct",
               strcmp(jp, us) != 0);
}

int main(void)
{
    printf("=== Theron V2 Phase 0/1 phase-gate unit test ===\n");
    check_default_config();
    check_null_args();
    check_v1_locked_domains();
    check_v2_eligible_domains();
    check_filter_config_gate();
    check_v2_active_helper();
    check_domain_names();
    check_source_evidence();
    check_decision_metadata();
    check_unknown_domain();
    check_track02_hashes();
    printf("--- %d / %d passed ---\n", g_passed, g_assertions);
    if (g_passed != g_assertions) {
        return 1;
    }
    return 0;
}
