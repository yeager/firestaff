/* Opt-in real DOS archive::SKSAVE resume gate. */

#include "dm2_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    static const struct {
        const char *member;
        int level;
        int party_x;
        int party_y;
        int party_dir;
    } saves[] = {
        { "data/sksave0.dat", 11, 15, 2, 3 },
        { "data/sksave0.bak", 11, 15, 3, 0 },
        { "data/sksave1.dat", 11, 15, 10, 2 },
        { "data/sksave1.bak", 11, 15, 10, 2 },
        { "data/sksave2.dat", 24, 4, 3, 1 },
        { "data/sksave2.bak", 8, 13, 10, 1 },
        { "data/sksave3.dat", 8, 8, 21, 0 },
        { "data/sksave3.bak", 8, 8, 21, 0 }
    };
    const char *archive = getenv("FIRESTAFF_DM2_DOS_ARCHIVE");
    char save_path[1024];
    size_t index;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM2_DOS_ARCHIVE is required");
        return 77;
    }
    for (index = 0u; index < sizeof(saves) / sizeof(saves[0]); ++index) {
        DM2_V1_BootStartupLaunch launch;
        DM2_V1_BootRuntimeReceipt before_turn;
        DM2_V1_BootRuntimeReceipt after_turn;
        int launched;
        int prepared;
        int committed;
        int restored;
        int turned;

        if (snprintf(save_path, sizeof(save_path), "%s::%s", archive,
                     saves[index].member) >= (int)sizeof(save_path)) {
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
        restored = committed &&
            dm2_v1_boot_runtime_capture(launch.profile, &before_turn) &&
            before_turn.current_level == saves[index].level &&
            before_turn.party_x == saves[index].party_x &&
            before_turn.party_y == saves[index].party_y &&
            before_turn.party_dir == saves[index].party_dir;
        turned = restored &&
            dm2_v1_boot_runtime_turn(launch.profile, -1, &after_turn) &&
            after_turn.operation_result == 0 &&
            after_turn.party_dir == ((before_turn.party_dir + 3) & 3);
        dm2_v1_boot_startup_launch_cleanup(&launch);
        if (!turned) {
            printf("FAIL: archive %s launch=%d prepare=%d commit=%d restore=%d turn=%d\n",
                   saves[index].member, launched, prepared, committed,
                   restored, turned);
            return 1;
        }
    }
    puts("PASS: every authentic DM2 DOS archive::SKSAVE slot resumes and turns in memory");
    return 0;
}
