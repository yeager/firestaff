/*
 * test_dm2_v1_runtime_handoff_smoke.c
 *
 * Focused DM2 V1 runtime smoke gate.  This proves the boot/profile
 * handoff can enter the V1 runtime and advance exactly one deterministic
 * tick boundary.  It deliberately uses a synthetic verified profile and
 * does not claim real-asset launch, rendering, movement parity, or
 * end-to-end playability.
 *
 * Source-lock:
 *   DM2: SKULL.ASM T520/T560 are the documented party placement and
 *   dungeon tick anchors in dm2_v1_boot.c / dm2_v1_runtime.c.
 *   ReDMCSB GAMELOOP.C lines 55-70 shows the V1 loop boundary that
 *   advances timeline/tick work once per loop.
 *   ReDMCSB TOWNSGLB.H lines 1381-1388 documents party direction,
 *   party map position, and game-time globals used by the V1 lineage.
 */

#include "dm2_v1_boot.h"
#include "dm2_v1_game.h"
#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

static void make_synthetic_verified_profile(DM2_V1_BootProfile *profile)
{
    dm2_v1_boot_profile_init(profile);
    profile->assets_verified = 1;
    snprintf(profile->asset_root, sizeof(profile->asset_root),
             "synthetic-dm2-v1-runtime-handoff");
}

static void test_first_tick_after_boot_profile_handoff(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_GameState *state;
    DM2_V1_SessionState session;

    make_synthetic_verified_profile(&profile);
    CHECK(dm2_v1_boot_enter_game(&profile) == 0,
          "synthetic verified profile enters DM2 V1 game state");
    CHECK(profile.dm2_state != NULL,
          "boot handoff populates dm2_state");
    CHECK(profile.dungeon_data != NULL,
          "boot handoff owns a dungeon data handle even without real assets");

    state = (DM2_V1_GameState *)profile.dm2_state;
    CHECK(state->party_x == 15 && state->party_y == 15 &&
          state->party_dir == 0 && state->current_level == 0,
          "boot handoff starts at the documented DM2 V1 party snapshot");

    dm2_v1_runtime_init(&profile);
    CHECK(dm2_v1_runtime_get_tick_count() == 0,
          "runtime tick counter starts at zero after handoff bind");
    CHECK(dm2_v1_runtime_has_dungeon_data() == 1,
          "runtime bind exposes the boot handoff dungeon data");
    CHECK(dm2_v1_runtime_get_party_x() == 15 &&
          dm2_v1_runtime_get_party_y() == 15 &&
          dm2_v1_runtime_get_party_dir() == 0,
          "runtime accessors read the handoff party snapshot");

    memset(&session, 0, sizeof(session));
    dm2_v1_session_new(&session);
    session.game_tick = 77;
    session.party_level = 2;
    session.party_x = 19;
    session.party_y = 12;
    session.party_dir = 3;
    session.outdoor_mode = 1;
    session.time_of_day_minutes = 1080;
    session.rain_intensity = 64;
    CHECK(dm2_v1_runtime_apply_session(&session) == 0,
          "runtime accepts a bounded DM2 startup session after handoff");
    CHECK(state->party_x == 19 && state->party_y == 12 &&
          state->party_dir == 3 && state->current_level == 2 &&
          state->outdoor == 1,
          "session apply updates the boot-owned DM2 game state");
    CHECK(dm2_v1_runtime_get_tick_count() == 77 &&
          dm2_v1_runtime_get_party_x() == 19 &&
          dm2_v1_runtime_get_party_y() == 12 &&
          dm2_v1_runtime_get_party_dir() == 3,
          "session apply updates runtime tick and party accessors");
    CHECK(dm2_v1_runtime_get_weather() == DM2_WEATHER_RAIN &&
          dm2_v1_runtime_get_weather_intensity() == 64,
          "session apply updates runtime weather state");

    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_get_tick_count() == 78,
          "first deterministic DM2 V1 runtime tick is observable");
    CHECK(dm2_v1_runtime_get_party_x() == 19 &&
          dm2_v1_runtime_get_party_y() == 12 &&
          dm2_v1_runtime_get_party_dir() == 3,
          "first tick preserves the resumed snapped party state");

    dm2_v1_boot_cleanup(&profile);
}

int main(void)
{
    printf("=== DM2 V1 Runtime Handoff Smoke Gate ===\n\n");
    test_first_tick_after_boot_profile_handoff();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    if (failed == 0) {
        puts("ok: DM2 V1 boot/profile handoff reaches one deterministic runtime tick without claiming full playability");
        puts("sourceEvidence=SKULL.ASM T520/T560; ReDMCSB GAMELOOP.C lines 55-70; TOWNSGLB.H lines 1381-1388");
    }
    return failed == 0 ? 0 : 1;
}
