/*
 * Core-only CSBWin resume must clear all Extended Features ownership.
 * Source: CSBWin SaveGame.cpp ReadExtendedFeatures/ReadDSAs before the
 * GAMEBLOCK load path; a save without that preamble owns no stale DSA index.
 */

#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures;

static void check(int condition, const char *message)
{
    if (condition) printf("PASS: %s\n", message);
    else { fprintf(stderr, "FAIL: %s\n", message); ++g_failures; }
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_CSBWin512BodyReport core_report;

    csb_v1_runtime_init(&profile, NULL);
    memset(&core_report, 0, sizeof(core_report));
    core_report.header_valid = 1;
    core_report.sections_verified = CSB_V1_CSBWIN_512_SECTION_COUNT;
    core_report.party_x = 4u;
    core_report.party_y = 5u;
    core_report.party_level = 2u;
    core_report.party_facing = 1u;

    profile.csbwin_extended_features_valid = 1;
    profile.csbwin_extended_features_version = 21u;
    profile.csbwin_extended_features_flags = 0x7fu;
    profile.csbwin_extended_features_flags32 = 0x12345678u;
    profile.csbwin_extended_cell_flag_array_size = 4096u;
    profile.csbwin_extended_level_index_present = 1;
    profile.csbwin_extended_level_dsa_index[2][3] = 77u;
    profile.csbwin_extended_game_info = malloc(4u);
    check(profile.csbwin_extended_game_info != NULL,
          "test owns prior extended game-info allocation");
    if (!profile.csbwin_extended_game_info) return 1;
    memcpy(profile.csbwin_extended_game_info, "old", 4u);
    profile.csbwin_extended_game_info_size = 3u;
    profile.csbwin_extended_game_info_fnv1a = 0xabcdef01u;

    check(csb_v1_runtime_apply_csbwin_resume_report(
              &profile, &core_report) == 0 &&
              profile.current_level == 2 && profile.party_x == 4 &&
              profile.party_y == 5 && profile.party_dir == 1,
          "core-only resume still stages the source GAMEBLOCK handoff");
    check(profile.csbwin_extended_features_valid == 0 &&
              profile.csbwin_extended_features_version == 0u &&
              profile.csbwin_extended_features_flags == 0u &&
              profile.csbwin_extended_features_flags32 == 0u &&
              profile.csbwin_extended_cell_flag_array_size == 0u &&
              profile.csbwin_extended_game_info == NULL &&
              profile.csbwin_extended_game_info_size == 0u &&
              profile.csbwin_extended_game_info_fnv1a == 0u &&
              profile.csbwin_extended_level_index_present == 0 &&
              profile.csbwin_extended_level_dsa_index[2][3] == 0xffffu &&
              profile.csbwin_extended_dsa_state.imported_action_count == 0,
          "core-only resume clears stale Extended Features and DSA ownership");

    csb_v1_runtime_cleanup(&profile);
    return g_failures == 0 ? 0 : 1;
}
