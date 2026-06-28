/*
 * M11 Nexus Light/Torch/Darkness spell dispatch gate.
 *
 * Source-lock:
 *   ReDMCSB MENU.C:1926-1942 dispatches Light / Magic Torch /
 *     Darkness into G0407_s_Party.MagicalLightAmount plus C70 light
 *     timeline events.
 *   ReDMCSB TIMELINE.C:487-555 F0238 preserves BUG0_18 silent-drop
 *     semantics when the light event budget is exhausted.
 *   ReDMCSB TIMELINE.C:1720-1767 F0257 owns the recursive weaker
 *     light-decay chain.
 *   ReDMCSB CHAMPION.C:27 owns MagicalLightAmount.
 *
 * Scope: data-free M11 dispatch only. No real Nexus DM.BIN, FONT256.S2D,
 * MNS, save bytes, or rendered Saturn screen are touched.
 */

#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

#define ASSERT_TRUE(cond, msg) do { \
    if (cond) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

static void seed_nexus_state(M11_GameViewState *state, int guard_rejects) {
    memset(state, 0, sizeof(*state));
    M11_GameView_Init(state);
    state->active = 1;
    state->sourceKind = M11_GAME_SOURCE_NEXUS_DGN;
    state->nexusState.tick_count = 123;
    nexus_v1_light_runtime_init(&state->nexusLightRuntime, guard_rejects);
    state->nexusLightRuntimeReady = 1;
}

static void enter_light_spell(M11_GameViewState *state) {
    ASSERT_EQ(M11_GameView_OpenSpellPanel(state), 1, "spell panel opens for Nexus Light");
    ASSERT_EQ(M11_GameView_EnterRune(state, 4), 1, "power rune entered");
    ASSERT_EQ(M11_GameView_EnterRune(state, 2), 1, "OH rune entered");
    ASSERT_EQ(M11_GameView_EnterRune(state, 3), 1, "IR rune entered");
    ASSERT_EQ(M11_GameView_EnterRune(state, 4), 1, "RA rune entered");
}

static void enter_torch_spell(M11_GameViewState *state) {
    ASSERT_EQ(M11_GameView_OpenSpellPanel(state), 1, "spell panel opens for Nexus Torch");
    ASSERT_EQ(M11_GameView_EnterRune(state, 0), 1, "LO rune entered");
    ASSERT_EQ(M11_GameView_EnterRune(state, 3), 1, "FUL rune entered");
}

static void enter_darkness_spell(M11_GameViewState *state) {
    ASSERT_EQ(M11_GameView_OpenSpellPanel(state), 1, "spell panel opens for Nexus Darkness");
    ASSERT_EQ(M11_GameView_EnterRune(state, 4), 1, "power rune entered");
    ASSERT_EQ(M11_GameView_EnterRune(state, 4), 1, "DES rune entered");
    ASSERT_EQ(M11_GameView_EnterRune(state, 3), 1, "IR rune entered");
    ASSERT_EQ(M11_GameView_EnterRune(state, 5), 1, "SAR rune entered");
}

static void test_m11_nexus_light_cast_drives_runtime(void) {
    M11_GameViewState state;
    seed_nexus_state(&state, 0);

    enter_light_spell(&state);
    ASSERT_EQ(M11_GameView_CastSpell(&state), 1, "Nexus Light cast consumed");
    ASSERT_EQ(state.spellPanelOpen, 0, "Nexus Light cast closes panel");
    ASSERT_EQ(state.spellBuffer.runeCount, 0, "Nexus Light cast clears runes");
    ASSERT_EQ(nexus_v1_light_runtime_total_casts_applied(&state.nexusLightRuntime),
              1, "Nexus Light increments applied counter");
    ASSERT_EQ(state.nexusLightRuntime.last_cast_kind, NEXUS_LIGHT_KIND_LIGHT,
              "last cast kind is LIGHT");
    ASSERT_EQ(state.nexusLightRuntime.last_cast_power_symbol_ordinal, 4,
              "last power ordinal preserved");
    ASSERT_EQ(nexus_v1_light_runtime_magical_light_amount(&state.nexusLightRuntime),
              68, "Light cast raises MagicalLightAmount through Nexus runtime");
    ASSERT_EQ(state.nexusLightRuntime.last_classification,
              NEXUS_LIGHT_OVERFLOW_NONE, "normal Light classifies NONE");

    M11_GameView_Shutdown(&state);
}

static void test_m11_nexus_torch_and_darkness_route_to_runtime(void) {
    M11_GameViewState state;
    seed_nexus_state(&state, 0);

    enter_torch_spell(&state);
    ASSERT_EQ(M11_GameView_CastSpell(&state), 1, "Nexus Torch cast consumed");
    ASSERT_EQ(state.nexusLightRuntime.last_cast_kind, NEXUS_LIGHT_KIND_TORCH,
              "last cast kind is TORCH");
    ASSERT_EQ(nexus_v1_light_runtime_magical_light_amount(&state.nexusLightRuntime),
              12, "Torch cast raises MagicalLightAmount by LightPower 2 amount");

    enter_darkness_spell(&state);
    ASSERT_EQ(M11_GameView_CastSpell(&state), 1, "Nexus Darkness cast consumed");
    ASSERT_EQ(state.nexusLightRuntime.last_cast_kind, NEXUS_LIGHT_KIND_DARKNESS,
              "last cast kind is DARKNESS");
    ASSERT_EQ(nexus_v1_light_runtime_magical_light_amount(&state.nexusLightRuntime),
              -28, "Darkness cast subtracts LightPower 5 amount from runtime total");
    ASSERT_EQ(state.nexusLightRuntime.last_classification,
              NEXUS_LIGHT_OVERFLOW_LIGHT_BLEED_NEGATIVE,
              "negative MagicalLightAmount classifies as light bleed");

    M11_GameView_Shutdown(&state);
}

static void test_m11_nexus_guard_mode_rejects_at_cap_without_mode_leak(void) {
    M11_GameViewState state;
    int32_t before;
    seed_nexus_state(&state, 1);

    while (state.nexusLightRuntime.timeline.active_count <
           NEXUS_V1_LIGHT_TIMELINE_BASE_CAP) {
        (void)nexus_v1_light_runtime_apply_cast(&state.nexusLightRuntime,
                                                NEXUS_LIGHT_KIND_LIGHT, 4);
    }
    before = nexus_v1_light_runtime_magical_light_amount(&state.nexusLightRuntime);

    enter_light_spell(&state);
    ASSERT_EQ(M11_GameView_CastSpell(&state), 1, "guarded Nexus Light input consumed");
    ASSERT_EQ(state.nexusLightRuntime.guard_rejects, 1, "M11 dispatch preserves guard mode");
    ASSERT_EQ(nexus_v1_light_runtime_magical_light_amount(&state.nexusLightRuntime),
              before, "guarded M11 cast does not mutate MagicalLightAmount");
    ASSERT_EQ(state.nexusLightRuntime.last_classification,
              NEXUS_LIGHT_OVERFLOW_CAST_REJECTED,
              "guarded M11 cast surfaces CAST_REJECTED");
    ASSERT_TRUE(strstr(state.inspectDetail, "CAST_REJECTED") != NULL,
                "inspect detail surfaces CAST_REJECTED");

    M11_GameView_Shutdown(&state);
}

static void test_m11_nexus_unknown_spell_is_readiness_gated(void) {
    M11_GameViewState state;
    seed_nexus_state(&state, 0);

    ASSERT_EQ(M11_GameView_OpenSpellPanel(&state), 1, "spell panel opens for unknown Nexus spell");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 0), 1, "LO rune entered for unknown");
    ASSERT_EQ(M11_GameView_EnterRune(&state, 0), 1, "YA rune entered for unknown");
    ASSERT_EQ(M11_GameView_CastSpell(&state), 1, "unknown Nexus spell cast input consumed");
    ASSERT_EQ(nexus_v1_light_runtime_total_casts_applied(&state.nexusLightRuntime),
              0, "unknown Nexus spell does not call apply_cast");
    ASSERT_EQ(nexus_v1_light_runtime_magical_light_amount(&state.nexusLightRuntime),
              0, "unknown Nexus spell leaves MagicalLightAmount untouched");
    ASSERT_TRUE(strstr(state.inspectDetail, "ONLY LIGHT, TORCH, AND DARKNESS") != NULL,
                "unknown spell reports readiness gate");

    M11_GameView_Shutdown(&state);
}

int main(void) {
    printf("=== M11 Nexus light spell dispatch gate ===\n");
    printf("ReDMCSB: MENU.C:1926-1942, TIMELINE.C:F0238/F0257, CHAMPION.C:27\n\n");

    test_m11_nexus_light_cast_drives_runtime();
    test_m11_nexus_torch_and_darkness_route_to_runtime();
    test_m11_nexus_guard_mode_rejects_at_cap_without_mode_leak();
    test_m11_nexus_unknown_spell_is_readiness_gated();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
