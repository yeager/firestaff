/*
 * test_nexus_v2_phase_gate_pc34.c
 *
 * Unit test for Nexus V2 Phase 0 V1 compatibility lock and
 * Phase 1 launch/profile separation. Validates:
 *
 *  - nexus_v2_phase_gate_defaults() initialises to V1-only behaviour
 *  - Domain classification: all 19 Phase 0/1 domains are correctly
 *    classified as V1-source-locked (gameplay) or V2-eligible (presentation)
 *  - V1-source-locked domains always return v1SourceLocked=1
 *  - V2-eligible domains return v2PresentationAllowed=0 when V2 is off
 *  - V2-eligible domains return v2PresentationAllowed=1 when V2 is on
 *  - CONFIG_PRESENTATION additionally requires v2ConfigPersistenceEnabled
 *  - nexus_v2_phase_gate_v2_active() returns 1 only when v2PresentationEnabled=1
 *  - All 20 domain names are non-NULL
 *  - Source evidence references NEXUS.C / NEXUS2.C / nexus_v1_*
 *  - Unknown domains default to V1-locked
 *  - Default config has both toggles off
 *  - Phase domain count is 20 (NEXUS_V2_PHASE_DOMAIN_COUNT)
 *  - Source evidence is non-empty
 *
 * Source-lock:
 *  - include/nexus_v2_phase_gate_pc34.h
 *  - src/nexus/nexus_v2_phase_gate_pc34.c
 */

#include "nexus_v2_phase_gate_pc34.h"

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
    NEXUS_V2_PhaseGateConfig cfg;
    memset(&cfg, 0xff, sizeof(cfg));  /* poison so we can see defaults win */
    nexus_v2_phase_gate_defaults(&cfg);
    check_int("default.v2PresentationEnabled", cfg.v2PresentationEnabled, 0);
    check_int("default.v2ConfigPersistenceEnabled", cfg.v2ConfigPersistenceEnabled, 0);
    check_int("default.v2_active", nexus_v2_phase_gate_v2_active(&cfg), 0);
}

static void check_null_args(void)
{
    /* Defaults on NULL must be a no-op, no crash. */
    nexus_v2_phase_gate_defaults(0);
    check_int("null.v2_active", nexus_v2_phase_gate_v2_active(0), 0);
    /* Decide on NULL config returns v1SourceLocked=1, v2PresentationAllowed=0
     * for any domain (because the V2-active check sees NULL). */
    NEXUS_V2_PhaseGateDecision d =
        nexus_v2_phase_gate_decide(0, NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    check_int("null.decision.v1SourceLocked", d.v1SourceLocked, 0);
    check_int("null.decision.v2PresentationAllowed", d.v2PresentationAllowed, 0);
}

static void check_v1_locked_domains(void)
{
    /* All 11 V1-source-locked domains must always be v1SourceLocked=1
     * and v2PresentationAllowed=0, regardless of the V2 toggle. */
    static const NEXUS_V2_PhaseDomain v1Locked[] = {
        NEXUS_V2_PHASE_DOMAIN_DMDF_DGN_LOADING,
        NEXUS_V2_PHASE_DOMAIN_SATURN_ISO_READER,
        NEXUS_V2_PHASE_DOMAIN_GAME_STATE_INIT,
        NEXUS_V2_PHASE_DOMAIN_CHAMPION_PARTY,
        NEXUS_V2_PHASE_DOMAIN_CREATURE_AI,
        NEXUS_V2_PHASE_DOMAIN_SPELL_MAGIC,
        NEXUS_V2_PHASE_DOMAIN_MOVEMENT,
        NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD,
        NEXUS_V2_PHASE_DOMAIN_SOUND_DRIVER,
        NEXUS_V2_PHASE_DOMAIN_RASTERIZER,
        NEXUS_V2_PHASE_DOMAIN_INVENTORY
    };
    NEXUS_V2_PhaseGateConfig cfg;
    NEXUS_V2_PhaseGateDecision d;
    size_t i;
    int n = (int)(sizeof(v1Locked) / sizeof(v1Locked[0]));

    check_int("v1_locked.count", n, 11);

    /* V2 off. */
    nexus_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < (size_t)n; ++i) {
        char id[96];
        d = nexus_v2_phase_gate_decide(&cfg, v1Locked[i]);
        snprintf(id, sizeof(id), "v1_locked.v2_off[%zu].v1SourceLocked", i);
        check_int(id, d.v1SourceLocked, 1);
        snprintf(id, sizeof(id), "v1_locked.v2_off[%zu].v2PresentationAllowed", i);
        check_int(id, d.v2PresentationAllowed, 0);
        snprintf(id, sizeof(id), "v1_locked.v2_off[%zu].is_gameplay", i);
        check_int(id, nexus_v2_phase_gate_is_gameplay_domain(v1Locked[i]), 1);
        snprintf(id, sizeof(id), "v1_locked.v2_off[%zu].name_nonnull", i);
        check_true(id, nexus_v2_phase_gate_domain_name(v1Locked[i]) != 0);
    }

    /* V2 on - V1-locked domains must still be locked. */
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    for (i = 0; i < (size_t)n; ++i) {
        char id[96];
        d = nexus_v2_phase_gate_decide(&cfg, v1Locked[i]);
        snprintf(id, sizeof(id), "v1_locked.v2_on[%zu].v1SourceLocked", i);
        check_int(id, d.v1SourceLocked, 1);
        snprintf(id, sizeof(id), "v1_locked.v2_on[%zu].v2PresentationAllowed", i);
        check_int(id, d.v2PresentationAllowed, 0);
    }
}

static void check_v2_eligible_domains(void)
{
    /* All 9 V2-eligible domains must be v1SourceLocked=0. */
    static const NEXUS_V2_PhaseDomain v2Eligible[] = {
        NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION,
        NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION,
        NEXUS_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION,
        NEXUS_V2_PHASE_DOMAIN_HUD_OVERLAY,
        NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS,
        NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE,
        NEXUS_V2_PHASE_DOMAIN_INPUT_PRESENTATION,
        NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION,
        NEXUS_V2_PHASE_DOMAIN_UPSCALER
    };
    NEXUS_V2_PhaseGateConfig cfg;
    NEXUS_V2_PhaseGateDecision d;
    size_t i;
    int n = (int)(sizeof(v2Eligible) / sizeof(v2Eligible[0]));

    check_int("v2_eligible.count", n, 9);

    /* V2 off -> v2PresentationAllowed=0 for all V2-eligible domains. */
    nexus_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < (size_t)n; ++i) {
        char id[96];
        d = nexus_v2_phase_gate_decide(&cfg, v2Eligible[i]);
        snprintf(id, sizeof(id), "v2_eligible.v2_off[%zu].v1SourceLocked", i);
        check_int(id, d.v1SourceLocked, 0);
        if (v2Eligible[i] == NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION) {
            /* CONFIG requires BOTH v2 AND config-persistence. Both off. */
            snprintf(id, sizeof(id), "v2_eligible.v2_off[%zu].v2PresentationAllowed", i);
            check_int(id, d.v2PresentationAllowed, 0);
        } else {
            snprintf(id, sizeof(id), "v2_eligible.v2_off[%zu].v2PresentationAllowed", i);
            check_int(id, d.v2PresentationAllowed, 0);
        }
        snprintf(id, sizeof(id), "v2_eligible.v2_off[%zu].is_not_gameplay", i);
        check_int(id, nexus_v2_phase_gate_is_gameplay_domain(v2Eligible[i]), 0);
    }

    /* V2 on (config-persistence off) -> non-CONFIG domains allow V2. */
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 0;
    for (i = 0; i < (size_t)n; ++i) {
        char id[96];
        d = nexus_v2_phase_gate_decide(&cfg, v2Eligible[i]);
        snprintf(id, sizeof(id), "v2_eligible.v2_on_persist_off[%zu].v1SourceLocked", i);
        check_int(id, d.v1SourceLocked, 0);
        if (v2Eligible[i] == NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION) {
            snprintf(id, sizeof(id), "v2_eligible.v2_on_persist_off[%zu].config_blocked", i);
            check_int(id, d.v2PresentationAllowed, 0);
        } else {
            snprintf(id, sizeof(id), "v2_eligible.v2_on_persist_off[%zu].v2PresentationAllowed", i);
            check_int(id, d.v2PresentationAllowed, 1);
        }
    }

    /* V2 on + config-persistence on -> all V2-eligible domains allow V2. */
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    for (i = 0; i < (size_t)n; ++i) {
        char id[96];
        d = nexus_v2_phase_gate_decide(&cfg, v2Eligible[i]);
        snprintf(id, sizeof(id), "v2_eligible.v2_on_persist_on[%zu].v2PresentationAllowed", i);
        check_int(id, d.v2PresentationAllowed, 1);
    }
}

static void check_config_persistence_gate(void)
{
    NEXUS_V2_PhaseGateConfig cfg;
    NEXUS_V2_PhaseGateDecision d;

    /* V2 on but config-persistence off -> CONFIG blocked. */
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 0;
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    check_int("config.v2_on_persist_off.v1SourceLocked", d.v1SourceLocked, 0);
    check_int("config.v2_on_persist_off.v2PresentationAllowed", d.v2PresentationAllowed, 0);

    /* V2 off but config-persistence on -> CONFIG still blocked. */
    cfg.v2PresentationEnabled = 0;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    check_int("config.v2_off_persist_on.v2PresentationAllowed", d.v2PresentationAllowed, 0);

    /* Both on -> CONFIG allowed. */
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    check_int("config.v2_on_persist_on.v2PresentationAllowed", d.v2PresentationAllowed, 1);
}

static void check_v2_active_helper(void)
{
    NEXUS_V2_PhaseGateConfig cfg;

    /* NULL -> 0 */
    check_int("v2_active.null", nexus_v2_phase_gate_v2_active(0), 0);

    /* Defaults -> 0 */
    nexus_v2_phase_gate_defaults(&cfg);
    check_int("v2_active.default", nexus_v2_phase_gate_v2_active(&cfg), 0);

    cfg.v2PresentationEnabled = 1;
    check_int("v2_active.v2_on", nexus_v2_phase_gate_v2_active(&cfg), 1);

    cfg.v2PresentationEnabled = 0;
    check_int("v2_active.v2_off", nexus_v2_phase_gate_v2_active(&cfg), 0);

    /* v2ConfigPersistenceEnabled alone does NOT activate V2. */
    cfg.v2PresentationEnabled = 0;
    cfg.v2ConfigPersistenceEnabled = 1;
    check_int("v2_active.persist_only", nexus_v2_phase_gate_v2_active(&cfg), 0);
}

static void check_domain_names(void)
{
    /* All 20 enum values must produce a non-NULL name. */
    int i;
    for (i = 0; i < (int)NEXUS_V2_PHASE_DOMAIN_COUNT; ++i) {
        char id[96];
        const char *name = nexus_v2_phase_gate_domain_name((NEXUS_V2_PhaseDomain)i);
        snprintf(id, sizeof(id), "domain_name[%d].nonnull", i);
        check_true(id, name != 0);
        snprintf(id, sizeof(id), "domain_name[%d].nonempty", i);
        check_true(id, name[0] != 0);
    }
    /* Unknown domain returns "UNKNOWN". */
    check_true("domain_name.unknown",
               strcmp(nexus_v2_phase_gate_domain_name((NEXUS_V2_PhaseDomain)9999),
                      "UNKNOWN") == 0);
    /* Domain count is 20. */
    check_int("domain_count", (int)NEXUS_V2_PHASE_DOMAIN_COUNT, 20);
}

static void check_source_evidence(void)
{
    const char *e = nexus_v2_phase_gate_source_evidence();

    check_true("evidence.present", e != 0);
    check_true("evidence.nonempty", e && e[0] != 0);
    check_true("evidence.nexus_iso_reader", strstr(e, "nexus_v1_iso_reader") != 0);
    check_true("evidence.nexus_dmdf_model", strstr(e, "nexus_v1_dmdf_model") != 0);
    check_true("evidence.nexus_dungeon", strstr(e, "nexus_v1_dungeon") != 0);
    check_true("evidence.nexus_engine", strstr(e, "nexus_v1_engine") != 0);
    check_true("evidence.nexus_game", strstr(e, "nexus_v1_game") != 0);
    check_true("evidence.nexus_champions", strstr(e, "nexus_v1_champions") != 0);
    check_true("evidence.nexus_movement", strstr(e, "nexus_v1_movement") != 0);
    check_true("evidence.nexus_save_load", strstr(e, "nexus_v1_save_load") != 0);
    check_true("evidence.nexus_rasterizer", strstr(e, "nexus_v1_rasterizer") != 0);
    check_true("evidence.NEXUS.C", strstr(e, "NEXUS.C") != 0);
    check_true("evidence.NEXUS2.C", strstr(e, "NEXUS2.C") != 0);
    check_true("evidence.NEXUS.BIN", strstr(e, "NEXUS.BIN") != 0);
    check_true("evidence.F0365", strstr(e, "F0365") != 0);
    check_true("evidence.F0366", strstr(e, "F0366") != 0);
    check_true("evidence.F0380", strstr(e, "F0380") != 0);
    check_true("evidence.HuC6260", strstr(e, "HuC6260") != 0);
}

static void check_decision_metadata(void)
{
    /* Decision has sourceAnchor and rule, and the rule is non-empty. */
    NEXUS_V2_PhaseGateConfig cfg;
    NEXUS_V2_PhaseGateDecision d;
    size_t i;

    nexus_v2_phase_gate_defaults(&cfg);

    /* Iterate all 20 enum values; every decision must have non-null
     * sourceAnchor and rule. */
    for (i = 0; i < (size_t)NEXUS_V2_PHASE_DOMAIN_COUNT; ++i) {
        char id[96];
        d = nexus_v2_phase_gate_decide(&cfg, (NEXUS_V2_PhaseDomain)i);
        snprintf(id, sizeof(id), "decision[%zu].source_anchor", i);
        check_true(id, d.sourceAnchor != 0 && d.sourceAnchor[0] != 0);
        snprintf(id, sizeof(id), "decision[%zu].rule", i);
        check_true(id, d.rule != 0 && d.rule[0] != 0);
    }
}

static void check_unknown_domain(void)
{
    /* Unknown domain (>= NEXUS_V2_PHASE_DOMAIN_COUNT) defaults to
     * v1SourceLocked=1, v2PresentationAllowed=0. */
    NEXUS_V2_PhaseGateConfig cfg;
    NEXUS_V2_PhaseGateDecision d;

    nexus_v2_phase_gate_defaults(&cfg);
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;

    d = nexus_v2_phase_gate_decide(&cfg, (NEXUS_V2_PhaseDomain)999);
    check_int("unknown.v1SourceLocked", d.v1SourceLocked, 1);
    check_int("unknown.v2PresentationAllowed", d.v2PresentationAllowed, 0);
    check_int("unknown.is_gameplay", nexus_v2_phase_gate_is_gameplay_domain((NEXUS_V2_PhaseDomain)999), 1);
}

int main(void)
{
    printf("=== Nexus V2 Phase 0/1 phase-gate unit test ===\n");
    check_default_config();
    check_null_args();
    check_v1_locked_domains();
    check_v2_eligible_domains();
    check_config_persistence_gate();
    check_v2_active_helper();
    check_domain_names();
    check_source_evidence();
    check_decision_metadata();
    check_unknown_domain();
    printf("--- %d / %d passed ---\n", g_passed, g_assertions);
    if (g_passed != g_assertions) {
        return 1;
    }
    return 0;
}
