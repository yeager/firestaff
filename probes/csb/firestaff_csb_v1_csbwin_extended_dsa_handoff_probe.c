/*
 * CSBWin extended-save DSA -> live CSB runtime provenance probe.
 *
 * This is deliberately an opt-in real-package probe. It does not create a
 * save, DSA record, level index, or fallback action: callers provide the
 * original package's Dungeon.dat and a CSBWin extended save.
 *
 * Source: CSBWin SaveGame.cpp ReadExtendedFeatures/ReadDSAs/ReadDSALevelIndex
 * and DSA.cpp DSA::Read/DSAState::Program.
 * ReDMCSB: LOADSAVE.C F0435_STARTEND_LoadGame lines ~2192-2748.
 */

#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

static int checks;
static int failures;

#define CHECK(condition, message) do {                                    \
    ++checks;                                                              \
    if (condition) {                                                       \
        printf("  PASS: %s\n", message);                                  \
    } else {                                                               \
        ++failures;                                                        \
        printf("  FAIL: %s\n", message);                                  \
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

static unsigned long action_fingerprint(const CSB_V1_DSAImportedAction *action)
{
    unsigned long hash = 2166136261u;
    int i;

    if (!action) return 0u;
    hash = (hash ^ action->dsa_id) * 16777619u;
    hash = (hash ^ action->state_index) * 16777619u;
    hash = (hash ^ action->column) * 16777619u;
    hash = (hash ^ (unsigned long)action->program_word_count) * 16777619u;
    for (i = 0; i < action->program_word_count; ++i) {
        hash = (hash ^ action->program_words[i]) * 16777619u;
    }
    return hash;
}

static int action_is_first_for_column(const CSB_V1_ChaosMagicState *state,
                                      int action_index)
{
    const CSB_V1_DSAImportedAction *action;
    int i;

    if (!state || action_index < 0 || action_index >= state->imported_action_count) {
        return 0;
    }
    action = &state->imported_actions[action_index];
    for (i = 0; i < action_index; ++i) {
        const CSB_V1_DSAImportedAction *prior = &state->imported_actions[i];
        if (prior->dsa_id == action->dsa_id &&
            prior->state_index == action->state_index &&
            prior->column == action->column) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv)
{
    const char *dungeon_path = path_arg_or_env(
        argc, argv, 1, "FIRESTAFF_CSBWIN_DUNGEON");
    const char *save_path = path_arg_or_env(
        argc, argv, 2, "FIRESTAFF_CSBWIN_SAVE");
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData *dungeon;
    const CSB_V1_ChaosMagicState *state;
    const CSB_V1_DSAImportedAction *first_action;
    const CSB_V1_DSAImportedAction *first_action_after_tick;
    unsigned long first_action_hash;
    int mapped_entries = 0;
    int mapped_actions = 0;
    int i;
    int level;
    int selector;

    printf("=== CSBWin extended DSA runtime handoff probe ===\n\n");
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

    CHECK(csb_v1_runtime_apply_csbwin_resume_file(
              &profile, save_path, 4u * 1024u * 1024u) == 0,
          "admit supplied CSBWin save through production resume path");
    state = &profile.csbwin_extended_dsa_state;
    CHECK(profile.csbwin_extended_features_valid,
          "resume publishes CSBWin Extended Features into runtime state");
    CHECK(profile.csbwin_extended_level_index_present,
          "resume publishes the source DSA level-index table");
    CHECK(state->imported_actions != NULL && state->imported_action_count > 0,
          "resume retains authenticated source DSA actions");

    if (profile.csbwin_extended_features_valid &&
        profile.csbwin_extended_level_index_present &&
        state->imported_actions && state->imported_action_count > 0) {
        for (i = 0; i < state->imported_action_count; ++i) {
            const CSB_V1_DSAImportedAction *action = &state->imported_actions[i];
            const CSB_V1_DSAImportedAction *resolved = NULL;

            if (action->program_word_count > 0) {
                CHECK(action->program_words != NULL,
                      "non-empty source action owns its decoded words");
            }
            CHECK(state->imported_headers[action->dsa_id].valid,
                  "source action retains an authenticated DSA header");
            if (action_is_first_for_column(state, i)) {
                resolved = csb_v1_chaos_find_imported_action_column(
                    state, action->dsa_id, action->state_index, action->column);
                CHECK(resolved == action,
                      "source action column resolves to its runtime-owned pointer");
            }
        }

        for (level = 0; level < 64; ++level) {
            for (selector = 0; selector < 32; ++selector) {
                uint16_t dsa_id = profile.csbwin_extended_level_dsa_index[level][selector];
                if (dsa_id == 0xffffu) continue;
                ++mapped_entries;
                if (dsa_id < CSB_V1_MAX_DSA_SCRIPTS &&
                    state->imported_headers[dsa_id].valid) {
                    for (i = 0; i < state->imported_action_count; ++i) {
                        if (state->imported_actions[i].dsa_id == (uint8_t)dsa_id) {
                            ++mapped_actions;
                            break;
                        }
                    }
                }
            }
        }
        CHECK(mapped_entries > 0,
              "source DSA level index contains at least one selector entry");
        CHECK(mapped_actions > 0,
              "a source DSA selector reaches an authenticated runtime action");

        first_action = &state->imported_actions[0];
        first_action_hash = action_fingerprint(first_action);
        csb_v1_runtime_tick(&profile, CSB_V1_TICK_MS_NOMINAL);
        first_action_after_tick = &profile.csbwin_extended_dsa_state.imported_actions[0];
        CHECK(first_action_after_tick == first_action &&
                  action_fingerprint(first_action_after_tick) == first_action_hash,
              "runtime tick preserves the admitted source DSA action owner");
    }

    csb_v1_runtime_cleanup(&profile);
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "runtime cleanup releases the package dungeon singleton");

    printf("\n=== Summary: %d checks, %d failures ===\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
