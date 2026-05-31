/*
 * firestaff_dm2_v2_phase3_hud_overlay_probe.c — DM2 V2 Phase 3 HUD Probe
 *
 * Phase 3: Enhanced UI overlays — HUD compass, depth, gold, champion bars,
 * and action strip.  Tests:
 *   1. DM2_V2_PHASE_DOMAIN_HUD gate exists and is correctly gated
 *   2. HUD domain requires LAUNCH+PROFILE to be enabled
 *   3. dm2_v2_hud_init/render/access/render cycle works correctly
 *   4. Phase gate integration with dm2_v2_runtime (HUD state, no crash)
 *
 * Headless: no game data, no SDL rendering required.
 * Compile:  gcc -Wall -Wextra -O2 -I../../include
 *           -o firestaff_dm2_v2_phase3_hud_overlay_probe
 *           firestaff_dm2_v2_phase3_hud_overlay_probe.c
 *
 * Source: SKULL.ASM T560 (DM2 HUD rendering)
 *         SKULLWIN/SKWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *         ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *         ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing)
 */

#include "dm2_v2_phase_gate.h"
#include "dm2_v2_hud_overlay.h"
#include "dm2_v2_runtime.h"
#include <stdio.h>
#include <string.h>

static int s_pass = 0;
static int s_fail = 0;

static void check(const char *name, int cond) {
    if (cond) {
        printf("  PASS: %s\n", name);
        s_pass++;
    } else {
        printf("  FAIL: %s\n", name);
        s_fail++;
    }
}

int main(void) {
    printf("=== DM2 V2 Phase 3 — HUD Overlay Probe ===\n\n");

    /* ── HUD domain enum exists ───────────────────────────────────── */
    printf("[ HUD domain enum ]\n");
    /* DM2_V2_PHASE_DOMAIN_HUD must be in the enum */
    check("DM2_V2_PHASE_DOMAIN_HUD != LAUNCH",
        DM2_V2_PHASE_DOMAIN_HUD != DM2_V2_PHASE_DOMAIN_LAUNCH);
    check("DM2_V2_PHASE_DOMAIN_HUD != PROFILE",
        DM2_V2_PHASE_DOMAIN_HUD != DM2_V2_PHASE_DOMAIN_PROFILE);
    check("DM2_V2_PHASE_DOMAIN_HUD == 2",
        DM2_V2_PHASE_DOMAIN_HUD == 2);

    /* ── Domain check API ─────────────────────────────────────────── */
    printf("\n[ Domain check API ]\n");
    check("is_hud_domain(LAUNCH) = false",
        !dm2_v2_phase_gate_is_hud_domain(DM2_V2_PHASE_DOMAIN_LAUNCH));
    check("is_hud_domain(PROFILE) = false",
        !dm2_v2_phase_gate_is_hud_domain(DM2_V2_PHASE_DOMAIN_PROFILE));
    check("is_hud_domain(HUD) = true",
        dm2_v2_phase_gate_is_hud_domain(DM2_V2_PHASE_DOMAIN_HUD));

    /* ── HUD gate: default (0,0) → V1 source-locked ───────────────── */
    printf("\n[ HUD gate: default config ]\n");
    DM2_V2_PhaseGateConfig cfg = {0, 0};

    DM2_V2_PhaseGateDecision d;

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_LAUNCH);
    check("LAUNCH default: v1SourceLocked=1", d.v1SourceLocked == 1);
    check("LAUNCH default: v2Allowed=0", d.v2Allowed == 0);

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_PROFILE);
    check("PROFILE default: v1SourceLocked=1", d.v1SourceLocked == 1);
    check("PROFILE default: v2Allowed=0", d.v2Allowed == 0);

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_HUD);
    check("HUD default: v1SourceLocked=1", d.v1SourceLocked == 1);
    check("HUD default: v2Allowed=0", d.v2Allowed == 0);

    /* ── HUD gate: LAUNCH-only (1,0) → PROFILE gated, HUD gated ──── */
    printf("\n[ HUD gate: LAUNCH-only config ]\n");
    cfg.v2LaunchEnabled = 1;
    cfg.v2ProfileEnabled = 0;

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_LAUNCH);
    check("LAUNCH(1,0): v1SourceLocked=0", d.v1SourceLocked == 0);
    check("LAUNCH(1,0): v2Allowed=1", d.v2Allowed == 1);

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_PROFILE);
    check("PROFILE(1,0): v1SourceLocked=1 (gated on LAUNCH)", d.v1SourceLocked == 1);
    check("PROFILE(1,0): v2Allowed=0", d.v2Allowed == 0);

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_HUD);
    check("HUD(1,0): v1SourceLocked=1 (needs PROFILE)", d.v1SourceLocked == 1);
    check("HUD(1,0): v2Allowed=0", d.v2Allowed == 0);

    /* ── HUD gate: LAUNCH+PROFILE (1,1) → HUD active ─────────────── */
    printf("\n[ HUD gate: LAUNCH+PROFILE config ]\n");
    cfg.v2LaunchEnabled = 1;
    cfg.v2ProfileEnabled = 1;

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_LAUNCH);
    check("LAUNCH(1,1): v1SourceLocked=0", d.v1SourceLocked == 0);
    check("LAUNCH(1,1): v2Allowed=1", d.v2Allowed == 1);

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_PROFILE);
    check("PROFILE(1,1): v1SourceLocked=0", d.v1SourceLocked == 0);
    check("PROFILE(1,1): v2Allowed=1", d.v2Allowed == 1);

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_HUD);
    check("HUD(1,1): v1SourceLocked=0", d.v1SourceLocked == 0);
    check("HUD(1,1): v2Allowed=1", d.v2Allowed == 1);
    check("HUD(1,1): sourceAnchor mentions SKULL.ASM T560",
        strstr(d.sourceAnchor, "SKULL.ASM T560") != NULL);
    check("HUD(1,1): rule mentions Phase 3",
        strstr(d.rule, "Phase 3") != NULL ||
        strstr(d.rule, "HUD") != NULL);

    /* ── HUD gate: PROFILE-only (0,1) → HUD gated (no LAUNCH) ─────── */
    printf("\n[ HUD gate: PROFILE-only config (invalid) ]\n");
    cfg.v2LaunchEnabled = 0;
    cfg.v2ProfileEnabled = 1;

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_PROFILE);
    check("PROFILE(0,1): v1SourceLocked=1", d.v1SourceLocked == 1);
    check("PROFILE(0,1): v2Allowed=0", d.v2Allowed == 0);

    d = dm2_v2_phase_gate_decide(&cfg, DM2_V2_PHASE_DOMAIN_HUD);
    check("HUD(0,1): v1SourceLocked=1 (no LAUNCH)", d.v1SourceLocked == 1);
    check("HUD(0,1): v2Allowed=0", d.v2Allowed == 0);

    /* ── Null-config: all domains V1-locked (no crash) ────────────── */
    printf("\n[ Null-config antisymmetric ]\n");
    d = dm2_v2_phase_gate_decide(NULL, DM2_V2_PHASE_DOMAIN_HUD);
    check("HUD null-config: v1SourceLocked=1", d.v1SourceLocked == 1);
    check("HUD null-config: v2Allowed=0", d.v2Allowed == 0);

    /* ── HUD lifecycle (no game data needed) ──────────────────────── */
    printf("\n[ HUD init / set / render / access cycle ]\n");
    DM2_V2_HudOverlay h;
    dm2_v2_hud_init(&h);
    check("HUD init: visible", h.visible == true);
    check("HUD init: opacity=255", h.opacity == 255);
    check("HUD init: compass direction=0", h.compass.direction == 0);

    dm2_v2_hud_set_direction(&h, 2);
    check("set_direction(2): direction=2", h.compass.direction == 2);

    dm2_v2_hud_set_level(&h, 5, 10);
    check("set_level(5,10): cur=5", h.depth.current_level == 5);
    check("set_level(5,10): max=10", h.depth.max_level == 10);

    dm2_v2_hud_set_gold(&h, 9999);
    check("set_gold(9999): party_gold=9999", h.gold.party_gold == 9999);

    dm2_v2_hud_set_champion_bar(&h, 0, 75, 50, 100, true, false);
    dm2_v2_hud_set_champion_bar(&h, 1, 30, 40, 50, false, true);
    check("set_champion_bar[0]: hp=75", h.champion_bars[0].hp_pct == 75);
    check("set_champion_bar[0]: leader=true", h.champion_bars[0].leader == true);
    check("set_champion_bar[1]: hp=30", h.champion_bars[1].hp_pct == 30);
    check("set_champion_bar[1]: spell_ready=true", h.champion_bars[1].spell_ready == true);

    dm2_v2_hud_set_action_active(&h, DM2_V2_ACTION_CAST);
    check("set_action_active(CST): CST active",
        h.action_strip.icons[DM2_V2_ACTION_CAST].active == true);
    check("set_action_active(CST): others inactive",
        !h.action_strip.icons[DM2_V2_ACTION_ATTACK].active);

    /* ── Render into 320×200 buffer (no SDL) ───────────────────────── */
    uint8_t fb[320 * 200];
    memset(fb, 0, sizeof(fb));
    dm2_v2_hud_render(&h, fb, 320, 320);
    int pixels = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) pixels++;
    }
    check("hud_render: some pixels written", pixels > 0);

    /* Hidden HUD: opacity=0 → no pixels */
    memset(fb, 0, sizeof(fb));
    h.opacity = 0;
    dm2_v2_hud_render(&h, fb, 320, 320);
    int zero_pixels = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) zero_pixels++;
    }
    check("hud_render opacity=0: no pixels", zero_pixels == 0);

    /* ── dm2_v2_runtime HUD integration (no crash) ────────────────── */
    printf("\n[ dm2_v2_runtime HUD integration ]\n");
    dm2_v2_runtime_init(2);  /* scale=2 for V2.0/V2.1 */

    DM2_V2_HudOverlay *gh = dm2_v2_runtime_get_hud();
    check("dm2_v2_runtime_get_hud: not NULL", gh != NULL);
    check("dm2_v2_runtime_get_hud: visible", gh->visible == true);
    check("dm2_v2_runtime_get_hud: opacity=255", gh->opacity == 255);

    dm2_v2_runtime_set_hud_enabled(0);
    dm2_v2_runtime_hud_render(fb, 320, 320);  /* should be no-op */
    int still_zero = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) still_zero++;
    }
    check("hud_render when disabled: no-op", still_zero == 0);

    dm2_v2_runtime_set_hud_enabled(1);
    dm2_v2_hud_set_direction(gh, 1);
    dm2_v2_hud_set_level(gh, 3, 8);
    dm2_v2_hud_set_gold(gh, 5432);
    dm2_v2_hud_set_champion_bar(gh, 0, 80, 60, 90, false, false);
    dm2_v2_hud_set_action_active(gh, DM2_V2_ACTION_ATTACK);
    dm2_v2_runtime_hud_render(fb, 320, 320);
    int hud_pixels = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) hud_pixels++;
    }
    check("hud_render when enabled: pixels written", hud_pixels > 0);
    check("hud direction=E after set", gh->compass.direction == 1);
    check("hud level=3/8 after set", gh->depth.current_level == 3 && gh->depth.max_level == 8);
    check("hud gold=5432 after set", gh->gold.party_gold == 5432);
    check("hud ATK active after set", gh->action_strip.icons[DM2_V2_ACTION_ATTACK].active == true);

    /* Null safety */
    dm2_v2_runtime_set_hud_enabled(1);
    dm2_v2_runtime_hud_render(NULL, 320, 320);  /* must not crash */
    check("hud_render(NULL): no crash", 1);

    /* ── Source evidence ─────────────────────────────────────────── */
    printf("\n[ Source evidence ]\n");
    const char *ev_hud = dm2_v2_hud_source_evidence();
    check("hud_source_evidence: not NULL", ev_hud != NULL && strlen(ev_hud) > 10);
    check("hud_source_evidence: mentions SKULL.ASM",
        strstr(ev_hud, "SKULL.ASM") != NULL);
    check("hud_source_evidence: mentions SKULLWIN",
        strstr(ev_hud, "SKULLWIN") != NULL);
    check("hud_source_evidence: mentions ReDMCSB PANEL.C",
        strstr(ev_hud, "PANEL.C") != NULL);

    /* ── Result ───────────────────────────────────────────────────── */
    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}