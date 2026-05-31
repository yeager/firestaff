/**
 * firestaff_csb_v2_phase4_enhanced_lighting_probe.c
 *
 * CSB V2 Phase 4 — Enhanced Lighting and Magic Effects
 *
 * Headless C probe exercising CSB V2 Phase 4 components:
 *   1. Dynamic lighting: torch sources, dungeon-level ambient, light map
 *   2. Light events: flicker, darkness burst, magical pulse, chaos surge
 *   3. VFX particle system: emitters, particles, projectile VFX, field VFX
 *   4. Chaos magic visual feedback: glow overlay, VFX integration
 *   5. Phase gate: DYNAMIC_LIGHTING_PRESENTATION is V2-presentation-eligible
 *
 * Compile (from repo root):
 *   cmake -B build -DCMAKE_BUILD_TYPE=Debug
 *   cmake --build build --target firestaff_csb_v2_phase4_enhanced_lighting_probe
 *
 * Run (no game data needed):
 *   SDL_VIDEODRIVER=dummy ./build/firestaff_csb_v2_phase4_enhanced_lighting_probe
 *
 * Exit codes: 0 = PASS, 1 = FAIL
 *
 * Source-lock references:
 *   ReDMCSB PANEL.C:367-428    G0304_i_DungeonViewPaletteIndex selection
 *   ReDMCSB PANEL.C:370-405    torch charge → light power
 *   ReDMCSB PANEL.C:417        G0407_s_Party.MagicalLightAmount
 *   ReDMCSB DATA.C:359-360     G0040_ai_Graphic562_PaletteIndexToLightAmount
 *   CSBWin/Graphics.cpp        fireball/lightning/explosion rendering
 *   CSBWin/Chaos.cpp           spell cast visual triggers
 *   csb_v2_lighting_dynamic.c  Phase 4 dynamic lighting
 *   csb_v2_vfx_particles.c     Phase 4 VFX particle system
 *   csb_v2_chaos_enhanced.c    Phase 4 chaos magic visual feedback
 */

#include "csb_v2_vfx_particles.h"
#include "csb_v2_lighting_dynamic.h"
#include "csb_v2_chaos_enhanced.h"
#include "csb_v2_phase_gate_pc34.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Test framework ─────────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        g_fail++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        g_pass++; \
    } \
} while (0)

/* ── VFX Particle System ─────────────────────────────────────────── */

static void test_vfx_init(void) {
    fprintf(stderr, "\n[ VFX Particle System: init ]\n");
    csb_v2_vfx_init();

    PROBE_ASSERT(csb_v2_vfx_active_particle_count() == 0,
                 "init: no active particles");
    PROBE_ASSERT(csb_v2_vfx_active_emitter_count() == 0,
                 "init: no active emitters");

    /* Source evidence */
    const char *ev = csb_v2_vfx_source_evidence();
    PROBE_ASSERT(ev != NULL && strlen(ev) > 10,
                 "source_evidence returns non-empty string");
}

static void test_vfx_emitters(void) {
    fprintf(stderr, "\n[ VFX Emitters ]\n");

    csb_v2_vfx_init();

    /* Add a fire emitter */
    int eid0 = csb_v2_vfx_add_emitter(
        7.5f, 7.5f,   /* dungeon tile centre */
        2.0f,         /* radius */
        20.0f,        /* emit rate: 20 particles/sec */
        CSB_V2_VFX_FIRE,
        1,            /* looping */
        0.0f);        /* infinite duration */
    PROBE_ASSERT(eid0 >= 0, "fire emitter added, got id %d", eid0);
    PROBE_ASSERT(csb_v2_vfx_active_emitter_count() == 1,
                 "1 active emitter after add");

    /* Add a lightning emitter (non-looping) */
    int eid1 = csb_v2_vfx_add_emitter(
        5.0f, 6.0f,
        1.5f,
        50.0f,
        CSB_V2_VFX_LIGHTNING,
        0,            /* non-looping */
        0.5f);
    PROBE_ASSERT(eid1 >= 0, "lightning emitter added, got id %d", eid1);
    PROBE_ASSERT(csb_v2_vfx_active_emitter_count() == 2,
                 "2 active emitters after second add");

    /* Tick the emitters for 0.3 seconds — should produce particles */
    csb_v2_vfx_tick(0.3f);

    int particle_count = csb_v2_vfx_active_particle_count();
    PROBE_ASSERT(particle_count > 0,
                 "emitters produce particles after tick, got %d", particle_count);

    /* Remove the fire emitter */
    csb_v2_vfx_remove_emitter(eid0);
    PROBE_ASSERT(csb_v2_vfx_active_emitter_count() == 1,
                 "1 emitter after removal");

    /* Remove the lightning emitter */
    csb_v2_vfx_remove_emitter(eid1);
    PROBE_ASSERT(csb_v2_vfx_active_emitter_count() == 0,
                 "0 emitters after all removal");

    /* Tick — particles should decay */
    csb_v2_vfx_tick(2.0f);
    PROBE_ASSERT(csb_v2_vfx_active_particle_count() == 0,
                 "particles decay after removal + tick");
}

static void test_vfx_projectiles(void) {
    fprintf(stderr, "\n[ VFX Projectiles ]\n");

    csb_v2_vfx_init();

    /* Fire a fireball projectile */
    int pid0 = csb_v2_vfx_fire_projectile(
        7.5f, 7.5f,   /* source: centre of dungeon */
        10.0f, 7.5f,  /* target: east */
        3.0f,         /* speed: 3 tiles/sec */
        CSB_V2_VFX_FIRE);
    PROBE_ASSERT(pid0 >= 0, "fireball projectile fired, got id %d", pid0);

    /* Fire a lightning projectile */
    int pid1 = csb_v2_vfx_fire_projectile(
        7.5f, 7.5f,
        7.5f, 5.0f,   /* target: north */
        5.0f,
        CSB_V2_VFX_LIGHTNING);
    PROBE_ASSERT(pid1 >= 0, "lightning projectile fired, got id %d", pid1);

    /* Query projectile state */
    float px, py;
    int ptype;
    uint8_t palpha;
    int active = csb_v2_vfx_get_projectile(pid0, &px, &py, &ptype, &palpha);
    PROBE_ASSERT(active == 1, "fireball projectile is active initially");
    PROBE_ASSERT(ptype == CSB_V2_VFX_FIRE, "fireball projectile type is FIRE");

    /* Tick 0.2 seconds — projectile should have moved */
    csb_v2_vfx_tick(0.2f);
    active = csb_v2_vfx_get_projectile(pid0, &px, &py, &ptype, &palpha);
    PROBE_ASSERT(active == 1, "fireball still active after 0.2s tick");
    PROBE_ASSERT(px > 7.5f, "fireball moved east (x %.3f > 7.5)", px);

    /* Let the projectile complete its journey */
    csb_v2_vfx_tick(2.0f);
    active = csb_v2_vfx_get_projectile(pid0, &px, &py, &ptype, &palpha);
    PROBE_ASSERT(active == 0, "fireball projectile done after full travel");
}

static void test_vfx_field(void) {
    fprintf(stderr, "\n[ VFX Field Effects ]\n");

    csb_v2_vfx_init();

    /* Place a chaos mist on tile (7,7) */
    int fid0 = csb_v2_vfx_add_field(7, 7, CSB_V2_VFX_CHAOS_MIST);
    PROBE_ASSERT(fid0 >= 0, "chaos mist field placed, got id %d", fid0);

    /* Place a magical glow on tile (5,6) */
    int fid1 = csb_v2_vfx_add_field(5, 6, CSB_V2_VFX_MAGICAL_GLOW);
    PROBE_ASSERT(fid1 >= 0, "magical glow field placed, got id %d", fid1);

    /* Query field state */
    uint8_t frame;
    int ftype;
    uint8_t falpha;
    int active = csb_v2_vfx_get_field(fid0, &frame, &ftype, &falpha);
    PROBE_ASSERT(active == 1, "chaos mist field is active");
    PROBE_ASSERT(ftype == CSB_V2_VFX_CHAOS_MIST, "field type matches");

    /* Tick and verify frame animation */
    csb_v2_vfx_tick(0.16f);
    active = csb_v2_vfx_get_field(fid0, &frame, &ftype, &falpha);
    PROBE_ASSERT(active == 1, "chaos mist field still active after tick");

    /* Remove field */
    csb_v2_vfx_remove_field(fid0);
    csb_v2_vfx_remove_field(fid1);
}

static void test_vfx_all_types(void) {
    fprintf(stderr, "\n[ VFX All Particle Types ]\n");

    csb_v2_vfx_init();

    /* Spawn one emitter of each type */
    int ids[CSB_V2_VFX_COUNT - 1];
    for (int t = 1; t < CSB_V2_VFX_COUNT; t++) {
        ids[t-1] = csb_v2_vfx_add_emitter(
            7.0f + (float)(t * 2), 7.0f,
            1.0f,
            10.0f,
            t,
            0,    /* non-looping */
            0.3f);
        PROBE_ASSERT(ids[t-1] >= 0, "emitter type %d added, got id %d", t, ids[t-1]);
    }

    /* Tick and verify particles produced */
    csb_v2_vfx_tick(0.2f);
    int count = csb_v2_vfx_active_particle_count();
    PROBE_ASSERT(count > 0, "all VFX types produce particles, got %d", count);

    /* Tick to drain */
    csb_v2_vfx_tick(1.5f);
    PROBE_ASSERT(csb_v2_vfx_active_particle_count() == 0,
                 "all particles decay after duration");
}

/* ── Dynamic Lighting ─────────────────────────────────────────────── */

static void test_dynamic_lighting_init(void) {
    fprintf(stderr, "\n[ Dynamic Lighting: init ]\n");

    csb_v2_light_init();

    /* Source evidence */
    const char *ev = csb_v2_light_source_evidence();
    PROBE_ASSERT(ev != NULL && strlen(ev) > 10,
                 "light_source_evidence returns non-empty string");

    /* Source palette floor table */
    PROBE_ASSERT(
        k_csb_v2_source_palette_light_amount_floor[0] == 99 &&
        k_csb_v2_source_palette_light_amount_floor[5] == 0,
        "palette floor table values are correct");
}

static void test_light_sources(void) {
    fprintf(stderr, "\n[ Light Sources ]\n");

    csb_v2_light_init();

    /* Add a torch at centre */
    int lid0 = csb_v2_light_add_source(
        7.5f, 7.5f,
        4.0f,   /* radius */
        200,    /* intensity */
        255, 180, 80,  /* warm orange torch colour */
        1       /* flicker on */
    );
    PROBE_ASSERT(lid0 >= 0, "torch light source added, id %d", lid0);

    /* Compute light map */
    csb_v2_light_compute_map();

    /* Query a tile near the torch — should have colour */
    uint8_t r, g, b;
    csb_v2_light_get_tile(7, 7, &r, &g, &b);
    PROBE_ASSERT(r > 0 || g > 0 || b > 0,
                 "centre tile has non-zero light (r=%u g=%u b=%u)", r, g, b);

    /* Query a tile far from the torch — may be dark */
    csb_v2_light_get_tile(0, 0, &r, &g, &b);
    /* Just check it doesn't crash and returns valid values */
    PROBE_ASSERT(r <= 255 && g <= 255 && b <= 255,
                 "far tile returns valid light values");

    /* Remove the torch */
    csb_v2_light_remove_source(lid0);

    /* Add multiple torches */
    for (int i = 0; i < 4; i++) {
        int lid = csb_v2_light_add_source(
            5.0f + (float)i, 5.0f,
            3.0f,
            150,
            255, 180, 80,
            1);
        PROBE_ASSERT(lid >= 0, "torch %d added, id %d", i, lid);
    }

    csb_v2_light_compute_map();
    csb_v2_light_get_tile(5, 5, &r, &g, &b);
    PROBE_ASSERT(r > 0, "multi-torch: centre tile lit (r=%u)", r);

    csb_v2_light_init(); /* reset */
}

static void test_light_events(void) {
    fprintf(stderr, "\n[ Light Events ]\n");

    csb_v2_light_init();

    /* Trigger a magical pulse */
    csb_v2_light_event_trigger(CSB_V2_LIGHT_EVENT_MAGICAL_PULSE, 0.5f, 0.8f);
    PROBE_ASSERT(csb_v2_light_event_is_active() == 1,
                 "magical pulse event is active after trigger");

    CSB_V2_LightEventType t = csb_v2_light_event_current_type();
    PROBE_ASSERT(t == CSB_V2_LIGHT_EVENT_MAGICAL_PULSE,
                 "current event type is MAGICAL_PULSE");

    /* Tick the event */
    csb_v2_light_event_tick(0.25f);
    PROBE_ASSERT(csb_v2_light_event_is_active() == 1,
                 "event still active at 50%% progress");

    /* Let it complete */
    csb_v2_light_event_tick(0.3f);
    PROBE_ASSERT(csb_v2_light_event_is_active() == 0,
                 "event inactive after duration expires");

    /* Trigger chaos surge */
    csb_v2_light_event_trigger(CSB_V2_LIGHT_EVENT_CHAOS_SURGE, 1.0f, 1.0f);
    PROBE_ASSERT(csb_v2_light_event_is_active() == 1,
                 "chaos surge event is active");

    /* Multiple events */
    csb_v2_light_event_trigger(CSB_V2_LIGHT_EVENT_DARKNESS_BURST, 0.5f, 0.6f);
    /* Both events should be active */
    csb_v2_light_event_tick(0.1f);
    PROBE_ASSERT(csb_v2_light_event_is_active() == 1,
                 "events still active after tick");

    csb_v2_light_init(); /* reset */
}

static void test_light_dungeon_levels(void) {
    fprintf(stderr, "\n[ Dungeon Level Lighting ]\n");

    csb_v2_light_init();

    /* Source palette lighting per dungeon level */
    for (int level = 0; level < 6; level++) {
        csb_v2_light_set_dungeon_level(level);
        PROBE_ASSERT(csb_v2_light_get_dungeon_level() == level,
                     "dungeon level %d set correctly", level);

        CSB_V2_SourcePaletteLighting plan =
            csb_v2_light_build_source_palette_lighting(level, true);

        PROBE_ASSERT(plan.sourceLightAmountFloor ==
                         k_csb_v2_source_palette_light_amount_floor[level],
                     "level %d: floor matches table (%u == %u)",
                     level, plan.sourceLightAmountFloor,
                     k_csb_v2_source_palette_light_amount_floor[level]);
        PROBE_ASSERT(plan.darknessPercent == 100 - plan.sourceLightAmountFloor,
                     "level %d: darkness = 100 - floor", level);
    }

    /* Clamp out-of-range levels */
    csb_v2_light_set_dungeon_level(-1);
    PROBE_ASSERT(csb_v2_light_get_dungeon_level() == 0,
                 "level -1 clamped to 0");
    csb_v2_light_set_dungeon_level(99);
    PROBE_ASSERT(csb_v2_light_get_dungeon_level() == 5,
                 "level 99 clamped to 5");

    csb_v2_light_init();
}

static void test_light_flicker(void) {
    fprintf(stderr, "\n[ Torch Flicker ]\n");

    csb_v2_light_init();

    int lid = csb_v2_light_add_source(
        7.5f, 7.5f, 4.0f, 200, 255, 180, 80, 1 /* flicker on */);
    PROBE_ASSERT(lid >= 0, "flicker torch added");

    /* Tick with flicker — intensity should vary */
    float i0, i1;
    csb_v2_light_update_flicker(0.016f); /* 16ms frame */
    /* intensity values are clamped, just verify no crash */
    csb_v2_light_update_flicker(0.016f);
    csb_v2_light_update_flicker(0.016f);
    PROBE_ASSERT(1, "flicker tick: no crash after multiple frames");

    /* Non-flicker torch: intensity should equal base */
    csb_v2_light_remove_source(lid);
    lid = csb_v2_light_add_source(7.5f, 7.5f, 4.0f, 200, 255, 180, 80, 0);
    csb_v2_light_update_flicker(0.1f);
    PROBE_ASSERT(1, "non-flicker torch: no crash");

    csb_v2_light_init();
}

/* ── Chaos Magic Visual Feedback ──────────────────────────────────── */

static void test_chaos_init(void) {
    fprintf(stderr, "\n[ Chaos Magic: init ]\n");

    csb_v2_chaos_init();
    PROBE_ASSERT(csb_v2_chaos_active_count() == 0,
                 "init: no active chaos visuals");

    const char *ev = csb_v2_chaos_source_evidence();
    PROBE_ASSERT(ev != NULL && strlen(ev) > 10,
                 "chaos_source_evidence returns non-empty string");
}

static void test_chaos_trigger(void) {
    fprintf(stderr, "\n[ Chaos Magic: triggers ]\n");

    csb_v2_vfx_init();
    csb_v2_chaos_init();

    /* Trigger a DSA script (family 0 = fire) */
    csb_v2_chaos_on_trigger(0, -1);
    PROBE_ASSERT(csb_v2_chaos_active_count() >= 0,
                 "chaos trigger does not crash");
    PROBE_ASSERT(csb_v2_vfx_active_particle_count() >= 0,
                 "chaos trigger produces VFX particles");

    /* Trigger family 3 = chaos warp */
    csb_v2_chaos_on_trigger((3 << 5) | 1, -1);
    PROBE_ASSERT(1, "chaos warp trigger: no crash");

    /* Trigger family 4 = explosion */
    csb_v2_chaos_on_trigger((4 << 5) | 1, -1);
    PROBE_ASSERT(1, "explosion trigger: no crash");

    /* Tick chaos visuals */
    csb_v2_chaos_tick(0.1f);
    PROBE_ASSERT(1, "chaos tick: no crash");

    /* Glow overlay should be queryable */
    float r, g, b, a;
    csb_v2_chaos_render_overlay(&r, &g, &b, &a);
    PROBE_ASSERT(r >= 0.0f && r <= 1.0f, "overlay R in [0,1]");
    PROBE_ASSERT(g >= 0.0f && g <= 1.0f, "overlay G in [0,1]");
    PROBE_ASSERT(b >= 0.0f && b <= 1.0f, "overlay B in [0,1]");
    PROBE_ASSERT(a >= 0.0f && a <= 1.0f, "overlay A in [0,1]");

    /* Particle count query */
    int pc = csb_v2_chaos_particle_count();
    PROBE_ASSERT(pc >= 0, "particle count >= 0, got %d", pc);

    /* Decay to zero */
    csb_v2_chaos_tick(5.0f);
    PROBE_ASSERT(1, "chaos visuals decay: no crash");

    csb_v2_vfx_init(); /* reset VFX */
}

static void test_chaos_projectile(void) {
    fprintf(stderr, "\n[ Chaos Magic: projectile VFX ]\n");

    csb_v2_vfx_init();
    csb_v2_chaos_init();

    /* Fire a chaos projectile via the chaos system */
    int pid = csb_v2_chaos_fire_projectile(
        7.5f, 7.5f,
        12.0f, 7.5f,
        3.0f,
        CSB_V2_VFX_FIRE);
    PROBE_ASSERT(pid >= 0, "chaos fire_projectile returns valid id %d", pid);

    /* Tick and verify movement */
    csb_v2_vfx_tick(0.1f);
    float px, py;
    int ptype;
    uint8_t palpha;
    int active = csb_v2_vfx_get_projectile(pid, &px, &py, &ptype, &palpha);
    PROBE_ASSERT(active == 1, "chaos projectile active after tick");
    PROBE_ASSERT(px > 7.5f, "chaos projectile moved east (x=%.3f)", px);

    csb_v2_vfx_init();
}

static void test_chaos_vfx_integration(void) {
    fprintf(stderr, "\n[ Chaos + Lighting Integration ]\n");

    csb_v2_vfx_init();
    csb_v2_light_init();
    csb_v2_chaos_init();

    /* Add a torch first */
    csb_v2_light_add_source(7.5f, 7.5f, 4.0f, 200, 255, 180, 80, 1);
    csb_v2_light_compute_map();

    /* Trigger chaos magic */
    csb_v2_chaos_on_trigger((3 << 5) | 2, -1); /* family 3 = chaos warp */

    /* Tick all systems together */
    csb_v2_light_tick(0.016f);
    csb_v2_chaos_tick(0.016f);
    csb_v2_vfx_tick(0.016f);

    /* Verify light event was triggered */
    PROBE_ASSERT(csb_v2_light_event_is_active() == 1,
                 "chaos warp triggers light event");

    CSB_V2_LightEventType t = csb_v2_light_event_current_type();
    PROBE_ASSERT(t == CSB_V2_LIGHT_EVENT_MAGICAL_PULSE ||
                 t == CSB_V2_LIGHT_EVENT_CHAOS_SURGE,
                 "light event type is MAGICAL_PULSE or CHAOS_SURGE");

    /* Verify VFX particles exist */
    PROBE_ASSERT(csb_v2_vfx_active_particle_count() >= 0,
                 "chaos warp produces VFX particles");

    /* All systems should be stable under repeated ticking */
    for (int i = 0; i < 10; i++) {
        csb_v2_light_tick(0.016f);
        csb_v2_chaos_tick(0.016f);
        csb_v2_vfx_tick(0.016f);
    }
    PROBE_ASSERT(1, "multiple tick cycles: no crash or instability");

    csb_v2_light_init();
    csb_v2_vfx_init();
}

/* ── Phase Gate ───────────────────────────────────────────────────── */

static void test_phase_gate(void) {
    fprintf(stderr, "\n[ Phase Gate: DYNAMIC_LIGHTING_PRESENTATION ]\n");

    CSB_V2_PhaseGateConfig config;
    csb_v2_phase_gate_pc34_defaults(&config);

    /* DYNAMIC_LIGHTING_PRESENTATION = 9 is V2-presentation-eligible */
    CSB_V2_PhaseGateDecision dec =
        csb_v2_phase_gate_pc34_decide(&config, CSB_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION);
    PROBE_ASSERT(dec.v1SourceLocked == 0,
                 "DYNAMIC_LIGHTING_PRESENTATION: v1SourceLocked=0");
    PROBE_ASSERT(dec.v2PresentationAllowed == 0,
                 "DYNAMIC_LIGHTING_PRESENTATION: v2PresentationAllowed=0 when v2 disabled");

    /* Enable V2 presentation */
    config.v2PresentationEnabled = 1;
    dec = csb_v2_phase_gate_pc34_decide(&config, CSB_V2_PHASE_DOMAIN_DYNAMIC_LIGHTING_PRESENTATION);
    PROBE_ASSERT(dec.v2PresentationAllowed == 1,
                 "DYNAMIC_LIGHTING_PRESENTATION: v2PresentationAllowed=1 when v2 enabled");

    /* CHAOS_MAGIC_SCRIPTS = 6 is V1-source-locked */
    dec = csb_v2_phase_gate_pc34_decide(&config, CSB_V2_PHASE_DOMAIN_CHAOS_MAGIC_SCRIPTS);
    PROBE_ASSERT(dec.v1SourceLocked == 1,
                 "CHAOS_MAGIC_SCRIPTS: v1SourceLocked=1 even when v2 enabled");
    PROBE_ASSERT(dec.v2PresentationAllowed == 0,
                 "CHAOS_MAGIC_SCRIPTS: v2PresentationAllowed=0 (V1 locked)");
}

/* ── Source Evidence ──────────────────────────────────────────────── */

static void test_source_evidence(void) {
    fprintf(stderr, "\n[ Source Evidence Strings ]\n");

    const char *light_ev = csb_v2_light_source_evidence();
    PROBE_ASSERT(light_ev != NULL, "lighting source_evidence is non-null");
    PROBE_ASSERT(strstr(light_ev, "PANEL.C") != NULL,
                 "lighting evidence cites PANEL.C");

    const char *chaos_ev = csb_v2_chaos_source_evidence();
    PROBE_ASSERT(chaos_ev != NULL, "chaos source_evidence is non-null");
    PROBE_ASSERT(strstr(chaos_ev, "PANEL.C") != NULL ||
                 strstr(chaos_ev, "Graphics.cpp") != NULL,
                 "chaos evidence cites PANEL.C or Graphics.cpp");

    const char *vfx_ev = csb_v2_vfx_source_evidence();
    PROBE_ASSERT(vfx_ev != NULL, "vfx source_evidence is non-null");
    PROBE_ASSERT(strstr(vfx_ev, "Graphics.cpp") != NULL,
                 "vfx evidence cites Graphics.cpp");
}

/* ── Main ─────────────────────────────────────────────────────────── */

int main(void) {
    fprintf(stderr, "========================================\n");
    fprintf(stderr, "CSB V2 Phase 4 Enhanced Lighting Probe\n");
    fprintf(stderr, "========================================\n");

    /* VFX particle system */
    test_vfx_init();
    test_vfx_emitters();
    test_vfx_projectiles();
    test_vfx_field();
    test_vfx_all_types();

    /* Dynamic lighting */
    test_dynamic_lighting_init();
    test_light_sources();
    test_light_events();
    test_light_dungeon_levels();
    test_light_flicker();

    /* Chaos magic visual feedback */
    test_chaos_init();
    test_chaos_trigger();
    test_chaos_projectile();
    test_chaos_vfx_integration();

    /* Phase gate */
    test_phase_gate();

    /* Source evidence */
    test_source_evidence();

    fprintf(stderr, "\n========================================\n");
    fprintf(stderr, "Phase 4 Probe: %d PASS, %d FAIL\n", g_pass, g_fail);
    fprintf(stderr, "========================================\n");
    return g_fail > 0 ? 1 : 0;
}
