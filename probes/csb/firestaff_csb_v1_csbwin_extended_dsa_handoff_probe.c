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
#include "csb_v1_csbwin_save_loader_boundary_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct {
    size_t size;
    uint32_t fnv1a;
} SourceFileReceipt;

/* Keep the supplied package pair stable for the entire probe. The runtime
 * owns decoded copies after admission, but a changed source path must not be
 * able to inherit that earlier DSA/timer result. */
static int source_file_receipt(const char *path, SourceFileReceipt *out)
{
    uint8_t buffer[4096];
    FILE *fp;
    size_t total = 0u;
    uint32_t hash = 2166136261u;
    size_t count;

    if (!path || !out) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    while ((count = fread(buffer, 1u, sizeof(buffer), fp)) != 0u) {
        size_t i;

        if (total > (size_t)-1 - count) {
            fclose(fp);
            return 0;
        }
        total += count;
        for (i = 0u; i < count; ++i) {
            hash = (hash ^ buffer[i]) * 16777619u;
        }
    }
    {
        const int had_error = ferror(fp);
        const int close_error = fclose(fp);

        if (had_error || close_error != 0) return 0;
    }
    out->size = total;
    out->fnv1a = hash;
    return 1;
}

static int source_file_receipt_matches(const char *path,
                                       const SourceFileReceipt *expected)
{
    SourceFileReceipt current;

    return expected && source_file_receipt(path, &current) &&
           current.size == expected->size && current.fnv1a == expected->fnv1a;
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

static uint32_t tail_fingerprint(const uint8_t *bytes, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!bytes || size == 0u) return 0u;
    for (i = 0u; i < size; ++i) {
        hash = (hash ^ bytes[i]) * 16777619u;
    }
    return hash;
}

static int extended_dsa_tail_is_current(const CSB_V1_RuntimeProfile *profile)
{
    return profile && profile->csbwin_extended_features_valid &&
           profile->csbwin_appended_tail_valid &&
           !profile->csbwin_appended_tail_truncated &&
           profile->csbwin_appended_tail_size != 0u &&
           profile->csbwin_appended_tail_size ==
               profile->csbwin_appended_tail_preserved_size &&
           profile->csbwin_appended_tail_fnv1a == tail_fingerprint(
               profile->csbwin_appended_tail,
               profile->csbwin_appended_tail_preserved_size);
}

/* A positive runtime result must identify the exact TimerQueue entry and the
 * exact imported action it executed. This is intentionally not a prediction:
 * the receipt is written only after the production tick reaches a complete
 * source-owned ProcessDSATimer5/6 pure-stack action. */
static int saved_timer_dsa_execution_is_authenticated(
    const CSB_V1_RuntimeProfile *profile)
{
    const CSB_V1_ChaosMagicState *state;
    const CSB_V1_DSAImportedAction *action;
    uint16_t queue_slot;
    uint16_t timer_index;
    int action_ordinal;

    if (!profile || !profile->csbwin_last_saved_timer_dsa_valid ||
        !profile->csbwin_body_runtime_summary_valid) {
        return 0;
    }
    queue_slot = profile->csbwin_last_saved_timer_dsa_queue_slot;
    timer_index = profile->csbwin_last_saved_timer_dsa_timer_index;
    action_ordinal = profile->csbwin_last_saved_timer_dsa_action_ordinal;
    state = &profile->csbwin_extended_dsa_state;
    if (queue_slot >= profile->csbwin_timer_queue_summary_count ||
        timer_index >= profile->csbwin_timer_summary_count ||
        profile->csbwin_timer_queue[queue_slot] != timer_index ||
        action_ordinal < 0 || action_ordinal >= state->imported_action_count ||
        !state->imported_actions) {
        return 0;
    }
    action = &state->imported_actions[action_ordinal];
    return action->dsa_id == profile->csbwin_last_saved_timer_dsa_id &&
           action->state_index ==
               profile->csbwin_last_saved_timer_dsa_state_index &&
           action->column == profile->csbwin_last_saved_timer_dsa_column &&
           state->imported_headers[action->dsa_id].valid;
}

static int action_is_first_for_column(const CSB_V1_ChaosMagicState *state,
                                      int action_index);

/* DSA.cpp keeps the decoded action table owned by the loaded save. A runtime
 * tick may consume a queued timer, but it must neither replace that catalog
 * nor redirect a source selector to a synthesized action. */
static int action_catalog_survives_tick(
    const CSB_V1_ChaosMagicState *state,
    const CSB_V1_DSAImportedAction *const *before_actions,
    const unsigned long *before_hashes,
    int action_count)
{
    int i;

    if (!state || !before_actions || !before_hashes ||
        action_count <= 0 || state->imported_action_count != action_count) {
        return 0;
    }
    for (i = 0; i < action_count; ++i) {
        const CSB_V1_DSAImportedAction *action = &state->imported_actions[i];

        if (action != before_actions[i] ||
            action_fingerprint(action) != before_hashes[i]) {
            return 0;
        }
        if (action_is_first_for_column(state, i) &&
            csb_v1_chaos_find_imported_action_column(
                state, action->dsa_id, action->state_index, action->column) != action) {
            return 0;
        }
    }
    return 1;
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

static int action_ordinal_in_state(const CSB_V1_ChaosMagicState *state,
                                   int action_index)
{
    const CSB_V1_DSAImportedAction *action;
    int i;
    int ordinal = 0;

    if (!state || action_index < 0 ||
        action_index >= state->imported_action_count) {
        return -1;
    }
    action = &state->imported_actions[action_index];
    for (i = 0; i < action_index; ++i) {
        const CSB_V1_DSAImportedAction *prior = &state->imported_actions[i];
        if (prior->dsa_id == action->dsa_id &&
            prior->state_index == action->state_index) {
            ++ordinal;
        }
    }
    return ordinal;
}

/* Walk decoded source Thing chains.  A binding is useful only when the
 * original package contains a type-47 actuator whose saved selector reaches
 * one of the actions admitted from this exact save. */
static int find_source_dsa_binding(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_DungeonData *dungeon,
    CSB_V1_RuntimeDSAFilterBinding *out_binding)
{
    int level;

    if (!profile || !dungeon || !out_binding) return 0;
    for (level = 0; level < dungeon->level_count; ++level) {
        int x;

        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            int y;

            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                int thing = csb_v1_dungeon_get_first_thing(dungeon, level, x, y);
                int remaining = 0;
                int type;

                for (type = 0; type < 16; ++type) {
                    remaining += dungeon->thing_type_counts[type];
                }
                while (thing >= 0 && thing != 0xfffe && remaining-- > 0) {
                    const uint8_t *record;
                    CSB_V1_DSAFilterLocation location;
                    int record_type;
                    int record_size;

                    record = csb_v1_dungeon_get_thing_record(
                        dungeon, (uint16_t)thing, &record_type, NULL, &record_size);
                    if (!record || record_size < 4) break;
                    if (record_type == CSB_V1_THING_TYPE_ACTUATOR &&
                        (((uint16_t)record[2] | ((uint16_t)record[3] << 8)) &
                         0x007fu) == CSB_V1_DSA_FILTER_ACTUATOR_TYPE) {
                        location.level = level;
                        location.x = x;
                        location.y = y;
                        location.position = 0;
                        location.party_level_only = 0;
                        location.max_distance = 0;
                        location.actuator_thing = (uint16_t)thing;
                        if (csb_v1_runtime_resolve_csbwin_dsa_filter_binding(
                                profile, dungeon, &location, out_binding)) {
                            return 1;
                        }
                    }
                    thing = (int)((uint16_t)record[0] |
                                  ((uint16_t)record[1] << 8));
                }
            }
        }
    }
    return 0;
}

/* The saved TIMER heap is materialized before a DSA can receive a timer
 * dispatch. Prove the live timeline still identifies each event by its exact
 * serialized queue slot and fields; do not infer a dispatch or action. */
static int saved_timer_queue_matches_runtime(
    const CSB_V1_RuntimeProfile *profile)
{
    uint16_t queue_slot;

    if (!profile || !profile->csbwin_body_runtime_summary_valid ||
        profile->csbwin_timer_queue_summary_count == 0u ||
        profile->csbwin_timer_queue_summary_count !=
            profile->csbwin_timer_summary_count ||
        profile->timeline_queue.eventCount !=
            (int)profile->csbwin_timer_queue_summary_count) {
        return 0;
    }
    for (queue_slot = 0u;
         queue_slot < profile->csbwin_timer_queue_summary_count;
         ++queue_slot) {
        const uint16_t timer_index = profile->csbwin_timer_queue[queue_slot];
        const CSB_V1_CSBWin512TimerSummary *timer;
        int event_index;
        int matched = 0;

        if (timer_index >= profile->csbwin_timer_summary_count) return 0;
        timer = &profile->csbwin_timers[timer_index];
        if (!timer->valid || timer->source_index != timer_index) return 0;
        for (event_index = 0;
             event_index < profile->timeline_queue.maxEvents;
             ++event_index) {
            const struct DM1_Event_V1 *event;

            if (profile->csbwin_timeline_event_queue_slot[event_index] !=
                queue_slot) {
                continue;
            }
            event = &profile->timeline_queue.events[event_index];
            if (event->map_time != timer->time ||
                event->type != timer->function ||
                event->priority != timer->ubyte5 ||
                event->b_mapX != timer->ubyte6 ||
                event->b_mapY != timer->ubyte7 ||
                event->c_cell != timer->ubyte8 ||
                event->c_effect != timer->ubyte9) {
                return 0;
            }
            if (++matched != 1) return 0;
        }
        if (matched != 1) return 0;
    }
    return 1;
}

/* A source timer may be consumed or requeued by the runtime tick, so the
 * complete serialized heap need not still be live afterwards. Every event
 * that does remain live must nevertheless retain an exact saved TimerQueue
 * slot and TIMER field receipt. This rejects a generic M10 replacement event
 * being introduced beside the real package queue. */
static int remaining_saved_timer_queue_matches_runtime(
    const CSB_V1_RuntimeProfile *profile)
{
    uint8_t seen[CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES] = { 0 };
    int event_ordinal;

    if (!profile || !profile->csbwin_body_runtime_summary_valid ||
        profile->csbwin_timer_queue_summary_count !=
            profile->csbwin_timer_summary_count ||
        profile->timeline_queue.eventCount < 0 ||
        profile->timeline_queue.eventCount > DM1_EVENT_MAX_COUNT) {
        return 0;
    }
    for (event_ordinal = 0;
         event_ordinal < profile->timeline_queue.eventCount;
         ++event_ordinal) {
        const int event_index = profile->timeline_queue.timeline[event_ordinal];
        const struct DM1_Event_V1 *event;
        const CSB_V1_CSBWin512TimerSummary *timer;
        uint16_t queue_slot;
        uint16_t timer_index;

        if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) return 0;
        queue_slot = profile->csbwin_timeline_event_queue_slot[event_index];
        if (queue_slot == CSB_V1_CSBWIN_TIMER_QUEUE_NONE) {
            continue;
        }
        if (queue_slot >= profile->csbwin_timer_queue_summary_count ||
            seen[queue_slot]) {
            return 0;
        }
        timer_index = profile->csbwin_timer_queue[queue_slot];
        if (timer_index >= profile->csbwin_timer_summary_count) return 0;
        timer = &profile->csbwin_timers[timer_index];
        event = &profile->timeline_queue.events[event_index];
        if (!timer->valid || timer->truncated ||
            timer->source_index != timer_index ||
            event->map_time != timer->time || event->type != timer->function ||
            event->priority != timer->ubyte5 ||
            event->b_mapX != timer->ubyte6 ||
            event->b_mapY != timer->ubyte7 ||
            event->c_cell != timer->ubyte8 || event->c_effect != timer->ubyte9) {
            return 0;
        }
        seen[queue_slot] = 1u;
    }
    return 1;
}

/* A live Extended Features package may no longer have an exportable core
 * after a timer fires: Firestaff deliberately refuses to invent CSBWin's
 * free-list/requeue state. When the unchanged source-owned heap remains
 * exportable, prove the core bytes through the production verifier and a
 * separate core-only resume. That resume must not inherit DSA metadata from
 * the extended package. */
static int exported_core_reloads_after_tick(
    const CSB_V1_RuntimeProfile *profile)
{
    uint8_t core_bytes[65536];
    size_t core_size = 0u;
    CSB_V1_CSBWin512BodyReport core_report;
    CSB_V1_RuntimeProfile core_runtime;

    if (!profile) return 0;
    if (csb_v1_runtime_export_csbwin_core_save_to_memory(
            profile, core_bytes, sizeof(core_bytes), &core_size) != 0) {
        return -1;
    }
    memset(&core_report, 0, sizeof(core_report));
    if (csb_v1_csbwin_512_verify_save_body(
            core_bytes, core_size, 0u, &core_report) != CSB_V1_CSBWIN_512_OK) {
        return 0;
    }
    csb_v1_runtime_init(&core_runtime, NULL);
    if (csb_v1_runtime_apply_csbwin_resume_report(
            &core_runtime, &core_report) != 0 ||
        core_runtime.csbwin_extended_features_valid ||
        core_runtime.csbwin_extended_level_index_present ||
        core_runtime.csbwin_extended_dsa_state.imported_action_count != 0 ||
        !saved_timer_queue_matches_runtime(&core_runtime) ||
        core_runtime.game_time != profile->game_time ||
        core_runtime.party_x != profile->party_x ||
        core_runtime.party_y != profile->party_y ||
        core_runtime.party_state.PartyDirection !=
            profile->party_state.PartyDirection) {
        return 0;
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
    CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 chain_receipt;
    CSB_V1_CSBWinDSARuntimeExecutionReceipt_PC34 execution_receipt;
    CSB_V1_DungeonData *dungeon;
    const CSB_V1_ChaosMagicState *state;
    const CSB_V1_DSAImportedAction **actions_before_tick = NULL;
    unsigned long *action_hashes_before_tick = NULL;
    CSB_V1_RuntimeDSAFilterBinding source_binding;
    CSB_V1_CSBWinDSASaveCorpusReceipt dsa_corpus;
    SourceFileReceipt dungeon_receipt;
    SourceFileReceipt save_receipt;
    int mapped_entries = 0;
    int mapped_actions = 0;
    int verified_core_actions = 0;
    int verified_transfer_actions = 0;
    int verified_runtime_owner_actions = 0;
    int verified_conditional_actions = 0;
    int verified_arithmetic_actions = 0;
    int verified_variable_actions = 0;
    int verified_timer_actions = 0;
    int verified_message_actions = 0;
    int verified_teleporter_copy_actions = 0;
    int verified_actuator_copy_actions = 0;
    int verified_dungeon_mutation_actions = 0;
    int source_binding_found;
    int i;
    int level;
    int selector;

    printf("=== CSBWin extended DSA runtime handoff probe ===\n\n");
    if (!dungeon_path || !save_path) {
        printf("SKIP: provide <Dungeon.dat> <csbgame*.dat> or set "
               "FIRESTAFF_CSBWIN_DUNGEON and FIRESTAFF_CSBWIN_SAVE.\n");
        return 0;
    }

    CHECK(source_file_receipt(dungeon_path, &dungeon_receipt) &&
              source_file_receipt(save_path, &save_receipt),
          "snapshot supplied Dungeon.dat and csbgame bytes before admission");
    if (failures != 0) return 1;

    /* CSBWin SaveGame.cpp only publishes the DSA runtime graph after a
     * complete Extended Features/DSA/core corpus has authenticated. A file
     * with an Extended Features preamble alone is not a DSA save, even if a
     * permissive compatibility decoder can read some of its body. Keep this
     * opt-in probe observational until the caller supplies that full corpus. */
    memset(&dsa_corpus, 0, sizeof(dsa_corpus));
    (void)csb_v1_csbwin_save_loader_boundary_dsa_corpus_receipt_file(
        save_path, 4u * 1024u * 1024u, &dsa_corpus);
    if (!dsa_corpus.runtime_handoff_ready) {
        printf("SKIP: supplied save is not a complete CSBWin DSA corpus "
               "(%s).\n",
               dsa_corpus.decision_label ? dsa_corpus.decision_label :
                                           "unclassified");
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
    CHECK(extended_dsa_tail_is_current(&profile),
          "runtime retains the byte-authenticated Extended Features tail");
    CHECK(profile.csbwin_extended_level_index_present,
          "resume publishes the source DSA level-index table");
    CHECK(saved_timer_queue_matches_runtime(&profile),
          "each saved CSBWin timer-queue slot retains its exact live timeline receipt");
    CHECK(state->imported_actions != NULL && state->imported_action_count > 0,
          "resume retains authenticated source DSA actions");
    CHECK(csb_v1_runtime_csbwin_dsa_runtime_chain_receipt_pc34(
              &profile, &chain_receipt) &&
              chain_receipt.valid &&
              chain_receipt.dsa_catalog_valid &&
              chain_receipt.level_index_valid &&
              chain_receipt.timer_queue_event_chain_valid,
          "resume exposes one production DSA catalog/index/timer chain receipt");

    if (profile.csbwin_extended_features_valid &&
        profile.csbwin_extended_level_index_present &&
        state->imported_actions && state->imported_action_count > 0) {
        for (i = 0; i < state->imported_action_count; ++i) {
            const CSB_V1_DSAImportedAction *action = &state->imported_actions[i];
            const CSB_V1_DSAImportedAction *resolved = NULL;
            CSB_V1_CSBWinDSACoreProgramReceipt core_receipt;
            int action_ordinal = action_ordinal_in_state(state, i);

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
            if (action_ordinal >= 0 &&
                csb_v1_csbwin_dsa_verify_authenticated_core_program(
                    state, action->dsa_id, action->state_index,
                    action_ordinal, &core_receipt) ==
                    CSB_V1_CSBWIN_DSA_CORE_OK && core_receipt.valid) {
                if (core_receipt.stack_core) ++verified_core_actions;
                if (core_receipt.transfer_only) ++verified_transfer_actions;
                if (core_receipt.requires_runtime_owner) {
                    ++verified_runtime_owner_actions;
                }
                if (core_receipt.conditional_core) {
                    ++verified_conditional_actions;
                }
                if (core_receipt.arithmetic_core) {
                    ++verified_arithmetic_actions;
                }
                if (core_receipt.variable_core) {
                    ++verified_variable_actions;
                }
                if (core_receipt.timer_core) {
                    ++verified_timer_actions;
                }
                if (core_receipt.message_core) {
                    ++verified_message_actions;
                }
                if (action->program_word_count > 0 &&
                    ((action->program_words[0] & 0x3fu) ==
                         CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER ||
                     (action->program_words[0] & 0x3fu) ==
                         CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER32)) {
                    ++verified_teleporter_copy_actions;
                }
                if (action->program_word_count > 0 &&
                    (action->program_words[0] & 0x3fu) ==
                        CSB_V1_CSBWIN_DSACMD_AMPERSAND &&
                    ((action->program_words[0] >> 6) & 0x7fu) == 76u) {
                    ++verified_actuator_copy_actions;
                }
                if (core_receipt.dungeon_mutation_core) {
                    ++verified_dungeon_mutation_actions;
                }
            }
        }
        CHECK(verified_core_actions + verified_transfer_actions > 0,
              "real DSA corpus contains at least one verified executable core action");
        printf("CSBWIN_DSA_CORE_ACTIONS=%d transfer=%d runtime_owner=%d "
               "conditional=%d arithmetic=%d variable=%d timer=%d message=%d "
               "copy_teleporter=%d actuator_copy=%d dungeon_mutation=%d\n",
               verified_core_actions, verified_transfer_actions,
               verified_runtime_owner_actions, verified_conditional_actions,
               verified_arithmetic_actions, verified_variable_actions,
               verified_timer_actions, verified_message_actions,
               verified_teleporter_copy_actions,
               verified_actuator_copy_actions,
               verified_dungeon_mutation_actions);

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
        source_binding_found = find_source_dsa_binding(
            &profile, dungeon, &source_binding);
        CHECK(source_binding_found,
              "a decoded source type-47 actuator resolves through the current saved DSA index");
        if (source_binding_found) {
            const CSB_V1_DSAImportedAction *bound_action = NULL;

            for (i = 0; i < state->imported_action_count; ++i) {
                if (state->imported_actions[i].dsa_id == source_binding.dsa_id) {
                    bound_action = &state->imported_actions[i];
                    break;
                }
            }
            CHECK(bound_action != NULL &&
                      state->imported_headers[source_binding.dsa_id].valid,
                  "source actuator binding reaches an authenticated save-owned DSA");
        }

        actions_before_tick = calloc((size_t)state->imported_action_count,
                                    sizeof(*actions_before_tick));
        action_hashes_before_tick = calloc((size_t)state->imported_action_count,
                                           sizeof(*action_hashes_before_tick));
        CHECK(actions_before_tick != NULL && action_hashes_before_tick != NULL,
              "allocate a complete save-owned DSA catalog receipt");
        if (actions_before_tick && action_hashes_before_tick) {
            int core_resume_result;

            for (i = 0; i < state->imported_action_count; ++i) {
                actions_before_tick[i] = &state->imported_actions[i];
                action_hashes_before_tick[i] = action_fingerprint(
                    actions_before_tick[i]);
            }
            csb_v1_runtime_tick(&profile, CSB_V1_TICK_MS_NOMINAL);
            CHECK(action_catalog_survives_tick(
                      &profile.csbwin_extended_dsa_state,
                      actions_before_tick,
                      action_hashes_before_tick,
                      state->imported_action_count),
                  "runtime tick preserves every admitted source DSA action and selector");
            CHECK(profile.dungeon_handle == dungeon &&
                      csb_v1_dungeon_get_current() == dungeon &&
                      remaining_saved_timer_queue_matches_runtime(&profile),
                  "post-tick live queue retains only exact package TIMER slots");
            CHECK(csb_v1_runtime_csbwin_dsa_runtime_chain_receipt_pc34(
                      &profile, &chain_receipt) &&
                      chain_receipt.valid &&
                      chain_receipt.dsa_catalog_valid &&
                      chain_receipt.level_index_valid &&
                      chain_receipt.timer_queue_event_chain_valid,
                  "runtime tick keeps the production DSA runtime-chain receipt valid");
            if (profile.csbwin_last_saved_timer_dsa_valid) {
                CHECK(saved_timer_dsa_execution_is_authenticated(&profile),
                      "live tick executed an exact saved TimerQueue DSA action");
                CHECK(chain_receipt.saved_timer_dsa_execution_valid,
                      "production receipt names the executed saved TimerQueue DSA action");
                CHECK(csb_v1_runtime_get_last_csbwin_dsa_execution_receipt_pc34(
                          &profile, &execution_receipt) &&
                          execution_receipt.valid &&
                          execution_receipt.rollback_guarded &&
                          execution_receipt.command_count > 0u &&
                          (execution_receipt.stack_core ||
                           execution_receipt.transfer_only) &&
                          (!execution_receipt.timer_core ||
                           execution_receipt.requires_runtime_owner) &&
                          (!execution_receipt.message_core ||
                           execution_receipt.timer_core) &&
                          (execution_receipt.timer_scheduled_count == 0u ||
                           execution_receipt.last_scheduled_event_type != 0u) &&
                          (!execution_receipt.dungeon_mutation_core ||
                           execution_receipt.requires_runtime_owner) &&
                          (execution_receipt.transfer_count == 0u ||
                           (execution_receipt.transfer_return_count > 0u &&
                            execution_receipt.transfer_frame_push_count ==
                                execution_receipt.transfer_frame_pop_count &&
                            execution_receipt.maximum_subroutine_depth >=
                                execution_receipt.transfer_frame_push_count &&
                            execution_receipt.transfer_returned_by_missing_program)) &&
                          execution_receipt.dsa_id ==
                              profile.csbwin_last_saved_timer_dsa_id &&
                          execution_receipt.state_index ==
                              profile.csbwin_last_saved_timer_dsa_state_index &&
                          execution_receipt.column ==
                              profile.csbwin_last_saved_timer_dsa_column &&
                          execution_receipt.action_ordinal ==
                              profile.csbwin_last_saved_timer_dsa_action_ordinal,
                      "production execution receipt publishes the committed authenticated DSA core");
                if (execution_receipt.transfer_count > 0u) {
                    printf("CSBWIN_DSA_TRANSFER_CHAIN=transfers=%u returns=%u "
                           "push=%u pop=%u max_depth=%u final_state=%d\n",
                           execution_receipt.transfer_count,
                           execution_receipt.transfer_return_count,
                           execution_receipt.transfer_frame_push_count,
                           execution_receipt.transfer_frame_pop_count,
                           execution_receipt.maximum_subroutine_depth,
                           execution_receipt.transfer_final_state);
                }
                if (execution_receipt.timer_scheduled_count > 0u) {
                    printf("CSBWIN_DSA_MESSAGE_TIMER=scheduled=%u event=%u "
                           "target=0x%08lx\n",
                           execution_receipt.timer_scheduled_count,
                           execution_receipt.last_scheduled_event_type,
                           (unsigned long)
                               execution_receipt.last_scheduled_target_location);
                }
                if (execution_receipt.teleporter_copy_count > 0u) {
                    printf("CSBWIN_DSA_COPY_TELEPORTER=copies=%u src=0x%08lx "
                           "dst=0x%08lx\n",
                           execution_receipt.teleporter_copy_count,
                           (unsigned long)execution_receipt
                               .last_teleporter_copy_source_location,
                           (unsigned long)execution_receipt
                               .last_teleporter_copy_destination_location);
                }
                if (execution_receipt.actuator_copy_count > 0u) {
                    printf("CSBWIN_DSA_COPY_ACTUATOR=copies=%u src=0x%04x "
                           "dst=0x%04x\n",
                           execution_receipt.actuator_copy_count,
                           execution_receipt.last_actuator_copy_source_thing,
                           execution_receipt
                               .last_actuator_copy_destination_thing);
                }
                puts("CSBWIN_DSA_TIMER_ACTION=verified");
            } else {
                puts("CSBWIN_DSA_TIMER_ACTION=unavailable");
            }
            CHECK(source_file_receipt_matches(dungeon_path, &dungeon_receipt) &&
                      source_file_receipt_matches(save_path, &save_receipt),
                  "package files retain their complete size/FNV receipts across runtime proof");
            core_resume_result = exported_core_reloads_after_tick(&profile);
            CHECK(core_resume_result != 0,
                  "post-tick core export either reloads exactly or remains unavailable");
            if (core_resume_result > 0) {
                puts("CSBWIN_DSA_CORE_RESUME_AFTER_TICK=verified");
            } else {
                puts("CSBWIN_DSA_CORE_RESUME_AFTER_TICK=unavailable");
            }
        }
    }

    free(action_hashes_before_tick);
    free(actions_before_tick);
    csb_v1_runtime_cleanup(&profile);
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "runtime cleanup releases the package dungeon singleton");

    printf("\n=== Summary: %d checks, %d failures ===\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
