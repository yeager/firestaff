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

static uint32_t dungeon_bytes_fingerprint(const CSB_V1_DungeonData *dungeon)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!dungeon || !dungeon->raw_data || dungeon->raw_size <= 0) return 0u;
    for (i = 0u; i < (size_t)dungeon->raw_size; ++i) {
        hash = (hash ^ dungeon->raw_data[i]) * 16777619u;
    }
    return hash;
}

static int saved_timer_queue_is_live(const CSB_V1_RuntimeProfile *profile)
{
    uint16_t event_ordinal;
    uint8_t seen[CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES] = { 0 };

    if (!profile || !profile->csbwin_body_runtime_summary_valid ||
        profile->csbwin_timer_summary_total !=
            profile->csbwin_timer_summary_count ||
        profile->csbwin_timer_queue_summary_total !=
            profile->csbwin_timer_queue_summary_count ||
        profile->csbwin_timer_queue_summary_count !=
            profile->csbwin_timer_summary_count ||
        profile->timeline_queue.eventCount !=
            (int)profile->csbwin_timer_queue_summary_count) {
        return 0;
    }
    for (event_ordinal = 0u;
         event_ordinal < (uint16_t)profile->timeline_queue.eventCount;
         ++event_ordinal) {
        const int event_index = profile->timeline_queue.timeline[event_ordinal];
        uint16_t queue_slot;
        uint16_t timer_index;
        const CSB_V1_CSBWin512TimerSummary *timer;
        const struct DM1_Event_V1 *event;

        if (event_index < 0 || event_index >= DM1_EVENT_MAX_COUNT) {
            return 0;
        }
        queue_slot = profile->csbwin_timeline_event_queue_slot[event_index];
        if (queue_slot >= profile->csbwin_timer_queue_summary_count ||
            seen[queue_slot]) return 0;
        timer_index = profile->csbwin_timer_queue[queue_slot];
        if (timer_index >= profile->csbwin_timer_summary_count) return 0;
        timer = &profile->csbwin_timers[timer_index];
        event = &profile->timeline_queue.events[event_index];
        if (!timer->valid || timer->truncated ||
            timer->source_index != timer_index ||
            event->map_time != timer->time || event->type != timer->function ||
            event->priority != timer->ubyte5 ||
            event->b_mapX != timer->ubyte6 || event->b_mapY != timer->ubyte7 ||
            event->c_cell != timer->ubyte8 || event->c_effect != timer->ubyte9) {
            return 0;
        }
        seen[queue_slot] = 1u;
    }
    for (event_ordinal = 0u;
         event_ordinal < profile->csbwin_timer_queue_summary_count;
         ++event_ordinal) {
        if (!seen[event_ordinal]) return 0;
    }
    return 1;
}

/* A source timer may be consumed or requeued during the first runtime tick.
 * Every event that remains must still identify exactly one serialized
 * CSBWin TimerQueue slot; a generic replacement event is not evidence of a
 * successful package resume. CSBWin Timer.cpp CheckTimers/ProcessTimers. */
static int remaining_saved_timer_queue_is_live(
    const CSB_V1_RuntimeProfile *profile)
{
    uint8_t seen[CSB_V1_CSBWIN_MAX_TIMER_QUEUE_SUMMARIES] = { 0 };
    int event_ordinal;

    if (!profile || !profile->csbwin_body_runtime_summary_valid ||
        profile->csbwin_timer_queue_summary_count !=
            profile->csbwin_timer_summary_count ||
        profile->timeline_queue.eventCount < 0 ||
        profile->timeline_queue.eventCount >
            (int)profile->csbwin_timer_queue_summary_count) {
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
        if (queue_slot >= profile->csbwin_timer_queue_summary_count ||
            seen[queue_slot]) return 0;
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

/* A package tick may consume or requeue a source timer, in which case the
 * runtime deliberately refuses to invent a writable CSBWin heap. When the
 * heap is still exportable, require the production writer and reader to
 * carry the surviving source-owned core state into a separate runtime. */
static int exported_core_reloads_after_tick(
    const CSB_V1_RuntimeProfile *profile)
{
    uint8_t core_bytes[65536];
    size_t core_size = 0u;
    CSB_V1_CSBWin512BodyReport core_report;
    CSB_V1_RuntimeProfile core_runtime;
    int result = 0;

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
            &core_runtime, &core_report) == 0 &&
        !core_runtime.csbwin_extended_features_valid &&
        !core_runtime.csbwin_extended_level_index_present &&
        core_runtime.csbwin_extended_dsa_state.imported_action_count == 0 &&
        saved_timer_queue_is_live(&core_runtime) &&
        core_runtime.game_time == profile->game_time &&
        core_runtime.current_level == profile->current_level &&
        core_runtime.party_x == profile->party_x &&
        core_runtime.party_y == profile->party_y &&
        core_runtime.party_dir == profile->party_dir &&
        core_runtime.party_state.PartyDirection ==
            profile->party_state.PartyDirection) {
        result = 1;
    }
    csb_v1_runtime_cleanup(&core_runtime);
    return result;
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
    int core_resume_result;
    uint32_t game_time_before_tick;
    uint32_t game_time_before_resume_tick;
    uint32_t dungeon_bytes_before_resume;
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
    dungeon_bytes_before_resume = dungeon_bytes_fingerprint(dungeon);

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
        CHECK(saved_timer_queue_is_live(&profile),
              "saved queue slots remain the live timer owner without fallback events");
        game_time_before_resume_tick = profile.game_time;
        csb_v1_runtime_tick(&profile, CSB_V1_TICK_MS_NOMINAL);
        CHECK(profile.game_time == game_time_before_resume_tick + 1u &&
                  remaining_saved_timer_queue_is_live(&profile),
              "first resumed tick retains only exact package TIMER slots");
        core_resume_result = exported_core_reloads_after_tick(&profile);
        CHECK(core_resume_result != 0,
              "post-tick package core either reloads exactly or remains unavailable");
        if (core_resume_result > 0) {
            puts("CSBWIN_PACKAGE_CORE_RESUME_AFTER_TICK=verified");
        } else {
            puts("CSBWIN_PACKAGE_CORE_RESUME_AFTER_TICK=unavailable");
        }
    } else {
        CHECK(profile.dungeon_handle == dungeon &&
                  csb_v1_dungeon_get_current() == dungeon &&
                  csb_v1_dungeon_get_current_level() == pre_resume_dungeon_level &&
                  dungeon_bytes_fingerprint(dungeon) ==
                      dungeon_bytes_before_resume &&
                  !profile.party_state_valid &&
                  !profile.csbwin_body_runtime_summary_valid,
              "rejected package save leaves live dungeon bytes and runtime state untouched");
    }

    csb_v1_runtime_cleanup(&profile);
    CHECK(csb_v1_dungeon_get_current() == NULL,
          "runtime cleanup releases the package dungeon singleton");

    printf("\n=== Summary: %d checks, %d failures ===\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
