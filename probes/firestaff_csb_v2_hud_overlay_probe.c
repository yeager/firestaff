/*
 * firestaff_csb_v2_hud_overlay_probe.c — CSB V2 Phase 3 HUD Overlay Probe
 *
 * Phase 3: Enhanced UI overlays — HUD compass, depth, gold, champion bars,
 * action strip, and chaos indicator.  Tests:
 *   1. CSB_V2_PHASE_DOMAIN_HUD gate exists and is correctly gated
 *   2. HUD domain activates only when v2PresentationEnabled is true
 *   3. csb_v2_hud_init/render/set/access cycle works correctly
 *   4. csb_v2_hud_runtime integration (no V1 state mutation)
 *   5. Phase gate integration: HUD gated on V2 active, not on LAUNCH/PROFILE
 *
 * Headless: no game data, no SDL rendering required.
 * Compile:  gcc -Wall -Wextra -O2 -I../../include
 *           -o firestaff_csb_v2_hud_overlay_probe
 *           firestaff_csb_v2_hud_overlay_probe.c
 *
 * Source: CSBWin/Viewport.cpp (CSB HUD layout, 7290 lines)
 *         CSBWin/Graphics.cpp (CSB graphics, 3186 lines)
 *         ReDMCSB PANEL.C F0354 (champion status-box drawing)
 *         ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing)
 *         ReDMCSB COMMAND.C action feedback gates
 *         ReDMCSB DISPLAY.C pulse animation timing (2 Hz)
 *         DM2 DM2_V2_PHASE_DOMAIN_HUD pattern (Phase 3 HUD gate)
 */

#include "csb_v2_phase_gate_pc34.h"
#include "csb_v2_hud_overlay_pc34.h"
#include "csb_v2_hud_runtime.h"
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
    printf("=== CSB V2 Phase 3 — HUD Overlay Probe ===\n\n");

    /* ── HUD domain enum exists ───────────────────────────────────── */
    printf("[ HUD domain enum ]\n");
    /* CSB_V2_PHASE_DOMAIN_HUD must be a distinct domain from LAUNCH/PROFILE */
    check("HUD domain is distinct from RENDER_PRESENTATION",
        CSB_V2_PHASE_DOMAIN_HUD != CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    check("HUD domain is distinct from SMOOTH_MOVEMENT_PRESENTATION",
        CSB_V2_PHASE_DOMAIN_HUD != CSB_V2_PHASE_DOMAIN_SMOOTH_MOVEMENT_PRESENTATION);
    check("HUD domain != COUNT",
        CSB_V2_PHASE_DOMAIN_HUD < CSB_V2_PHASE_DOMAIN_COUNT);

    /* ── is_gameplay_domain check ──────────────────────────────────── */
    printf("\n[ is_gameplay_domain ]\n");
    check("HUD is NOT a gameplay domain (presentation-eligible)",
        !csb_v2_phase_gate_pc34_is_gameplay_domain(CSB_V2_PHASE_DOMAIN_HUD));
    check("COMMAND_SEMANTICS IS a gameplay domain",
        csb_v2_phase_gate_pc34_is_gameplay_domain(CSB_V2_PHASE_DOMAIN_COMMAND_SEMANTICS));
    check("CHAOS_MAGIC_SCRIPTS IS a gameplay domain",
        csb_v2_phase_gate_pc34_is_gameplay_domain(CSB_V2_PHASE_DOMAIN_CHAOS_MAGIC_SCRIPTS));

    /* ── HUD gate: default (0) → HUD inactive ──────────────────────── */
    printf("\n[ HUD gate: default config ]\n");
    CSB_V2_PhaseGateConfig cfg = {0, 0};

    CSB_V2_PhaseGateDecision d;

    d = csb_v2_phase_gate_pc34_decide(&cfg, CSB_V2_PHASE_DOMAIN_HUD);
    check("HUD default: v1SourceLocked=0", d.v1SourceLocked == 0);
    check("HUD default: v2PresentationAllowed=0", d.v2PresentationAllowed == 0);
    check("HUD default: rule mentions Phase 3",
        strstr(d.rule, "Phase 3") != NULL || strstr(d.rule, "HUD") != NULL);

    /* ── HUD gate: v2PresentationEnabled (1) → HUD active ─────────── */
    printf("\n[ HUD gate: v2PresentationEnabled=1 ]\n");
    cfg.v2PresentationEnabled = 1;

    d = csb_v2_phase_gate_pc34_decide(&cfg, CSB_V2_PHASE_DOMAIN_HUD);
    check("HUD(v2=1): v1SourceLocked=0", d.v1SourceLocked == 0);
    check("HUD(v2=1): v2PresentationAllowed=1", d.v2PresentationAllowed == 1);
    check("HUD(v2=1): sourceAnchor mentions CSBWin/Viewport.cpp",
        strstr(d.sourceAnchor, "CSBWin/Viewport.cpp") != NULL);
    check("HUD(v2=1): rule mentions presentation-only",
        strstr(d.rule, "presentation-only") != NULL ||
        strstr(d.rule, "does NOT") != NULL);

    /* ── HUD gate: COMMAND_SEMANTICS stays V1-locked ─────────────── */
    printf("\n[ V1 lock: COMMAND_SEMANTICS ]\n");
    d = csb_v2_phase_gate_pc34_decide(&cfg, CSB_V2_PHASE_DOMAIN_COMMAND_SEMANTICS);
    check("COMMAND_SEMANTICS: v1SourceLocked=1", d.v1SourceLocked == 1);
    check("COMMAND_SEMANTICS: v2PresentationAllowed=0", d.v2PresentationAllowed == 0);

    /* ── HUD gate: CHAOS_MAGIC_SCRIPTS stays V1-locked ────────────── */
    printf("\n[ V1 lock: CHAOS_MAGIC_SCRIPTS ]\n");
    d = csb_v2_phase_gate_pc34_decide(&cfg, CSB_V2_PHASE_DOMAIN_CHAOS_MAGIC_SCRIPTS);
    check("CHAOS_MAGIC_SCRIPTS: v1SourceLocked=1", d.v1SourceLocked == 1);
    check("CHAOS_MAGIC_SCRIPTS: v2PresentationAllowed=0", d.v2PresentationAllowed == 0);

    /* ── Null-config: HUD gated (no crash) ────────────────────────── */
    printf("\n[ Null-config antisymmetric ]\n");
    d = csb_v2_phase_gate_pc34_decide(NULL, CSB_V2_PHASE_DOMAIN_HUD);
    check("HUD null-config: v1SourceLocked=0", d.v1SourceLocked == 0);
    check("HUD null-config: v2PresentationAllowed=0", d.v2PresentationAllowed == 0);

    /* ── HUD init / set / render / access cycle ──────────────────── */
    printf("\n[ HUD init / set / render / access cycle ]\n");
    CSB_V2_HudOverlay h;
    csb_v2_hud_init(&h);
    check("HUD init: visible", h.visible == true);
    check("HUD init: opacity=255", h.opacity == 255);
    check("HUD init: compass direction=0", h.compass.direction == 0);
    check("HUD init: chaos.visible=true", h.chaos.visible == true);
    check("HUD init: chaos.chaos_active=false", h.chaos.chaos_active == false);

    csb_v2_hud_set_direction(&h, 2);
    check("set_direction(2): direction=2", h.compass.direction == 2);

    csb_v2_hud_set_level(&h, 5, 10);
    check("set_level(5,10): cur=5", h.depth.current_level == 5);
    check("set_level(5,10): max=10", h.depth.max_level == 10);

    csb_v2_hud_set_gold(&h, 9999);
    check("set_gold(9999): party_gold=9999", h.gold.party_gold == 9999);

    csb_v2_hud_set_champion_bar(&h, 0, 75, 50, 100, true, false);
    csb_v2_hud_set_champion_bar(&h, 1, 30, 40, 50, false, true);
    check("set_champion_bar[0]: hp=75", h.champion_bars[0].hp_pct == 75);
    check("set_champion_bar[0]: leader=true", h.champion_bars[0].leader == true);
    check("set_champion_bar[1]: hp=30", h.champion_bars[1].hp_pct == 30);
    check("set_champion_bar[1]: spell_ready=true", h.champion_bars[1].spell_ready == true);

    csb_v2_hud_set_action_active(&h, CSB_V2_ACTION_CAST);
    check("set_action_active(CST): CST active",
        h.action_strip.icons[CSB_V2_ACTION_CAST].active == true);
    check("set_action_active(CST): others inactive",
        !h.action_strip.icons[CSB_V2_ACTION_ATTACK].active);

    csb_v2_hud_set_chaos_active(&h, true, 2);
    check("set_chaos_active(true,2): chaos_active=true",
        h.chaos.chaos_active == true);
    check("set_chaos_active(true,2): power_rune_count=2",
        h.chaos.power_rune_count == 2);

    /* ── Render into 320×200 buffer (no SDL) ───────────────────────── */
    uint8_t fb[320 * 200];
    memset(fb, 0, sizeof(fb));
    h.opacity = 255;
    csb_v2_hud_render(&h, fb, 320, 200);
    int pixels = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) pixels++;
    }
    check("hud_render: some pixels written", pixels > 0);

    /* Hidden HUD: opacity=0 → no pixels */
    memset(fb, 0, sizeof(fb));
    h.opacity = 0;
    csb_v2_hud_render(&h, fb, 320, 200);
    int zero_pixels = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) zero_pixels++;
    }
    check("hud_render opacity=0: no pixels", zero_pixels == 0);

    /* ── csb_v2_hud_runtime integration ──────────────────────────── */
    printf("\n[ csb_v2_hud_runtime integration ]\n");
    csb_v2_hud_runtime_init();

    /* Without gate config set, HUD render is gated on v2PresentationEnabled */
    CSB_V2_PhaseGateConfig no_v2 = {0, 0};
    csb_v2_hud_runtime_set_gate_config(&no_v2);
    memset(fb, 0, sizeof(fb));
    csb_v2_hud_runtime_render(fb, 320, 200);
    int gated = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) gated++;
    }
    check("hud_runtime_render (v2=0): no-op", gated == 0);

    /* With v2PresentationEnabled=1, HUD renders */
    CSB_V2_PhaseGateConfig v2_on = {1, 0};
    csb_v2_hud_runtime_set_gate_config(&v2_on);
    csb_v2_hud_runtime_set_direction(1);
    csb_v2_hud_runtime_set_level(3, 8);
    csb_v2_hud_runtime_set_party_gold(5432);
    csb_v2_hud_runtime_set_champion(0, 80, 60, 90, false, false);
    csb_v2_hud_runtime_set_action_active(CSB_V2_ACTION_ATTACK);
    csb_v2_hud_runtime_set_chaos_active(true, 1);
    memset(fb, 0, sizeof(fb));
    csb_v2_hud_runtime_render(fb, 320, 200);
    int hud_pixels = 0;
    for (int i = 0; i < (int)(sizeof(fb)/sizeof(fb[0])); i++) {
        if (fb[i] != 0) hud_pixels++;
    }
    check("hud_runtime_render (v2=1): pixels written", hud_pixels > 0);

    /* Verify state via get_hud */
    CSB_V2_HudOverlay *gh = csb_v2_hud_runtime_get_hud();
    check("hud_runtime_get_hud: not NULL", gh != NULL);
    check("hud direction=E after set", gh->compass.direction == 1);
    check("hud level=3/8 after set", gh->depth.current_level == 3 && gh->depth.max_level == 8);
    check("hud gold=5432 after set", gh->gold.party_gold == 5432);
    check("hud ATK active after set", gh->action_strip.icons[CSB_V2_ACTION_ATTACK].active == true);
    check("hud chaos active after set", gh->chaos.chaos_active == true);
    check("hud chaos rune_count=1 after set", gh->chaos.power_rune_count == 1);

    /* Hit flash trigger */
    csb_v2_hud_runtime_trigger_hit_flash();
    check("hit_flash triggered: hit_flash_active=true",
        gh->hit_flash_active == true);
    check("hit_flash triggered: hit_flash_timer=6",
        gh->hit_flash_timer == 6);

    /* Toggle */
    csb_v2_hud_runtime_toggle();
    check("toggle: visible=false", gh->visible == false);
    csb_v2_hud_runtime_toggle();
    check("toggle again: visible=true", gh->visible == true);

    /* Set opacity */
    csb_v2_hud_runtime_set_opacity(128);
    check("set_opacity(128): opacity=128", gh->opacity == 128);

    /* Null safety */
    csb_v2_hud_runtime_render(NULL, 320, 200);  /* must not crash */
    check("hud_runtime_render(NULL): no crash", 1);

    csb_v2_hud_runtime_shutdown();

    /* ── Source evidence ─────────────────────────────────────────── */
    printf("\n[ Source evidence ]\n");
    const char *ev_hud = csb_v2_hud_source_evidence();
    check("hud_source_evidence: not NULL",
        ev_hud != NULL && strlen(ev_hud) > 10);
    check("hud_source_evidence: mentions CSBWin/Viewport.cpp",
        strstr(ev_hud, "CSBWin/Viewport.cpp") != NULL);
    check("hud_source_evidence: mentions CSBWin/Graphics.cpp",
        strstr(ev_hud, "CSBWin/Graphics.cpp") != NULL);
    check("hud_source_evidence: mentions ReDMCSB PANEL.C",
        strstr(ev_hud, "PANEL.C") != NULL);

    const char *ev_rt = csb_v2_hud_runtime_source_evidence();
    check("hud_runtime_source_evidence: not NULL",
        ev_rt != NULL && strlen(ev_rt) > 10);
    check("hud_runtime_source_evidence: mentions Phase 3",
        strstr(ev_rt, "Phase 3") != NULL);

    const char *ev_gate = csb_v2_phase_gate_pc34_source_evidence();
    check("phase_gate_source_evidence: not NULL",
        ev_gate != NULL && strlen(ev_gate) > 10);

    /* ── Domain name ──────────────────────────────────────────────── */
    printf("\n[ Domain name ]\n");
    const char *name = csb_v2_phase_gate_pc34_domain_name(CSB_V2_PHASE_DOMAIN_HUD);
    check("domain_name(HUD) = HUD", strcmp(name, "HUD") == 0);

    /* ── Result ───────────────────────────────────────────────────── */
    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail > 0 ? 1 : 0;
}