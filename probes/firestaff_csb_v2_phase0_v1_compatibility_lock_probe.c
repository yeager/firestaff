/*
 * firestaff_csb_v2_phase0_v1_compatibility_lock_probe.c
 *
 * CSB V2 Phase 0 — V1 Compatibility Lock Probe
 *
 * Headless probe: verifies the CSB V2 Phase 0 V1 compatibility lock
 * (csb_v2_phase_gate_pc34.c) without requiring live game asset files.
 *
 * This probe validates:
 *
 *   1. csb_v2_phase_gate_pc34_defaults() initialises to V1-only behaviour
 *      - v2PresentationEnabled = 0
 *      - v2ConfigPersistenceEnabled = 0
 *
 *   2. Domain classification: all 13 Phase 0 domains are correctly
 *      classified as V1-source-locked (gameplay) or V2-eligible (presentation)
 *
 *   3. V1-source-locked domains (7):
 *      COMMAND_SEMANTICS, DUNGEON_LOADING, DUNGEON_TIMING, COLLISION_RULES,
 *      SAVE_LOAD_DATA, CHAMPION_RESURRECT, CHAOS_MAGIC_SCRIPTS
 *      - Always return v1SourceLocked=1, v2PresentationAllowed=0
 *      - Independent of v2PresentationEnabled
 *
 *   4. V2-presentation-eligible domains (6):
 *      RENDER_PRESENTATION, SMOOTH_MOVEMENT_PRESENTATION,
 *      DYNAMIC_LIGHTING_PRESENTATION, MINIMAP_PRESENTATION,
 *      INPUT_PRESENTATION, CONFIG_PRESENTATION
 *      - Return v2PresentationAllowed=0 when v2PresentationEnabled=0
 *      - Return v2PresentationAllowed=1 when v2PresentationEnabled=1
 *      - v1SourceLocked=0 in both cases
 *
 *   5. CONFIG_PRESENTATION additionally requires v2ConfigPersistenceEnabled
 *      - v2PresentationAllowed=0 when v2PresentationEnabled=1 but
 *        v2ConfigPersistenceEnabled=0
 *      - v2PresentationAllowed=1 when both are 1
 *
 *   6. csb_v2_phase_gate_pc34_v2_active() returns 1 only when
 *      v2PresentationEnabled=1
 *
 *   7. csb_v2_phase_gate_pc34_domain_name() returns non-NULL strings for
 *      all 13 domains
 *
 *   8. csb_v2_phase_gate_pc34_source_evidence() returns a non-NULL,
 *      non-empty string referencing ReDMCSB/CSBWin sources
 *
 *   9. All domains are handled (no switch fallthrough to default)
 *
 *  10. Source citations in gate decisions contain ReDMCSB or CSBWin
 *      references for all domains
 *
 * Exit codes:
 *   0 — all checks passed
 *   1 — one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy ./firestaff_csb_v2_phase0_v1_compatibility_lock_probe
 *
 * Source references:
 *   ReDMCSB COMMAND.C:2045-2155 F0359 (command queue dispatch)
 *   ReDMCSB CLIKMENU.C:142 F0365 (turn party)
 *   ReDMCSB CLIKMENU.C:180 F0366 (move party)
 *   ReDMCSB CLIKMENU.C:278-323 (collision)
 *   ReDMCSB GAMELOOP.C:150-155, 215-219 (timing/input wait)
 *   ReDMCSB MOVESENS.C:316-345 F0267 (move-result)
 *   ReDMCSB LOADSAVE.C:1520-1534, 2730-2742 (save/load)
 *   ReDMCSB DUNGEON.C (dungeon format)
 *   ReDMCSB CHAMPION.C (champion resurrect)
 *   ReDMCSB PANEL.C:418-428 (canonical palette)
 *   CSBWin/Chaos.cpp:60-69 (DSA dispatch)
 *   CSBWin/DSA.cpp (DSA interpreter)
 *   CSBWin/champion.cpp (champion resurrect)
 *   CSBWin/Viewport.cpp (viewport 7290 lines)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csb_v2_phase_gate_pc34.h"

/* ── Helpers ──────────────────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

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

/* ── V1-source-locked domains (must always return v1SourceLocked=1) ── */

static const CSB_V2_PhaseDomain g_v1_locked_domains[] = {
    CSB_V2_PHASE_DOMAIN_COMMAND_SEMANTICS,
    CSB_V2_PHASE_DOMAIN_DUNGEON_LOADING,
    CSB_V2_PHASE_DOMAIN_DUNGEON_TIMING,
    CSB_V2_PHASE_DOMAIN_COLLISION_RULES,
    CSB_V2_PHASE_DOMAIN_SAVE_LOAD_DATA,
    CSB_V2_PHASE_DOMAIN_CHAMPION_RESURRECT,
    CSB_V2_PHASE_DOMAIN_CHAOS_MAGIC_SCRIPTS,
};

static const char *g_v1_locked_names[] = {
    "COMMAND_SEMANTICS",
    "DUNGEON_LOADING",
    "DUNGEON_TIMING",
    "COLLISION_RULES",
    "SAVE_LOAD_DATA",
    "CHAMPION_RESURRECT",
    "CHAOS_MAGIC_SCRIPTS",
};

/* ── V2-presentation-eligible domains ── */

static const CSB_V2_PhaseDomain g_v2_eligible_domains[] = {
    CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION,
    CSB_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION,
    CSB_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION,
    CSB_V2_PHASE_DOMAIN_MINIMAP_PRESENTATION,
    CSB_V2_PHASE_DOMAIN_INPUT_PRESENTATION,
    CSB_V2_PHASE_DOMAIN_CONFIG_PRESENTATION,
};

static const char *g_v2_eligible_names[] = {
    "RENDER_PRESENTATION",
    "SMOOTH_MOVEMENT_PRESENTATION",
    "DYNAMIC_LIGHTING_PRESENTATION",
    "MINIMAP_PRESENTATION",
    "INPUT_PRESENTATION",
    "CONFIG_PRESENTATION",
};

#define N_V1_LOCKED   (int)(sizeof(g_v1_locked_domains)/sizeof(g_v1_locked_domains[0]))
#define N_V2_ELIGIBLE (int)(sizeof(g_v2_eligible_domains)/sizeof(g_v2_eligible_domains[0]))
#define N_DOMAINS     (N_V1_LOCKED + N_V2_ELIGIBLE)

/* ── TC-1: defaults() ──────────────────────────────────────────────── */

static void tc_defaults(void)
{
    CSB_V2_PhaseGateConfig cfg;
    csb_v2_phase_gate_pc34_defaults(&cfg);
    record(cfg.v2PresentationEnabled == 0,
           "TC-1", "v2PresentationEnabled=0 after defaults");
    record(cfg.v2ConfigPersistenceEnabled == 0,
           "TC-1", "v2ConfigPersistenceEnabled=0 after defaults");
}

/* ── TC-2: v2_active() ───────────────────────────────────────────── */

static void tc_v2_active(void)
{
    CSB_V2_PhaseGateConfig cfg;
    csb_v2_phase_gate_pc34_defaults(&cfg);
    record(csb_v2_phase_gate_pc34_v2_active(&cfg) == 0,
           "TC-2", "v2_active()=0 when v2PresentationEnabled=0");

    cfg.v2PresentationEnabled = 1;
    record(csb_v2_phase_gate_pc34_v2_active(&cfg) == 1,
           "TC-2", "v2_active()=1 when v2PresentationEnabled=1");

    cfg.v2PresentationEnabled = 0;
    record(csb_v2_phase_gate_pc34_v2_active(NULL) == 0,
           "TC-2", "v2_active(NULL)=0 (null-safe)");
}

/* ── TC-3: domain classification ─────────────────────────────────── */

static void tc_domain_classification(void)
{
    int i;
    for (i = 0; i < N_V1_LOCKED; ++i) {
        char buf[64];
        CSB_V2_PhaseDomain d = g_v1_locked_domains[i];
        snprintf(buf, sizeof(buf), "TC-3 [%s]", g_v1_locked_names[i]);
        record(csb_v2_phase_gate_pc34_is_gameplay_domain(d) == 1,
               buf, "recognised as V1-source-locked gameplay domain");
    }
    for (i = 0; i < N_V2_ELIGIBLE; ++i) {
        char buf[64];
        CSB_V2_PhaseDomain d = g_v2_eligible_domains[i];
        snprintf(buf, sizeof(buf), "TC-3 [%s]", g_v2_eligible_names[i]);
        record(csb_v2_phase_gate_pc34_is_gameplay_domain(d) == 0,
               buf, "recognised as V2-presentation-eligible domain");
    }
}

/* ── TC-4: V1 domains — v1SourceLocked=1 always ──────────────────── */

static void tc_v1_locked_always(void)
{
    CSB_V2_PhaseGateConfig cfg_disabled, cfg_enabled;
    int i;

    csb_v2_phase_gate_pc34_defaults(&cfg_disabled);
    csb_v2_phase_gate_pc34_defaults(&cfg_enabled);
    cfg_enabled.v2PresentationEnabled = 1;

    for (i = 0; i < N_V1_LOCKED; ++i) {
        char buf[64];
        CSB_V2_PhaseDomain d = g_v1_locked_domains[i];
        CSB_V2_PhaseGateDecision dec_disabled, dec_enabled;

        snprintf(buf, sizeof(buf), "TC-4 [%s]", g_v1_locked_names[i]);

        dec_disabled = csb_v2_phase_gate_pc34_decide(&cfg_disabled, d);
        record(dec_disabled.v1SourceLocked == 1,
               buf, "v1SourceLocked=1 (V2 disabled)");
        record(dec_disabled.v2PresentationAllowed == 0,
               buf, "v2PresentationAllowed=0 (V2 disabled)");

        dec_enabled = csb_v2_phase_gate_pc34_decide(&cfg_enabled, d);
        record(dec_enabled.v1SourceLocked == 1,
               buf, "v1SourceLocked=1 (V2 enabled)");
        record(dec_enabled.v2PresentationAllowed == 0,
               buf, "v2PresentationAllowed=0 (V2 enabled)");
    }
}

/* ── TC-5: V2-eligible domains — v2PresentationAllowed follows toggle ─ */

static void tc_v2_eligible_toggle(void)
{
    CSB_V2_PhaseGateConfig cfg_disabled, cfg_enabled;
    int i;

    csb_v2_phase_gate_pc34_defaults(&cfg_disabled);
    csb_v2_phase_gate_pc34_defaults(&cfg_enabled);
    cfg_enabled.v2PresentationEnabled = 1;

    for (i = 0; i < N_V2_ELIGIBLE; ++i) {
        char buf[64];
        CSB_V2_PhaseDomain d = g_v2_eligible_domains[i];
        CSB_V2_PhaseGateDecision dec;

        snprintf(buf, sizeof(buf), "TC-5 [%s]", g_v2_eligible_names[i]);

        /* Skip CONFIG_PRESENTATION — it has a second toggle */
        if (d == CSB_V2_PHASE_DOMAIN_CONFIG_PRESENTATION)
            continue;

        dec = csb_v2_phase_gate_pc34_decide(&cfg_disabled, d);
        record(dec.v1SourceLocked == 0,
               buf, "v1SourceLocked=0 (V2 disabled)");
        record(dec.v2PresentationAllowed == 0,
               buf, "v2PresentationAllowed=0 (V2 disabled)");

        dec = csb_v2_phase_gate_pc34_decide(&cfg_enabled, d);
        record(dec.v1SourceLocked == 0,
               buf, "v1SourceLocked=0 (V2 enabled)");
        record(dec.v2PresentationAllowed == 1,
               buf, "v2PresentationAllowed=1 (V2 enabled)");
    }
}

/* ── TC-6: CONFIG_PRESENTATION requires both toggles ─────────────── */

static void tc_config_presentation_both_toggles(void)
{
    CSB_V2_PhaseGateConfig cfg0, cfg1, cfg2, cfg3;
    CSB_V2_PhaseGateDecision dec;

    /* Both off */
    csb_v2_phase_gate_pc34_defaults(&cfg0);
    /* presentation on, persistence off */
    csb_v2_phase_gate_pc34_defaults(&cfg1);
    cfg1.v2PresentationEnabled = 1;
    /* persistence on, presentation off (should still be blocked) */
    csb_v2_phase_gate_pc34_defaults(&cfg2);
    cfg2.v2ConfigPersistenceEnabled = 1;
    /* both on */
    csb_v2_phase_gate_pc34_defaults(&cfg3);
    cfg3.v2PresentationEnabled = 1;
    cfg3.v2ConfigPersistenceEnabled = 1;

    dec = csb_v2_phase_gate_pc34_decide(&cfg0, CSB_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    record(dec.v2PresentationAllowed == 0,
           "TC-6", "CONFIG_PRES v2Allowed=0 (both off)");

    dec = csb_v2_phase_gate_pc34_decide(&cfg1, CSB_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    record(dec.v2PresentationAllowed == 0,
           "TC-6", "CONFIG_PRES v2Allowed=0 (persistence off)");

    dec = csb_v2_phase_gate_pc34_decide(&cfg2, CSB_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    record(dec.v2PresentationAllowed == 0,
           "TC-6", "CONFIG_PRES v2Allowed=0 (presentation off)");

    dec = csb_v2_phase_gate_pc34_decide(&cfg3, CSB_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    record(dec.v2PresentationAllowed == 1,
           "TC-6", "CONFIG_PRES v2Allowed=1 (both on)");
}

/* ── TC-7: domain_name() returns all 13 names ─────────────────────── */

static void tc_domain_names(void)
{
    CSB_V2_PhaseDomain d;
    int matched = 0;
    const char *name;

    for (d = 0; d < CSB_V2_PHASE_DOMAIN_COUNT; ++d) {
        name = csb_v2_phase_gate_pc34_domain_name(d);
        record(name != NULL && strlen(name) > 0,
               "TC-7", "domain_name() non-null for all domains");
        if (name != NULL && strlen(name) > 0)
            ++matched;
    }
    record(matched == CSB_V2_PHASE_DOMAIN_COUNT,
           "TC-7", "all domain names returned (no truncation)");
}

/* ── TC-8: source_evidence() is non-null ────────────────────────── */

static void tc_source_evidence(void)
{
    const char *ev = csb_v2_phase_gate_pc34_source_evidence();
    record(ev != NULL,
           "TC-8", "source_evidence() != NULL");
    record(ev != NULL && strlen(ev) > 8,
           "TC-8", "source_evidence() is non-empty");
    if (ev != NULL) {
        record(strstr(ev, "ReDMCSB") != NULL || strstr(ev, "CSBWin") != NULL,
               "TC-8", "source_evidence() references ReDMCSB or CSBWin");
    }
}

/* ── TC-9: all decisions have non-null anchor and rule strings ────── */

static void tc_decision_strings(void)
{
    CSB_V2_PhaseGateConfig cfg_enabled;
    CSB_V2_PhaseDomain d;

    csb_v2_phase_gate_pc34_defaults(&cfg_enabled);
    cfg_enabled.v2PresentationEnabled = 1;
    cfg_enabled.v2ConfigPersistenceEnabled = 1;

    for (d = 0; d < CSB_V2_PHASE_DOMAIN_COUNT; ++d) {
        char id[64];
        CSB_V2_PhaseGateDecision dec = csb_v2_phase_gate_pc34_decide(&cfg_enabled, d);
        snprintf(id, sizeof(id), "TC-9 [%s]",
                 csb_v2_phase_gate_pc34_domain_name(d));

        record(dec.sourceAnchor != NULL && strlen(dec.sourceAnchor) > 4,
               id, "decision has a sourceAnchor");
        record(dec.rule != NULL && strlen(dec.rule) > 4,
               id, "decision has a rule description");
    }
}

/* ── TC-10: V1 domain source anchors reference ReDMCSB/CSBWin ──────── */

static void tc_v1_domain_source_anchors(void)
{
    CSB_V2_PhaseGateConfig cfg_enabled;
    int i;

    csb_v2_phase_gate_pc34_defaults(&cfg_enabled);
    cfg_enabled.v2PresentationEnabled = 1;

    for (i = 0; i < N_V1_LOCKED; ++i) {
        char buf[64];
        CSB_V2_PhaseDomain d = g_v1_locked_domains[i];
        CSB_V2_PhaseGateDecision dec = csb_v2_phase_gate_pc34_decide(&cfg_enabled, d);

        snprintf(buf, sizeof(buf), "TC-10 [%s]", g_v1_locked_names[i]);
        record(dec.sourceAnchor != NULL && strlen(dec.sourceAnchor) > 4,
               buf, "sourceAnchor is non-empty");
        record(strstr(dec.sourceAnchor, "ReDMCSB") != NULL ||
               strstr(dec.sourceAnchor, "CSBWin") != NULL,
               buf, "sourceAnchor references ReDMCSB or CSBWin");
    }
}

/* ── TC-11: V2-eligible domain decisions reference ReDMCSB/CSBWin ─── */

static void tc_v2_domain_source_anchors(void)
{
    CSB_V2_PhaseGateConfig cfg_enabled;
    int i;

    csb_v2_phase_gate_pc34_defaults(&cfg_enabled);
    cfg_enabled.v2PresentationEnabled = 1;

    for (i = 0; i < N_V2_ELIGIBLE; ++i) {
        char buf[64];
        CSB_V2_PhaseDomain d = g_v2_eligible_domains[i];
        CSB_V2_PhaseGateDecision dec = csb_v2_phase_gate_pc34_decide(&cfg_enabled, d);

        snprintf(buf, sizeof(buf), "TC-11 [%s]", g_v2_eligible_names[i]);
        record(dec.sourceAnchor != NULL && strlen(dec.sourceAnchor) > 4,
               buf, "sourceAnchor is non-empty");
        record(strstr(dec.sourceAnchor, "ReDMCSB") != NULL ||
               strstr(dec.sourceAnchor, "CSBWin") != NULL,
               buf, "sourceAnchor references ReDMCSB or CSBWin");
    }
}

/* ── TC-12: NULL config is safe (null-pointer guard) ─────────────── */

static void tc_null_config_safe(void)
{
    CSB_V2_PhaseGateDecision dec;
    CSB_V2_PhaseDomain d;

    /* NULL config is a programming error; the gate should not crash.
     * With NULL config, v2Active becomes 0 (fail-safe).
     * v1SourceLocked reflects domain classification, not config state:
     *   V1-locked domains:  v1SourceLocked=1 regardless of config
     *   V2-eligible domains: v1SourceLocked=0 (they are NOT V1-locked,
     *     they just have v2PresentationAllowed=0 when v2 is inactive) */
    for (d = 0; d < CSB_V2_PHASE_DOMAIN_COUNT; ++d) {
        char buf[64];
        int expected_v1_locked = csb_v2_phase_gate_pc34_is_gameplay_domain(d);
        dec = csb_v2_phase_gate_pc34_decide(NULL, d);
        snprintf(buf, sizeof(buf), "TC-12 [%s]",
                 csb_v2_phase_gate_pc34_domain_name(d));
        /* v1SourceLocked must match domain classification */
        record(dec.v1SourceLocked == expected_v1_locked,
               buf, "v1SourceLocked matches domain classification with NULL config");
        /* v2PresentationAllowed must be 0 with NULL config (v2Active=0 fail-safe) */
        record(dec.v2PresentationAllowed == 0,
               buf, "v2PresentationAllowed=0 with NULL config (fail-safe)");
    }
}

/* ── TC-13: defaults() is null-safe ──────────────────────────────── */

static void tc_defaults_null_safe(void)
{
    /* Must not crash with NULL */
    csb_v2_phase_gate_pc34_defaults(NULL);
    record(1, "TC-13", "defaults(NULL) did not crash");
}

/* ── TC-14: count matches — all domains covered ──────────────────── */

static void tc_domain_count(void)
{
    record(N_DOMAINS == CSB_V2_PHASE_DOMAIN_COUNT,
           "TC-14", "enum count matches N_V1_LOCKED + N_V2_ELIGIBLE");
}

/* ── TC-15: default state is V1-locked ───────────────────────────── */

static void tc_default_is_v1_locked(void)
{
    CSB_V2_PhaseGateConfig cfg;
    CSB_V2_PhaseDomain d;

    csb_v2_phase_gate_pc34_defaults(&cfg);

    for (d = 0; d < CSB_V2_PHASE_DOMAIN_COUNT; ++d) {
        char buf[64];
        CSB_V2_PhaseGateDecision dec = csb_v2_phase_gate_pc34_decide(&cfg, d);
        snprintf(buf, sizeof(buf), "TC-15 [%s]",
                 csb_v2_phase_gate_pc34_domain_name(d));

        if (csb_v2_phase_gate_pc34_is_gameplay_domain(d)) {
            /* V1 gameplay domains must be locked */
            record(dec.v1SourceLocked == 1,
                   buf, "V1 gameplay domain locked by default");
        }
    }

    /* v2_active() must be 0 by default */
    record(csb_v2_phase_gate_pc34_v2_active(&cfg) == 0,
           "TC-15", "v2_active()=0 by default (no V2 presentation active)");
}

/* ── Main ─────────────────────────────────────────────────────────── */

int main(void)
{
    printf("CSB V2 Phase 0 — V1 Compatibility Lock Probe\n");
    printf("Headless: no game assets required\n");
    printf("SDL_VIDEODRIVER=%s\n\n", getenv("SDL_VIDEODRIVER") ?: "(null)");

    section("TC-1: defaults()");
    tc_defaults();

    section("TC-2: v2_active()");
    tc_v2_active();

    section("TC-3: domain classification");
    tc_domain_classification();

    section("TC-4: V1-source-locked domains — always locked");
    tc_v1_locked_always();

    section("TC-5: V2-eligible domains — toggle-responsive");
    tc_v2_eligible_toggle();

    section("TC-6: CONFIG_PRESENTATION — requires both toggles");
    tc_config_presentation_both_toggles();

    section("TC-7: domain_name() — all 13 domains");
    tc_domain_names();

    section("TC-8: source_evidence()");
    tc_source_evidence();

    section("TC-9: decision strings — anchor and rule");
    tc_decision_strings();

    section("TC-10: V1 domain source anchors");
    tc_v1_domain_source_anchors();

    section("TC-11: V2 domain source anchors");
    tc_v2_domain_source_anchors();

    section("TC-12: NULL config safety");
    tc_null_config_safe();

    section("TC-13: defaults(NULL) null-safety");
    tc_defaults_null_safe();

    section("TC-14: domain count");
    tc_domain_count();

    section("TC-15: default state is V1-locked");
    tc_default_is_v1_locked();

    printf("\n=== SUMMARY ===\n");
    printf("PASS: %d  FAIL: %d\n", g_pass, g_fail);
    printf("RESULT: %s\n", g_fail == 0 ? "PASS" : "FAIL");

    return g_fail == 0 ? 0 : 1;
}
