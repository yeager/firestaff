/* Focused CSBWin TimerQueue -> M11 resume handoff.
 * SaveGame.cpp restores the TimerQueue storage, while GAMEBLOCK2.NumTimer
 * names the active heap prefix.  Do not promote unused serialized slots. */

#include "csb_v1_boot.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csbwin_resume_fixture.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;

#define CHECK(condition, message) \
    do { if (!(condition)) { fprintf(stderr, "FAIL: %s\n", (message)); ++failures; } } while (0)

static void fill_spec(M11_GameLaunchSpec *spec, const char *data_dir,
                      const char *save_path)
{
    memset(spec, 0, sizeof(*spec));
    spec->title = "CHAOS STRIKES BACK";
    spec->gameId = "csb";
    spec->sourceId = "csb";
    spec->dataDir = data_dir;
    spec->savePath = save_path;
    spec->rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec->presentationMode = M12_PRESENTATION_V1_ORIGINAL;
}

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_CSB_DATA_DIR");
    char save_dir[] = "/tmp/firestaff-csbwin-timer-XXXXXX";
    char save_path[512];
    CSB_V1_BootProfile *profile;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_CSB_DATA_DIR is not set");
        return 0;
    }
    if (!mkdtemp(save_dir)) {
        fputs("FAIL: unable to allocate CSBWin save fixture\n", stderr);
        return 1;
    }
    snprintf(save_path, sizeof(save_path), "%s/CSBGAME2.DAT", save_dir);
    CHECK(firestaff_test_write_csbwin_resume_fixture(save_path, 0),
          "fixture writes checksum-verified CSBWin timer queue");
    fill_spec(&spec, data_dir, save_path);
    M11_GameView_Init(&view);
    CHECK(M11_GameView_Start(&view, &spec),
          "M11 accepts the verified CSBWin timer queue");
    profile = (CSB_V1_BootProfile *)view.csbBootProfile;
    CHECK(profile && profile->runtime.csbwin_num_timer == 2u &&
              profile->runtime.csbwin_timer_queue_summary_count == 3u &&
              profile->runtime.csbwin_timer_queue[0] == 0u &&
              profile->runtime.csbwin_timer_queue[1] == 2u &&
              profile->runtime.timeline_queue.eventCount == 2 &&
              profile->runtime.timeline_queue.gameTick ==
                  profile->runtime.game_time,
          "M11 binds active queue slots and restored source tick atomically");
    M11_GameView_Shutdown(&view);

    CHECK(firestaff_test_write_csbwin_resume_fixture(save_path, 1),
          "fixture writes a corrupt CSBWin timer queue");
    M11_GameView_Init(&view);
    CHECK(!M11_GameView_Start(&view, &spec) && !view.active &&
              view.csbBootProfile == NULL,
          "M11 rejects a corrupt queue before a session can publish");
    M11_GameView_Shutdown(&view);
    remove(save_path);
    rmdir(save_dir);
    if (failures) {
        fprintf(stderr, "FAIL: csb_v1_m11_csbwin_timer_queue_resume (%d failures)\n",
                failures);
        return 1;
    }
    puts("PASS: csb_v1_m11_csbwin_timer_queue_resume");
    return 0;
}
