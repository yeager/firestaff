#include "dm1_v2_presentation_profile_pc34.h"

#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed: %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static int test_defaults_keep_v1_gameplay_and_v1_presentation(void) {
    DM1_V2_PresentationProfile profile;

    dm1_v2_presentation_profile_defaults(&profile);

    /* Source-lock: GAMELOOP.C:164-219 and COMMAND.C:2045-2155 own gameplay
     * command dispatch; Phase 1 defaults must keep V1/off as the boot path. */
    CHECK(dm1_v2_presentation_profile_uses_v1_gameplay(&profile));
    CHECK(profile.gameplayRoute == DM1_V2_GAMEPLAY_ROUTE_V1_SOURCE);
    CHECK(profile.presentationMode == DM1_V2_PRESENTATION_MODE_V1_ORIGINAL);
    CHECK(profile.v2PresentationEnabled == 0);
    CHECK(profile.settings.scalePercent == 100);
    CHECK(profile.settings.aspectMode == DM1_V2_ASPECT_ORIGINAL_4_3);
    CHECK(profile.presentationFrameCount == 0U);
    CHECK(profile.presentationDirty == 0);
    return 0;
}

static int test_m12_config_enables_v2_presentation_without_rerouting_gameplay(void) {
    M12_Config cfg;
    DM1_V2_PresentationProfile profile;

    M12_Config_SetDefaults(&cfg);
    cfg.graphicsIndex = 1;
    cfg.dm1V2ScalePercent = 250;
    cfg.dm1V2SmoothingEnabled = 0;
    cfg.dm1V2DynamicLightingEnabled = 0;
    cfg.dm1V2AccessibilityTouchEnabled = 1;
    cfg.dm1V2AspectMode = 1;

    dm1_v2_presentation_profile_from_m12_config(&profile, &cfg);

    CHECK(dm1_v2_presentation_profile_uses_v1_gameplay(&profile));
    CHECK(profile.gameplayRoute == DM1_V2_GAMEPLAY_ROUTE_V1_SOURCE);
    CHECK(profile.presentationMode == DM1_V2_PRESENTATION_MODE_V2_SHELL);
    CHECK(profile.v2PresentationEnabled == 1);
    CHECK(profile.settings.scalePercent == 250);
    CHECK(profile.settings.smoothingEnabled == 0);
    CHECK(profile.settings.dynamicLightingEnabled == 0);
    CHECK(profile.settings.accessibilityTouchEnabled == 1);
    CHECK(profile.settings.aspectMode == DM1_V2_ASPECT_WIDESCREEN_16_9);

    /* V2/on affects presentation/config only; it must not rewrite the source
     * config route into a gameplay route or mutate the caller-owned config. */
    CHECK(cfg.graphicsIndex == 1);
    CHECK(cfg.dm1V2ScalePercent == 250);
    return 0;
}

static int test_default_m12_config_remains_v1_off_even_with_v2_settings(void) {
    M12_Config cfg;
    DM1_V2_PresentationProfile profile;

    M12_Config_SetDefaults(&cfg);
    cfg.graphicsIndex = 0;
    cfg.dm1V2ScalePercent = 400;
    cfg.dm1V2AspectMode = 1;

    dm1_v2_presentation_profile_from_m12_config(&profile, &cfg);

    CHECK(dm1_v2_presentation_profile_uses_v1_gameplay(&profile));
    CHECK(profile.presentationMode == DM1_V2_PRESENTATION_MODE_V1_ORIGINAL);
    CHECK(profile.v2PresentationEnabled == 0);
    CHECK(profile.settings.scalePercent == 400);
    CHECK(profile.settings.aspectMode == DM1_V2_ASPECT_WIDESCREEN_16_9);
    return 0;
}

static int test_snapshot_binding_copies_gameplay_state_without_mutating_source(void) {
    DM1_V2_PresentationProfile profile;
    DM1_V2_GameplaySnapshot snapshot;

    dm1_v2_presentation_profile_defaults(&profile);
    dm1_v2_gameplay_snapshot_init(&snapshot, 4, 7, 6, 12345U);
    snapshot.disabledMovementTicks = 9;
    snapshot.projectileDisabledMovementTicks = 3;

    dm1_v2_presentation_profile_bind_snapshot(&profile, &snapshot);

    /* Source-lock: GAMELOOP.C:90 and DUNVIEW.C:8318-8338 draw from V1
     * direction/map state. The V2 profile stores a normalized copy for
     * presentation; the input snapshot remains caller-owned gameplay state. */
    CHECK(profile.snapshot.mapX == 4);
    CHECK(profile.snapshot.mapY == 7);
    CHECK(profile.snapshot.facingDir == 2);
    CHECK(profile.snapshot.gameTime == 12345U);
    CHECK(profile.snapshot.disabledMovementTicks == 9);
    CHECK(profile.snapshot.projectileDisabledMovementTicks == 3);
    CHECK(snapshot.facingDir == 2);
    CHECK(profile.presentationDirty == 1);

    snapshot.mapX = 99;
    snapshot.gameTime = 999U;
    CHECK(profile.snapshot.mapX == 4);
    CHECK(profile.snapshot.gameTime == 12345U);
    return 0;
}

static int test_presentation_tick_does_not_advance_gameplay_snapshot(void) {
    DM1_V2_PresentationProfile profile;
    DM1_V2_GameplaySnapshot before;

    dm1_v2_presentation_profile_defaults(&profile);
    dm1_v2_gameplay_snapshot_init(&profile.snapshot, 12, 13, 1, 77U);
    dm1_v2_presentation_profile_set_enabled(&profile, 1);
    before = profile.snapshot;

    dm1_v2_presentation_profile_tick(&profile);
    dm1_v2_presentation_profile_tick(&profile);

    CHECK(profile.presentationFrameCount == 2U);
    CHECK(profile.presentationDirty == 0);
    CHECK(memcmp(&profile.snapshot, &before, sizeof(before)) == 0);
    CHECK(dm1_v2_presentation_profile_uses_v1_gameplay(&profile));
    return 0;
}

static int test_source_evidence_string_names_required_anchors(void) {
    const char* evidence = dm1_v2_presentation_profile_source_evidence();

    CHECK(strstr(evidence, "GAMELOOP.C:90") != NULL);
    CHECK(strstr(evidence, "GAMELOOP.C:164-219") != NULL);
    CHECK(strstr(evidence, "DUNVIEW.C:2999-3000") != NULL);
    CHECK(strstr(evidence, "DUNVIEW.C:8318-8338") != NULL);
    CHECK(strstr(evidence, "COMMAND.C:2045-2155") != NULL);
    return 0;
}

int main(void) {
    if (test_defaults_keep_v1_gameplay_and_v1_presentation()) return 1;
    if (test_m12_config_enables_v2_presentation_without_rerouting_gameplay()) return 1;
    if (test_default_m12_config_remains_v1_off_even_with_v2_settings()) return 1;
    if (test_snapshot_binding_copies_gameplay_state_without_mutating_source()) return 1;
    if (test_presentation_tick_does_not_advance_gameplay_snapshot()) return 1;
    if (test_source_evidence_string_names_required_anchors()) return 1;
    puts("dm1_v2_presentation_profile_pc34: ok");
    return 0;
}
