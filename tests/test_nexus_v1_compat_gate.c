/*
 * test_nexus_v1_compat_gate.c — Nexus V1/V2 Phase Gate Tests
 *
 * Phase 0: V1 Compatibility Lock
 *
 * Tests compile-time flag and runtime gate behaviour for all 15 phase
 * domains. Validates:
 *   1. Compile-time NEXUS_V1_PHASE_DOMAIN_LOCK flag is set to 1
 *   2. nexus_v1_phase_gate_defaults() sets v2PresentationEnabled=0
 *   3. All 8 V1-source-locked domains always return v1SourceLocked=1
 *   4. V2-presentation-eligible domains return correct v2PresentationAllowed
 *      (0 when v2PresentationEnabled=0, 1 when v2PresentationEnabled=1)
 *   5. nexus_v1_phase_gate_v2_active() returns 0 by default
 *   6. nexus_v1_phase_gate_domain_name() returns valid names
 *   7. nexus_v1_phase_gate_source_evidence() returns non-empty string
 *   8. nexus_v1_phase_gate_compile_lock() returns 1
 *
 * Source: src/nexus/nexus_v1_compat_gate.c; include/nexus/nexus_v1_compat_gate.h
 *
 * Compile standalone:
 *   gcc -Wall -Wextra -O2 -I include \
 *       tests/test_nexus_v1_compat_gate.c \
 *       src/nexus/nexus_v1_compat_gate.c -o test_nexus_v1_compat_gate
 *
 * Run: ./test_nexus_v1_compat_gate
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nexus/nexus_v1_compat_gate.h"

/* ── Test helpers ─────────────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

static void check_int(int got, int expected, const char *name, const char *desc)
{
    if (got == expected) {
        g_pass++;
        printf("  PASS: %s (%s)\n", name, desc);
    } else {
        g_fail++;
        printf("  FAIL: %s (%s) — got %d, expected %d\n", name, desc, got, expected);
    }
}

static void check_nonzero(int val, const char *name, const char *desc)
{
    if (val != 0) {
        g_pass++;
        printf("  PASS: %s (%s)\n", name, desc);
    } else {
        g_fail++;
        printf("  FAIL: %s (%s) — got 0, expected non-zero\n", name, desc);
    }
}

/* ── Test 1: Compile-time gate flag ───────────────────────────────── */

static void test_compile_lock_flag(void)
{
    printf("\n[Test 1] Compile-time NEXUS_V1_PHASE_DOMAIN_LOCK flag\n");

    check_int(NEXUS_V1_PHASE_DOMAIN_LOCK, 1,
              "NEXUS_V1_PHASE_DOMAIN_LOCK == 1", "compile-time flag is 1");

    check_int(nexus_v1_phase_gate_compile_lock(), 1,
              "compile_lock()", "runtime echo returns 1");

    check_int(nexus_v1_phase_gate_compile_lock(), NEXUS_V1_PHASE_DOMAIN_LOCK,
              "runtime == macro", "runtime echo matches compile-time value");
}

/* ── Test 2: Defaults ────────────────────────────────────────────── */

static void test_defaults(void)
{
    printf("\n[Test 2] nexus_v1_phase_gate_defaults()\n");

    Nexus_V1_PhaseGateConfig cfg;
    cfg.v2PresentationEnabled = 99;
    nexus_v1_phase_gate_defaults(&cfg);

    check_int(cfg.v2PresentationEnabled, 0,
              "v2PresentationEnabled reset", "defaults to 0");

    check_int((int)nexus_v1_phase_gate_v2_active(&cfg), 0,
              "v2_active()", "returns 0 with defaults");

    /* NULL is safe */
    nexus_v1_phase_gate_defaults(NULL);
    printf("  PASS: NULL config is safe (no crash)\n");
    g_pass++;
}

/* ── Test 3: V1-source-locked domains ─────────────────────────────── */

static void test_v1_locked_domains(void)
{
    printf("\n[Test 3] V1-source-locked domains (always v1SourceLocked=1)\n");

    Nexus_V2_PhaseDomain v1_domains[] = {
        NEXUS_V2_PHASE_DOMAIN_DUNGEON_LOADING,
        NEXUS_V2_PHASE_DOMAIN_DUNGEON_RASTERIZER,
        NEXUS_V2_PHASE_DOMAIN_CHAMPION_POOL,
        NEXUS_V2_PHASE_DOMAIN_COMBAT,
        NEXUS_V2_PHASE_DOMAIN_MOVEMENT,
        NEXUS_V2_PHASE_DOMAIN_MAGIC,
        NEXUS_V2_PHASE_DOMAIN_SAVE_LOAD,
        NEXUS_V2_PHASE_DOMAIN_AUDIO,
    };
    const char *names[] = {
        "DUNGEON_LOADING", "DUNGEON_RASTERIZER", "CHAMPION_POOL",
        "COMBAT", "MOVEMENT", "MAGIC", "SAVE_LOAD", "AUDIO",
    };
    int n = (int)(sizeof(v1_domains) / sizeof(v1_domains[0]));

    for (int i = 0; i < n; i++) {
        Nexus_V2_PhaseDomain d = v1_domains[i];

        check_int(nexus_v1_phase_gate_is_v1_locked(d), 1,
                  names[i], "is_v1_locked() == 1");

        /* Even with V2 enabled, V1 domains stay locked */
        Nexus_V1_PhaseGateConfig cfg;
        nexus_v1_phase_gate_defaults(&cfg);
        cfg.v2PresentationEnabled = 1;

        Nexus_V1_PhaseGateDecision dec =
            nexus_v1_phase_gate_decide(&cfg, d);

        if (dec.v1SourceLocked != 1) { g_fail++; }
        else { g_pass++; }
        printf("     %s: V1-locked even with V2 enabled: %s\n",
               dec.v1SourceLocked == 1 ? "PASS" : "FAIL", names[i]);

        check_int(dec.v2PresentationAllowed, 0,
                  names[i], "v2PresentationAllowed == 0");

        check_nonzero((int)(dec.sourceAnchor != NULL && strlen(dec.sourceAnchor) > 0),
                      names[i], "sourceAnchor non-empty");

        check_nonzero((int)(dec.rule != NULL && strlen(dec.rule) > 0),
                      names[i], "rule non-empty");
    }
}

/* ── Test 4: V2-presentation-eligible domains ──────────────────────── */

static void test_v2_domains(void)
{
    printf("\n[Test 4] V2-presentation-eligible domains\n");

    Nexus_V2_PhaseDomain v2_domains[] = {
        NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION,
        NEXUS_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION,
        NEXUS_V2_PHASE_DOMAIN_LIGHTING_PRESENTATION,
        NEXUS_V2_PHASE_DOMAIN_HUD_PRESENTATION,
        NEXUS_V2_PHASE_DOMAIN_ATMOSPHERE_PRESENTATION,
        NEXUS_V2_PHASE_DOMAIN_PARTICLE_EFFECTS,
        NEXUS_V2_PHASE_DOMAIN_TOUCH_PRESENTATION,
    };
    const char *names[] = {
        "RENDER_PRESENTATION", "SMOOTH_MOVEMENT",
        "LIGHTING", "HUD", "ATMOSPHERE", "PARTICLES", "TOUCH",
    };
    int n = (int)(sizeof(v2_domains) / sizeof(v2_domains[0]));

    for (int i = 0; i < n; i++) {
        Nexus_V2_PhaseDomain d = v2_domains[i];

        check_int(nexus_v1_phase_gate_is_v1_locked(d), 0,
                  names[i], "is_v1_locked() == 0");

        /* V2 disabled → v2PresentationAllowed=0 */
        {
            Nexus_V1_PhaseGateConfig cfg;
            nexus_v1_phase_gate_defaults(&cfg);
            Nexus_V1_PhaseGateDecision dec =
                nexus_v1_phase_gate_decide(&cfg, d);

            check_int(dec.v1SourceLocked, 0,
                      names[i], "v1SourceLocked=0 when V2 disabled");

            check_int(dec.v2PresentationAllowed, 0,
                      names[i], "v2PresentationAllowed=0 when V2 disabled");
        }

        /* V2 enabled → v2PresentationAllowed=1 */
        {
            Nexus_V1_PhaseGateConfig cfg;
            nexus_v1_phase_gate_defaults(&cfg);
            cfg.v2PresentationEnabled = 1;
            Nexus_V1_PhaseGateDecision dec =
                nexus_v1_phase_gate_decide(&cfg, d);

            check_int(dec.v1SourceLocked, 0,
                      names[i], "v1SourceLocked=0 when V2 enabled");

            check_int(dec.v2PresentationAllowed, 1,
                      names[i], "v2PresentationAllowed=1 when V2 enabled");
        }
    }
}

/* ── Test 5: Source evidence and domain names ──────────────────────── */

static void test_evidence_and_names(void)
{
    printf("\n[Test 5] Source evidence and domain names\n");

    const char *ev = nexus_v1_phase_gate_source_evidence();
    check_nonzero((int)(ev != NULL && strlen(ev) > 10),
                  "source_evidence()", "returns non-empty string");
    printf("  Evidence: %s\n", ev);

    /* All 15 named domains return non-NULL non-empty names */
    for (int d = 0; d < NEXUS_V2_PHASE_DOMAIN_COUNT; d++) {
        const char *name = nexus_v1_phase_gate_domain_name((Nexus_V2_PhaseDomain)d);
        check_nonzero((int)(name != NULL && strlen(name) > 0),
                      "domain_name()", "non-empty for all 15 domains");
    }

    /* Unknown domain returns "UNKNOWN" */
    check_int(
        (int)strcmp(nexus_v1_phase_gate_domain_name((Nexus_V2_PhaseDomain)999), "UNKNOWN"),
        0, "unknown domain", "returns 'UNKNOWN'");
}

/* ── Test 6: Phase domain count ────────────────────────────────────── */

static void test_domain_count(void)
{
    printf("\n[Test 6] Phase domain enumeration\n");

    check_int(NEXUS_V2_PHASE_DOMAIN_COUNT, 15,
              "DOMAIN_COUNT", "NEXUS_V2_PHASE_DOMAIN_COUNT == 15");

    /* All domains have distinct numeric values */
    int seen[16] = {0};
    int all_unique = 1;
    for (int d = 0; d < NEXUS_V2_PHASE_DOMAIN_COUNT; d++) {
        if (seen[d]) all_unique = 0;
        seen[d] = 1;
    }
    check_int(all_unique, 1, "all_unique", "each domain has a distinct value");
}

/* ── Test 7: NULL config safe ──────────────────────────────────────── */

static void test_null_safe(void)
{
    printf("\n[Test 7] NULL config safety\n");

    /* V1 domain with NULL → V1-locked, V2-allowed=0 */
    Nexus_V1_PhaseGateDecision dec =
        nexus_v1_phase_gate_decide(NULL, NEXUS_V2_PHASE_DOMAIN_DUNGEON_LOADING);
    check_int(dec.v1SourceLocked, 1, "NULL:DUNGEON_LOADING", "V1-locked (fail-safe)");
    check_int(dec.v2PresentationAllowed, 0, "NULL:DUNGEON_LOADING", "v2Allowed=0 (fail-safe)");

    /* V2 domain with NULL → v1SourceLocked=0 (known domain, v2Active=0) */
    dec = nexus_v1_phase_gate_decide(NULL, NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    check_int(dec.v1SourceLocked, 0, "NULL:RENDER", "v1SourceLocked=0 for known V2 domain");
    check_int(dec.v2PresentationAllowed, 0, "NULL:RENDER", "v2Allowed=0 (v2Active=0 with NULL)");

    /* v2_active with NULL → 0 */
    check_int(nexus_v1_phase_gate_v2_active(NULL), 0, "v2_active(NULL)", "returns 0 (fail-safe)");
}

/* ── Test 8: Full round-trip — V2 enabled ──────────────────────────── */

static void test_roundtrip_v2_enabled(void)
{
    printf("\n[Test 8] Round-trip with V2 presentation enabled\n");

    Nexus_V1_PhaseGateConfig cfg;
    nexus_v1_phase_gate_defaults(&cfg);
    cfg.v2PresentationEnabled = 1;

    check_int(nexus_v1_phase_gate_v2_active(&cfg), 1,
              "v2_active()", "returns 1 when enabled");

    int v1_ok = 1;
    int v2_ok = 1;

    for (int d = 0; d < NEXUS_V2_PHASE_DOMAIN_COUNT; d++) {
        Nexus_V1_PhaseGateDecision dec =
            nexus_v1_phase_gate_decide(&cfg, (Nexus_V2_PhaseDomain)d);
        if (nexus_v1_phase_gate_is_v1_locked((Nexus_V2_PhaseDomain)d)) {
            if (dec.v1SourceLocked != 1) v1_ok = 0;
        } else {
            if (dec.v2PresentationAllowed != 1) v2_ok = 0;
        }
    }

    check_int(v1_ok, 1, "v1_locked domains", "all V1 domains stay locked with V2 on");
    check_int(v2_ok, 1, "v2 domains", "all V2 domains allow presentation with V2 on");
}

/* ── Main ─────────────────────────────────────────────────────────── */

int main(void)
{
    printf("======================================================\n");
    printf(" Nexus V1/V2 Phase Gate — Phase 0 Test Suite\n");
    printf(" NEXUS_V1_PHASE_DOMAIN_LOCK = %d\n", NEXUS_V1_PHASE_DOMAIN_LOCK);
    printf("======================================================\n");

    test_compile_lock_flag();
    test_defaults();
    test_v1_locked_domains();
    test_v2_domains();
    test_evidence_and_names();
    test_domain_count();
    test_null_safe();
    test_roundtrip_v2_enabled();

    printf("\n======================================================\n");
    printf(" Results: %d passed, %d failed\n", g_pass, g_fail);
    printf("======================================================\n");

    return g_fail > 0 ? 1 : 0;
}