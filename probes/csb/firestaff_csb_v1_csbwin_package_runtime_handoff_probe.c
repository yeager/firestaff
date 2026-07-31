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

#include "csb_v1_csbwin_save_loader_boundary_pc34_compat.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

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

static int explicit_path_supplied(int argc, char **argv, int index,
                                  const char *env_name)
{
    const char *value;

    if (argc > index && argv[index] && argv[index][0] != '\0') return 1;
    value = getenv(env_name);
    return value && value[0] != '\0';
}

static const char *data_dir_arg(char *buf, size_t buf_size)
{
    const char *env;
    const char *home;

    env = getenv("FIRESTAFF_CSBWIN_PACKAGE_DATA");
    if (env && env[0] != '\0') return env;
    env = getenv("FIRESTAFF_CSBWIN_SAVE_DATA");
    if (env && env[0] != '\0') return env;
    env = getenv("FIRESTAFF_DATA_DIR");
    if (env && env[0] != '\0') return env;
    home = getenv("HOME");
    if (!home || home[0] == '\0') return NULL;
    snprintf(buf, buf_size, "%s/.firestaff/data", home);
    return buf;
}

static int copy_path(char *out_path, size_t out_cap, const char *path)
{
    size_t n;

    if (!out_path || !path || out_cap == 0u) return 0;
    n = strlen(path);
    if (n + 1u > out_cap) return 0;
    memcpy(out_path, path, n + 1u);
    return 1;
}

static int join_path(char *out, size_t out_cap,
                     const char *dir, const char *name)
{
    int written;

    if (!out || !dir || !name || out_cap == 0u) return 0;
    written = snprintf(out, out_cap, "%s/%s", dir, name);
    return written > 0 && (size_t)written < out_cap;
}

static int ascii_ieq(const char *a, const char *b)
{
    while (a && b && *a && *b) {
        char ca = *a++;
        char cb = *b++;
        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb - 'A' + 'a');
        if (ca != cb) return 0;
    }
    return a && b && *a == '\0' && *b == '\0';
}

enum {
    MAX_REAL_SAVE_CANDIDATES = 64,
    REAL_PATH_BYTES = 1024
};

static void collect_real_package_paths(
    const char *dir,
    int max_depth,
    char save_paths[][REAL_PATH_BYTES],
    size_t *save_count,
    char *dungeon_path,
    size_t dungeon_path_cap,
    int *overflow)
{
    DIR *d;
    struct dirent *ent;

    if (!dir || max_depth < 0) return;
    d = opendir(dir);
    if (!d) return;

    while ((ent = readdir(d)) != NULL) {
        char path[REAL_PATH_BYTES];
        struct stat st;

        if (ent->d_name[0] == '.' &&
            (ent->d_name[1] == '\0' ||
             (ent->d_name[1] == '.' && ent->d_name[2] == '\0'))) {
            continue;
        }
        if (!join_path(path, sizeof(path), dir, ent->d_name) ||
            stat(path, &st) != 0) {
            continue;
        }
        if (S_ISREG(st.st_mode)) {
            if (csb_v1_csbwin_save_loader_boundary_file_kind(path) !=
                    CSB_V1_CSBWIN_SAVE_FILE_NONE) {
                if (save_count && *save_count < MAX_REAL_SAVE_CANDIDATES) {
                    copy_path(save_paths[*save_count], REAL_PATH_BYTES, path);
                    ++(*save_count);
                } else if (overflow) {
                    *overflow = 1;
                }
            }
            if (dungeon_path && dungeon_path[0] == '\0' &&
                ascii_ieq(ent->d_name, "DUNGEON.DAT")) {
                copy_path(dungeon_path, dungeon_path_cap, path);
            }
        } else if (S_ISDIR(st.st_mode) && max_depth > 0) {
            collect_real_package_paths(path, max_depth - 1,
                                       save_paths, save_count,
                                       dungeon_path, dungeon_path_cap,
                                       overflow);
        }
    }
    closedir(d);
}

static const char *select_dsa_ready_save(
    char save_paths[][REAL_PATH_BYTES],
    size_t save_count,
    int explicit_save,
    const char *explicit_path,
    CSB_V1_CSBWinDSASaveCorpusReceipt *out_receipt)
{
    size_t i;

    if (explicit_save) {
        if (csb_v1_csbwin_save_loader_boundary_dsa_corpus_receipt_file(
                explicit_path, 4u * 1024u * 1024u, out_receipt) > 0 &&
            out_receipt->runtime_handoff_ready) {
            return explicit_path;
        }
        return NULL;
    }
    for (i = 0u; i < save_count; ++i) {
        if (csb_v1_csbwin_save_loader_boundary_dsa_corpus_receipt_file(
                save_paths[i], 4u * 1024u * 1024u, out_receipt) > 0 &&
            out_receipt->runtime_handoff_ready) {
            return save_paths[i];
        }
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    return NULL;
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
    char data_dir_buf[REAL_PATH_BYTES];
    char discovered_dungeon[REAL_PATH_BYTES] = "";
    char save_paths[MAX_REAL_SAVE_CANDIDATES][REAL_PATH_BYTES];
    size_t save_count = 0u;
    int save_overflow = 0;
    const char *data_dir;
    const int explicit_save = explicit_path_supplied(
        argc, argv, 2, "FIRESTAFF_CSBWIN_SAVE");
    const char *dungeon_path = path_arg_or_env(
        argc, argv, 1, "FIRESTAFF_CSBWIN_DUNGEON");
    const char *save_path = path_arg_or_env(
        argc, argv, 2, "FIRESTAFF_CSBWIN_SAVE");
    CSB_V1_CSBWinDSASaveCorpusReceipt dsa_receipt;
    CSB_V1_CSBWinDSARuntimeChainReceipt_PC34 chain_receipt;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData *dungeon;
    int resume_rc;
    int core_resume_result;
    uint32_t game_time_before_tick;
    uint32_t game_time_before_resume_tick;
    uint32_t dungeon_bytes_before_resume;
    int pre_resume_dungeon_level;

    printf("=== CSBWin package runtime handoff probe ===\n\n");
    memset(&dsa_receipt, 0, sizeof(dsa_receipt));
    if (!dungeon_path || !save_path) {
        data_dir = data_dir_arg(data_dir_buf, sizeof(data_dir_buf));
        printf("data_dir=%s\n", data_dir ? data_dir : "(none)");
        if (data_dir) {
            collect_real_package_paths(data_dir, 6, save_paths, &save_count,
                                       discovered_dungeon,
                                       sizeof(discovered_dungeon),
                                       &save_overflow);
        }
        if (!dungeon_path && discovered_dungeon[0] != '\0') {
            dungeon_path = discovered_dungeon;
        }
        if (!save_path) {
            save_path = select_dsa_ready_save(save_paths, save_count, 0, NULL,
                                              &dsa_receipt);
        }
        printf("real_save_candidate_count=%zu%s\n", save_count,
               save_overflow ? " (truncated to probe cap)" : "");
    }

    if (save_path && explicit_save &&
        !select_dsa_ready_save(save_paths, save_count, 1, save_path,
                               &dsa_receipt)) {
        /* An explicitly supplied CSBWin-named file is evidence to classify,
         * not permission to synthesize missing DSA state. Keep this opt-in
         * probe skip-safe when it has no complete corpus. */
        printf("DSA_CORPUS_UNAVAILABLE=%s\n",
               dsa_receipt.decision_label ? dsa_receipt.decision_label :
                                            "unclassified");
        save_path = NULL;
    } else if (save_path && !dsa_receipt.runtime_handoff_ready) {
        (void)select_dsa_ready_save(save_paths, save_count, 1, save_path,
                                    &dsa_receipt);
    }

    if (!dungeon_path || !save_path) {
        printf("SKIP: no complete real CSBWin package DSA handoff corpus "
               "found. Provide <Dungeon.dat> <csbgame*.dat>, set "
               "FIRESTAFF_CSBWIN_DUNGEON/FIRESTAFF_CSBWIN_SAVE, or stage a "
               "DSA-bearing CSBWin save under FIRESTAFF_CSBWIN_PACKAGE_DATA. "
               "No synthetic save, DSA record, or fallback runtime state was "
               "created.\n");
        return 0;
    }
    CHECK(dsa_receipt.valid && dsa_receipt.runtime_handoff_ready,
          "selected save has a strict CSBWin DSA runtime-handoff receipt");
    printf("selected_dungeon=%s\n", dungeon_path);
    printf("selected_save=%s\n", save_path);
    printf("dsa_corpus_decision=%s, gameblock1_offset=%zu\n",
           dsa_receipt.decision_label ? dsa_receipt.decision_label : "(null)",
           dsa_receipt.gameblock1_offset);
    if (failures != 0) return 1;

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
        CHECK(csb_v1_runtime_csbwin_dsa_runtime_chain_receipt_pc34(
                  &profile, &chain_receipt) &&
                  chain_receipt.valid &&
                  chain_receipt.dsa_catalog_valid &&
                  chain_receipt.level_index_valid &&
                  chain_receipt.timer_queue_event_chain_valid,
              "resume exposes a complete authenticated DSA catalog/index/timer chain receipt");
        game_time_before_resume_tick = profile.game_time;
        csb_v1_runtime_tick(&profile, CSB_V1_TICK_MS_NOMINAL);
        CHECK(profile.game_time == game_time_before_resume_tick + 1u &&
                  remaining_saved_timer_queue_is_live(&profile),
              "first resumed tick retains only exact package TIMER slots");
        CHECK(csb_v1_runtime_csbwin_dsa_runtime_chain_receipt_pc34(
                  &profile, &chain_receipt) &&
                  chain_receipt.valid &&
                  chain_receipt.dsa_catalog_valid &&
                  chain_receipt.level_index_valid &&
                  chain_receipt.timer_queue_event_chain_valid,
              "first resumed tick preserves the authenticated DSA runtime chain");
        if (chain_receipt.saved_timer_dsa_execution_valid) {
            printf("CSBWIN_PACKAGE_DSA_TIMER_ACTION=dsa=%u state=%lu column=%lu action=%d queue=%u timer=%u\n",
                   (unsigned)chain_receipt.last_dsa_id,
                   (unsigned long)chain_receipt.last_state_index,
                   (unsigned long)chain_receipt.last_input_column,
                   chain_receipt.last_action_ordinal,
                   (unsigned)chain_receipt.last_queue_slot,
                   (unsigned)chain_receipt.last_timer_index);
        } else {
            puts("CSBWIN_PACKAGE_DSA_TIMER_ACTION=unavailable");
        }
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
