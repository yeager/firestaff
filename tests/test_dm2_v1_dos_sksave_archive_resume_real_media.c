/* Opt-in real DOS archive::SKSAVE resume gate. */

#include "dm2_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    DM2_V1_BootStartupLaunch launch;
    char save_path[1024];
    int launched;
    int prepared;
    int committed;
    DM2_V1_BootRuntimeReceipt before_turn;
    DM2_V1_BootRuntimeReceipt after_turn;
    int turned;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ARCHIVE is required");
        return 0;
    }
    if (snprintf(save_path, sizeof(save_path), "%s::data/sksave1.dat",
                 archive) >= (int)sizeof(save_path)) {
        puts("FAIL: archive path is too long");
        return 1;
    }

    memset(&launch, 0, sizeof(launch));
    launched = dm2_v1_boot_startup_launch_alloc(archive, &launch) &&
        launch.profile != NULL;
    prepared = launched &&
        dm2_v1_boot_prepare_sksave_resume_path(&launch, save_path);
    committed = prepared &&
        dm2_v1_boot_commit_sksave_resume_session(launch.profile);
    memset(&before_turn, 0, sizeof(before_turn));
    memset(&after_turn, 0, sizeof(after_turn));
    turned = committed &&
        dm2_v1_boot_runtime_capture(launch.profile, &before_turn) &&
        dm2_v1_boot_runtime_turn(launch.profile, -1, &after_turn) &&
        after_turn.operation_result == 0 &&
        after_turn.party_dir == ((before_turn.party_dir + 3) & 3);
    dm2_v1_boot_startup_launch_cleanup(&launch);
    if (!turned) {
        printf("FAIL: archive SKSAVE resume launch=%d prepare=%d commit=%d turn=%d\n",
               launched, prepared, committed, turned);
        return 1;
    }
    puts("PASS: authentic DM2 DOS archive::SKSAVE resumes and turns in memory");
    return 0;
}
