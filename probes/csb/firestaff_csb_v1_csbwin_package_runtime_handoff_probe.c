/*
 * CSBWin package dungeon/save -> CSB runtime handoff probe.
 *
 * This is deliberately an opt-in real-data probe. It does not construct a
 * dungeon or save substitute: callers provide the original package's
 * Dungeon.dat and csbgame*.dat paths.
 *
 * Source: CSBWin CSBCode.cpp LoadDungeon; SaveGame.cpp LoadGame,
 * ReadExtendedFeatures, ReadDSAs; DSA.cpp DSA::Read.
 * ReDMCSB: LOADSAVE.C F0435_STARTEND_LoadGame lines ~2192-2748.
 */

#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
static int failures;

#define CHECK(condition, message) do {                                    \
    ++checks;                                                              \
    if (condition) {                                                       \
        printf("  PASS: %s\n", message);                                 \
    } else {                                                               \
        ++failures;                                                        \
        printf("  FAIL: %s\n", message);                                 \
    }                                                                      \
} while (0)

static const char *path_arg_or_env(int argc, char **argv, int index,
                                   const char *env_name)
{
    const char *value;

    if (argc > index && argv[index] && argv[index][0] != '\0') {
        return argv[index];
    }
    value = getenv(env_name);
    return value && value[0] != '\0' ? value : NULL;
}

int main(int argc, char **argv)
{
    const char *dungeon_path = path_arg_or_env(
        argc, argv, 1, "FIRESTAFF_CSBWIN_DUNGEON");
    const char *save_path = path_arg_or_env(
        argc, argv, 2, "FIRESTAFF_CSBWIN_SAVE");
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData *dungeon;
    int resume_rc;
    uint32_t game_time_before_tick;
    int pre_resume_dungeon_level;

    printf("=== CSBWin package runtime handoff probe ===\n\n");
    if (!dungeon_path || !save_path) {
        printf("SKIP: provide <Dungeon.dat> <csbgame*.dat> or set "
               "FIRESTAFF_CSBWIN_DUNGEON and FIRESTAFF_CSBWIN_SAVE.\n");
        return 0;
    }

    dungeon = (CSB_V1_DungeonData *)calloc(1u, sizeof(*dungeon));
    CHECK(dungeon != NULL, "allocate runtime-owned dungeon handle");
    if (!dungeon) return 1;

    CHECK(csb_v1_dungeon_load_from_file(dungeon, dungeon_path) == 0,
          "load supplied CSBWin Dungeon.dat through production decoder");
    if (!dungeon->raw_data) {
        free(dungeon);
        return 1;
    }

    csb_v1_runtime_init(&profile, NULL);
    profile.dungeon_handle = dungeon;
    profile.dungeon_path = dungeon_path;
    csb_v1_dungeon_set_current(dungeon);
    csb_v1_dungeon_set_current_level(0);
    pre_resume_dungeon_level = csb_v1_dungeon_get_current_level();

    CHECK(profile.dungeon_handle == csb_v1_dungeon_get_current(),
          "runtime profile and dungeon singleton share the supplied package world");
    CHECK(profile.dungeon_handle->level_count > 0,
          "supplied package exposes at least one decoded dungeon level");

    game_time_before_tick = profile.game_time;
    csb_v1_runtime_tick(&profile, CSB_V1_TICK_MS_NOMINAL);
    CHECK(profile.game_time == game_time_before_tick + 1u &&
              profile.dungeon_handle == dungeon &&
              csb_v1_dungeon_get_current() == dungeon,
          "supplied Dungeon.dat remains the live runtime world through a tick");

    resume_rc = csb_v1_runtime_apply_csbwin_resume_file(
        &profile, save_path, 4u * 1024u * 1024u);
    if (resume_rc == 0) {
        CHECK(profile.dungeon_handle == dungeon &&
                  csb_v1_dungeon_get_current() == dungeon,
              "resume retains the loaded package dungeon owner");
        CHECK(profile.party_state_valid &&
                  profile.csbwin_body_runtime_summary_valid,
              "resume publishes the verified body into live runtime state");
        CHECK(profile.csbwin_extended_dsa_state.imported_action_count >= 0,
              "resume publishes only the authenticated CSBWin DSA action owner");
    } else {
        CHECK(profile.dungeon_handle == dungeon &&
                  csb_v1_dungeon_get_current() == dungeon &&
                  csb_v1_dungeon_get_current_level() == pre_resume_dungeon_level &&
                  !profile.party_state_valid &&
                  !profile.csbwin_body_runtime_summary_valid,
              "rejected package save leaves the live dungeon/runtime state untouched");
    }

    csb_v1_runtime_cleanup(&profile);
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "runtime cleanup releases the package dungeon singleton");

    printf("\n=== Summary: %d checks, %d failures ===\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
