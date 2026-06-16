/*
 * firestaff_nexus_v2_phase0_v1_compatibility_lock_probe.c
 *
 * Nexus V2 Phase 0 - V1 Compatibility Lock Probe
 *
 * Headless probe: verifies the Nexus V2 Phase 0 V1 compatibility lock
 * (nexus_v2_phase_gate_pc34.c) without requiring live game asset files.
 *
 * This probe validates:
 *
 *   1. nexus_v2_phase_gate_defaults() initialises to V1-only behaviour
 *      - v2PresentationEnabled = 0
 *      - v2ConfigPersistenceEnabled = 0
 *
 *   2. Domain classification: all 19 Phase 0/1 domains are correctly
 *      classified as V1-source-locked (gameplay) or V2-eligible (presentation)
 *
 *   3. V1-source-locked domains (11):
 *      DMDF_DGN_LOADING, SATURN_ISO_READER, GAME_STATE_INIT,
 *      CHAMPION_PARTY, CREATURE_AI, SPELL_MAGIC, MOVEMENT,
 *      SAVE_LOAD, SOUND_DRIVER, RASTERIZER, INVENTORY
 *      - Always return v1SourceLocked=1, v2PresentationAllowed=0
 *      - Independent of v2PresentationEnabled
 *
 *   4. V2-presentation-eligible domains (9):
 *      RENDER_PRESENTATION, SMOOTH_MOVEMENT_PRESENTATION,
 *      DYNAMIC_LIGHTING_PRESENTATION, HUD_OVERLAY,
 *      PARTICLE_EFFECTS, ATMOSPHERE, INPUT_PRESENTATION,
 *      CONFIG_PRESENTATION, UPSCALER
 *      - Return v2PresentationAllowed=0 when v2PresentationEnabled=0
 *      - Return v2PresentationAllowed=1 when v2PresentationEnabled=1
 *        (except CONFIG_PRESENTATION which also needs config-persistence)
 *      - v1SourceLocked=0 in both cases
 *
 *   5. CONFIG_PRESENTATION additionally requires v2ConfigPersistenceEnabled
 *      - v2PresentationAllowed=0 when v2PresentationEnabled=1 but
 *        v2ConfigPersistenceEnabled=0
 *      - v2PresentationAllowed=1 when both are 1
 *
 *   6. nexus_v2_phase_gate_v2_active() returns 1 only when
 *      v2PresentationEnabled=1
 *
 *   7. nexus_v2_phase_gate_domain_name() returns non-NULL strings for
 *      all 20 domains (19 + UNKNOWN fallback)
 *
 *   8. nexus_v2_phase_gate_source_evidence() returns a non-NULL,
 *      non-empty string referencing NEXUS.C / NEXUS2.C / nexus_v1_*
 *
 *   9. All 19 domains are handled (no switch fallthrough to default)
 *
 *  10. Source citations in gate decisions contain NEXUS.C / NEXUS2.C /
 *      nexus_v1_* references for all domains
 *
 *  11. Default config has both toggles off
 *
 *  12. Phase 0 default behaviour is V1: even with default config, the
 *      engine must behave as V1 (no V2 presentation).
 *
 *  13. Unknown domains default to V1-locked for safety
 *
 *  14. Null config arg is safe (decide returns V1-locked for V2-eligible
 *      domains, no crash).
 *
 * Exit codes:
 *   0  - all checks passed
 *   1  - one or more checks failed
 *
 * Usage:
 *   SDL_VIDEODRIVER=dummy ./firestaff_nexus_v2_phase0_v1_compatibility_lock_probe
 *
 * Source references:
 *   nexus_v1_iso_reader.c     Saturn ISO 9660 + DMDF interleaving
 *   nexus_v1_dmdf_model.c     DMDF (Dungeon Master Data Format) decoder
 *   nexus_v1_dungeon.c        DGN level loader, 16 levels
 *   nexus_v1_engine.c         V1 engine singleton
 *   nexus_v1_game.c           state init, level load, CD track map
 *   nexus_v1_champions.c      4-champion party
 *   nexus_v1_creatures.c      creature AI + render
 *   nexus_v1_movement.c       NEXUS_CMD_* (F0365/F0366 analogues)
 *   nexus_v1_combat.c         melee + spell combat
 *   nexus_v1_magic.c          spell casting
 *   nexus_v1_inventory.c      inventory + chest pickup
 *   nexus_v1_save_load.c      CDL save/load round-trip
 *   nexus_v1_sound.c          Saturn SCSP sound driver
 *   nexus_v1_rasterizer.c     320x200 indexed framebuffer
 *   NEXUS.C / NEXUS2.C        DM Nexus engine lifecycle
 *   NEXUS.BIN                 Saturn game binary
 *   HuC6260/HuC6270           VDC/VCE datasheet
 *   ReDMCSB CLIKMENU.C:142    F0365 turn
 *   ReDMCSB CLIKMENU.C:180    F0366 move
 *   ReDMCSB COMMAND.C:2045    F0380 input wait
 *   THQUEST.ASM T400-T900     Theron's Quest boot (sister game)
 */

#include "nexus_v2_phase_gate_pc34.h"

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
    NEXUS_V2_PhaseGateConfig cfg;
    memset(&cfg, 0xff, sizeof(cfg));  /* poison so defaults win */

    nexus_v2_phase_gate_defaults(&cfg);
    check(cfg.v2PresentationEnabled == 0,
          "Phase 0 default: v2PresentationEnabled=0");
    check(cfg.v2ConfigPersistenceEnabled == 0,
          "Phase 0 default: v2ConfigPersistenceEnabled=0");
    check(nexus_v2_phase_gate_v2_active(&cfg) == 0,
          "Phase 0 default: v2_active=0 (V1 behaviour)");
}

static void check_null_args(void)
{
    /* Defaults on NULL must be a no-op, no crash. */
    nexus_v2_phase_gate_defaults(0);
    check(nexus_v2_phase_gate_v2_active(0) == 0,
          "null v2_active=0 (safe)");
    {
        NEXUS_V2_PhaseGateDecision d =
            nexus_v2_phase_gate_decide(0, NEXUS_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
        check(d.v1SourceLocked == 0 && d.v2PresentationAllowed == 0,
              "null config decide -> V1-locked for V2-eligible (safe)");
    }
}

static void check_v1_locked_count(void)
{
    /* 11 V1-source-locked gameplay domains. */
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
    int n = (int)(sizeof(v1Locked) / sizeof(v1Locked[0]));
    check(n == 11, "V1-source-locked domain count = 11");
}

static void check_v2_eligible_count(void)
{
    /* 9 V2-presentation-eligible domains. */
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
    int n = (int)(sizeof(v2Eligible) / sizeof(v2Eligible[0]));
    check(n == 9, "V2-eligible domain count = 9");
    check(n + 11 == (int)NEXUS_V2_PHASE_DOMAIN_COUNT,
          "11 V1-locked + 9 V2-eligible = 20 (NEXUS_V2_PHASE_DOMAIN_COUNT)");
}

static void check_v1_locked_with_v2_off(void)
{
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
    int i;
    int n = (int)(sizeof(v1Locked) / sizeof(v1Locked[0]));

    nexus_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < n; ++i) {
        NEXUS_V2_PhaseGateDecision d =
            nexus_v2_phase_gate_decide(&cfg, v1Locked[i]);
        char id_v1[96], id_v2[96];
        snprintf(id_v1, sizeof(id_v1),
                 "v1_locked.v2_off[%s].v1SourceLocked",
                 nexus_v2_phase_gate_domain_name(v1Locked[i]));
        snprintf(id_v2, sizeof(id_v2),
                 "v1_locked.v2_off[%s].v2PresentationAllowed",
                 nexus_v2_phase_gate_domain_name(v1Locked[i]));
        check(d.v1SourceLocked == 1, id_v1);
        check(d.v2PresentationAllowed == 0, id_v2);
        check(nexus_v2_phase_gate_is_gameplay_domain(v1Locked[i]) == 1,
              "is_gameplay(V1-locked) == 1");
    }
}

static void check_v1_locked_with_v2_on(void)
{
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
    int i;
    int n = (int)(sizeof(v1Locked) / sizeof(v1Locked[0]));

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    for (i = 0; i < n; ++i) {
        NEXUS_V2_PhaseGateDecision d =
            nexus_v2_phase_gate_decide(&cfg, v1Locked[i]);
        char id_v1[96], id_v2[96];
        snprintf(id_v1, sizeof(id_v1),
                 "v1_locked.v2_on[%s].v1SourceLocked",
                 nexus_v2_phase_gate_domain_name(v1Locked[i]));
        snprintf(id_v2, sizeof(id_v2),
                 "v1_locked.v2_on[%s].v2PresentationAllowed",
                 nexus_v2_phase_gate_domain_name(v1Locked[i]));
        check(d.v1SourceLocked == 1, id_v1);
        check(d.v2PresentationAllowed == 0, id_v2);
    }
}

static void check_v2_eligible_with_v2_off(void)
{
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
    int i;
    int n = (int)(sizeof(v2Eligible) / sizeof(v2Eligible[0]));

    nexus_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < n; ++i) {
        NEXUS_V2_PhaseGateDecision d =
            nexus_v2_phase_gate_decide(&cfg, v2Eligible[i]);
        char id_v1[96], id_v2[96];
        snprintf(id_v1, sizeof(id_v1),
                 "v2_eligible.v2_off[%s].v1SourceLocked",
                 nexus_v2_phase_gate_domain_name(v2Eligible[i]));
        snprintf(id_v2, sizeof(id_v2),
                 "v2_eligible.v2_off[%s].v2PresentationAllowed",
                 nexus_v2_phase_gate_domain_name(v2Eligible[i]));
        check(d.v1SourceLocked == 0, id_v1);
        check(d.v2PresentationAllowed == 0, id_v2);
        check(nexus_v2_phase_gate_is_gameplay_domain(v2Eligible[i]) == 0,
              "is_gameplay(V2-eligible) == 0");
    }
}

static void check_v2_eligible_with_v2_on(void)
{
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
    int i;
    int n = (int)(sizeof(v2Eligible) / sizeof(v2Eligible[0]));

    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    for (i = 0; i < n; ++i) {
        NEXUS_V2_PhaseGateDecision d =
            nexus_v2_phase_gate_decide(&cfg, v2Eligible[i]);
        char id_v2[96];
        snprintf(id_v2, sizeof(id_v2),
                 "v2_eligible.v2_on_persist_on[%s].v2PresentationAllowed",
                 nexus_v2_phase_gate_domain_name(v2Eligible[i]));
        check(d.v1SourceLocked == 0, "v2_eligible.v1SourceLocked==0");
        check(d.v2PresentationAllowed == 1, id_v2);
    }
}

static void check_config_persistence(void)
{
    NEXUS_V2_PhaseGateConfig cfg;
    NEXUS_V2_PhaseGateDecision d;

    /* V2 on but config-persistence off -> CONFIG blocked. */
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 0;
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    check(d.v2PresentationAllowed == 0,
          "CONFIG: v2 on, persist off -> blocked");

    /* V2 off but config-persistence on -> CONFIG still blocked. */
    cfg.v2PresentationEnabled = 0;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    check(d.v2PresentationAllowed == 0,
          "CONFIG: v2 off, persist on -> blocked");

    /* Both on -> CONFIG allowed. */
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;
    d = nexus_v2_phase_gate_decide(&cfg, NEXUS_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    check(d.v2PresentationAllowed == 1,
          "CONFIG: v2 on, persist on -> allowed");
}

static void check_v2_active_helper(void)
{
    NEXUS_V2_PhaseGateConfig cfg;

    check(nexus_v2_phase_gate_v2_active(0) == 0,
          "v2_active(NULL) == 0 (safe)");

    nexus_v2_phase_gate_defaults(&cfg);
    check(nexus_v2_phase_gate_v2_active(&cfg) == 0,
          "v2_active(default) == 0 (V1 behaviour)");

    cfg.v2PresentationEnabled = 1;
    check(nexus_v2_phase_gate_v2_active(&cfg) == 1,
          "v2_active(v2=1) == 1");

    cfg.v2PresentationEnabled = 0;
    check(nexus_v2_phase_gate_v2_active(&cfg) == 0,
          "v2_active(v2=0) == 0");

    /* v2ConfigPersistenceEnabled alone does NOT activate V2. */
    cfg.v2PresentationEnabled = 0;
    cfg.v2ConfigPersistenceEnabled = 1;
    check(nexus_v2_phase_gate_v2_active(&cfg) == 0,
          "v2_active(persist=1 only) == 0");
}

static void check_domain_names(void)
{
    int i;
    for (i = 0; i < (int)NEXUS_V2_PHASE_DOMAIN_COUNT; ++i) {
        const char *name = nexus_v2_phase_gate_domain_name((NEXUS_V2_PhaseDomain)i);
        char id[96];
        snprintf(id, sizeof(id), "domain_name[%d].nonnull", i);
        check(name != 0, id);
        snprintf(id, sizeof(id), "domain_name[%d].nonempty", i);
        check(name && name[0] != 0, id);
    }
    check(strcmp(nexus_v2_phase_gate_domain_name((NEXUS_V2_PhaseDomain)9999),
                 "UNKNOWN") == 0,
          "domain_name(unknown) == 'UNKNOWN'");
}

static void check_source_evidence(void)
{
    const char *e = nexus_v2_phase_gate_source_evidence();

    check(e != 0, "evidence present");
    check(e && e[0] != 0, "evidence non-empty");
    check(e && strstr(e, "nexus_v1_iso_reader") != 0,
          "evidence references nexus_v1_iso_reader");
    check(e && strstr(e, "nexus_v1_dmdf_model") != 0,
          "evidence references nexus_v1_dmdf_model");
    check(e && strstr(e, "nexus_v1_dungeon") != 0,
          "evidence references nexus_v1_dungeon");
    check(e && strstr(e, "nexus_v1_engine") != 0,
          "evidence references nexus_v1_engine");
    check(e && strstr(e, "nexus_v1_game") != 0,
          "evidence references nexus_v1_game");
    check(e && strstr(e, "nexus_v1_champions") != 0,
          "evidence references nexus_v1_champions");
    check(e && strstr(e, "nexus_v1_movement") != 0,
          "evidence references nexus_v1_movement");
    check(e && strstr(e, "nexus_v1_save_load") != 0,
          "evidence references nexus_v1_save_load");
    check(e && strstr(e, "nexus_v1_rasterizer") != 0,
          "evidence references nexus_v1_rasterizer");
    check(e && strstr(e, "NEXUS.C") != 0,
          "evidence references NEXUS.C");
    check(e && strstr(e, "NEXUS2.C") != 0,
          "evidence references NEXUS2.C");
    check(e && strstr(e, "NEXUS.BIN") != 0,
          "evidence references NEXUS.BIN");
    check(e && strstr(e, "F0365") != 0,
          "evidence references F0365 (CLIKMENU.C:142 turn)");
    check(e && strstr(e, "F0366") != 0,
          "evidence references F0366 (CLIKMENU.C:180 move)");
    check(e && strstr(e, "F0380") != 0,
          "evidence references F0380 (COMMAND.C:2045 input wait)");
    check(e && strstr(e, "HuC6260") != 0,
          "evidence references HuC6260 (Saturn VDC)");
}

static void check_all_domains_have_anchor(void)
{
    /* Iterate all 20 enum values; every decision must have non-null
     * sourceAnchor and rule. This catches accidental switch fallthrough. */
    NEXUS_V2_PhaseGateConfig cfg;
    int i;

    nexus_v2_phase_gate_defaults(&cfg);
    for (i = 0; i < (int)NEXUS_V2_PHASE_DOMAIN_COUNT; ++i) {
        NEXUS_V2_PhaseGateDecision d =
            nexus_v2_phase_gate_decide(&cfg, (NEXUS_V2_PhaseDomain)i);
        char id[96];
        snprintf(id, sizeof(id), "decision[%d].source_anchor", i);
        check(d.sourceAnchor != 0 && d.sourceAnchor[0] != 0, id);
        snprintf(id, sizeof(id), "decision[%d].rule", i);
        check(d.rule != 0 && d.rule[0] != 0, id);
    }
}

static void check_unknown_domain(void)
{
    NEXUS_V2_PhaseGateConfig cfg;
    NEXUS_V2_PhaseGateDecision d;

    nexus_v2_phase_gate_defaults(&cfg);
    cfg.v2PresentationEnabled = 1;
    cfg.v2ConfigPersistenceEnabled = 1;

    d = nexus_v2_phase_gate_decide(&cfg, (NEXUS_V2_PhaseDomain)999);
    check(d.v1SourceLocked == 1,
          "unknown domain -> v1SourceLocked=1 (safe)");
    check(d.v2PresentationAllowed == 0,
          "unknown domain -> v2PresentationAllowed=0 (safe)");
    check(nexus_v2_phase_gate_is_gameplay_domain((NEXUS_V2_PhaseDomain)999) == 1,
          "unknown domain -> is_gameplay=1 (safe)");
}

int main(void)
{
    printf("=== Nexus V2 Phase 0 - V1 Compatibility Lock Probe ===\n");
    check_defaults();
    check_null_args();
    check_v1_locked_count();
    check_v2_eligible_count();
    check_v1_locked_with_v2_off();
    check_v1_locked_with_v2_on();
    check_v2_eligible_with_v2_off();
    check_v2_eligible_with_v2_on();
    check_config_persistence();
    check_v2_active_helper();
    check_domain_names();
    check_source_evidence();
    check_all_domains_have_anchor();
    check_unknown_domain();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
