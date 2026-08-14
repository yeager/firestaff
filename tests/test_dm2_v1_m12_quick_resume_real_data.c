/* Opt-in real-data regression for M12 -> DM2 DOSBox Quick Resume. */

#include "menu_startup_m12.h"
#include "config_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    const char *data_root = getenv("FIRESTAFF_DM2_M12_DATA_ROOT");
    const char *save_path = getenv("FIRESTAFF_DM2_M12_SAVE_PATH");
    char home_template[] = "/tmp/firestaff-dm2-m12-qr-XXXXXX";
    M12_StartupMenuState state;

    if (!data_root || !data_root[0] || !save_path || !save_path[0]) {
        puts("SKIP: FIRESTAFF_DM2_M12_DATA_ROOT and FIRESTAFF_DM2_M12_SAVE_PATH are not set");
        return 0;
    }
    if (!mkdtemp(home_template)) {
        perror("mkdtemp");
        return 1;
    }
#if defined(_WIN32)
    if (_putenv_s("HOME", home_template) != 0) {
        fprintf(stderr, "_putenv_s HOME failed\n");
        return 1;
    }
#else
    if (setenv("HOME", home_template, 1) != 0) {
        perror("setenv HOME");
        return 1;
    }
#endif
    M12_Config_SetLastSavePath(save_path);
    M12_StartupMenu_InitWithDataDir(&state, data_root, "dm2");
    if (!state.quickResumeAvailable ||
        strcmp(state.quickResumeGameId, "dm2") != 0 ||
        strcmp(state.quickResumeSavePath, save_path) != 0) {
        fprintf(stderr, "FAIL: M12 did not retain the authenticated DM2 save path\n");
        return 1;
    }
    puts("PASS: M12 Quick Resume retains the authenticated DM2 DOSBox save");
    return 0;
}
