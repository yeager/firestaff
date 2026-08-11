#include "csb_v1_boot.h"
#include "csb_v1_fmtowns_game.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *archive = getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE");
    const char *language = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE");
    const int japanese = language && strcmp(language, "ja") == 0;
    CSB_V1_BootStartupLaunch_PC34 launch;
    CSB_V1_FmtownsGameHandoffReceipt game;
    CSB_V1_FmtownsStartupState startup;
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_FmtownsUserSaveReceipt user_save;
    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_CSB_FMTOWNS_ARCHIVE not set");
        return 0;
    }
    if (!csb_v1_boot_startup_launch_alloc_with_variant_pc34(
            archive, NULL, NULL, NULL, NULL,
            japanese ? CSB_V1_VARIANT_FMTOWNS_JA : CSB_V1_VARIANT_FMTOWNS_EN,
            &launch)) {
        fprintf(stderr, "FAIL: packed CSB FM Towns launch: %s\n",
                launch.failure_host_receipt.status
                    ? launch.failure_host_receipt.status : "(no status)");
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 1;
    }
    puts("PASS: packed CSB FM Towns launch reads the original ZIP in RAM");
    memset(&game, 0, sizeof(game));
    memset(&startup, 0, sizeof(startup));
    if (!csb_v1_fmtowns_game_handoff_open(
            launch.profile,
            japanese ? CSB_FMTOWNS_SWITCH_JAPANESE
                     : CSB_FMTOWNS_SWITCH_ENGLISH, &game)) {
        fprintf(stderr, "FAIL: packed CSB C03/MINI handoff\n");
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 1;
    }
    puts("PASS: packed CSB C03/MINI handoff stays in RAM");
    if (getenv("FIRESTAFF_CSB_FMTOWNS_USER_SAVE")) {
        memset(&user_save, 0, sizeof(user_save));
        if (!csb_v1_fmtowns_game_user_save_open(
                launch.profile, &game,
                getenv("FIRESTAFF_CSB_FMTOWNS_USER_SAVE"), &user_save)) {
            fprintf(stderr, "FAIL: packed CSB user save handoff\n");
            csb_v1_boot_startup_launch_cleanup_pc34(&launch);
            return 1;
        }
        puts("PASS: packed CSB user save handoff stays source-bound");
    }
    if (!csb_v1_fmtowns_game_load_startup_state(&game, &startup)) {
        fprintf(stderr, "FAIL: packed CSB MINI startup state\n");
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 1;
    }
    printf("PASS: packed CSB MINI startup state map=%d x=%d y=%d\n",
           startup.party_map_index, startup.party.PartyMapX,
           startup.party.PartyMapY);
    csb_v1_fmtowns_game_startup_state_free(&startup);
    memset(&session, 0, sizeof(session));
    if (!csb_v1_boot_startup_runtime_asset_session_open_pc34(
            launch.profile, &session)) {
        fprintf(stderr, "FAIL: packed CSB startup surfaces\n");
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 1;
    }
    puts("PASS: packed CSB startup surfaces decode from RAM");
    csb_v1_boot_startup_runtime_asset_session_release_pc34(&session);
    csb_v1_boot_startup_launch_cleanup_pc34(&launch);
    return 0;
}
