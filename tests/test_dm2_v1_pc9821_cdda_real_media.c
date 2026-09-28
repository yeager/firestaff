#include "dm2_v1_boot.h"
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_cdda_cd_dat.h"
#include "dm2_v1_dungeon_loader.h"
#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM2_PC9821_ARCHIVE");
    DM2_V1_BootProfile profile;
    DM2_V1_CddaCdDat cd;
    DM2_V1_BootStartupLaunch launch;
    DM2_V1_StartupMenuPointerLayout new_game_layout;
    DM2_V1_DungeonData dungeon;
    DM2_V1_AssetLoader graphics;
    M12_AssetStatus status;
    unsigned char seen[100] = {0};
    int extracted = 0;
    int i;

    if (!archive || !archive[0]) {
        puts("SKIP: authentic DM2 PC-9821 archive is not staged");
        return 77;
    }
    dm2_v1_boot_profile_init(&profile);
    if (dm2_v1_boot_scan_assets(&profile, archive) != 0 ||
        !profile.assets_verified ||
        profile.platform != DM2_PLATFORM_PC9821_JA ||
        !profile.cdda_cd_dat_verified || !profile.pc9821_disc_image ||
        !dm2_v1_cdda_cd_dat_parse(&cd, profile.cdda_cd_dat_data,
                                  profile.cdda_cd_dat_size)) {
        fprintf(stderr, "FAIL: authentic PC-9821 CDDA source did not bind\n");
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    if (dm2_v1_dungeon_load(&dungeon, profile.dungeon_mem,
                            (int)profile.dungeon_mem_size) != 0 ||
        dungeon.square_bytes != 1) {
        fputs("FAIL: authentic PC-9821 dungeon payload did not load as G1\n",
              stderr);
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    if (dm2_v1_asset_loader_init(&graphics, profile.graphics_mem,
                                 profile.graphics_mem_size) != 0) {
        fputs("FAIL: authentic PC-9821 GRAPHICS.DAT did not initialize as GDAT\n",
              stderr);
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    if (!dm2_v1_asset_loader_verify(&graphics)) {
        fputs("FAIL: authentic PC-9821 GDAT failed header verification\n", stderr);
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    if (!dm2_v1_asset_loader_validate_typed_graph(&graphics)) {
        fputs("FAIL: authentic PC-9821 GDAT typed graph is incomplete\n", stderr);
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    for (i = 0; i < dungeon.level_count; ++i) {
        uint16_t scene_flags = 0u;
        int graphicsset = dm2_v1_dungeon_get_map_graphics_style(&dungeon, i);
        if (graphicsset < 0 ||
            !dm2_v1_asset_load_word_value(
                &graphics, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset,
                DM2_GDAT_GFXSET_SCENE_FLAGS, &scene_flags)) {
            fprintf(stderr,
                    "FAIL: PC-9821 map %d graphics set %d lacks scene flags\n",
                    i, graphicsset);
            dm2_v1_boot_cleanup(&profile);
            return 1;
        }
    }
    if (!dm2_v1_boot_startup_menu_pointer_layout(&profile,
                                                 &new_game_layout) ||
        !new_game_layout.valid || new_game_layout.new_game.w <= 0 ||
        new_game_layout.new_game.h <= 0) {
        fputs("FAIL: authentic PC-9821 GDAT has no New Game hit target\n",
              stderr);
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    printf("PC9821_NEW_GAME_RECT=%d,%d,%d,%d\n",
           new_game_layout.new_game.x, new_game_layout.new_game.y,
           new_game_layout.new_game.w, new_game_layout.new_game.h);
    M12_AssetStatus_ScanGame(&status, archive, "dm2");
    {
        int version_index = M12_AssetStatus_FindVersionIndex("dm2", "pc9821-ja");
        const char *runtime_data_dir =
            M12_AssetStatus_GetRuntimeDataDir(&status, "dm2");
        printf("PC9821_M12 available=%d versionMatched=%d runtimeDataDir=%s\n",
               M12_AssetStatus_GameAvailable(&status, "dm2"),
               version_index >= 0 &&
                   M12_AssetStatus_GetVersion(&status, "dm2",
                                              (size_t)version_index)->matched,
               runtime_data_dir);
        if (!M12_AssetStatus_GameAvailable(&status, "dm2") ||
            version_index < 0 ||
            !M12_AssetStatus_GetVersion(&status, "dm2",
                                        (size_t)version_index)->matched ||
            !runtime_data_dir ||
            !strstr(runtime_data_dir,
                    "Dungeon-Master-II-Skullkeep_PC-9821_JA.zip")) {
            fputs("FAIL: M12 did not retain the explicitly selected PC-9821 ZIP owner\n",
                  stderr);
            dm2_v1_boot_cleanup(&profile);
            return 1;
        }
        for (i = 0; i < (int)M12_AssetStatus_GetRequiredFileCount(&status, "dm2");
             ++i) {
            const M12_AssetRequiredFileStatus *required =
                M12_AssetStatus_GetRequiredFile(&status, "dm2", (size_t)i);
            printf("PC9821_REQUIRED role=%s matched=%d path=%s hash=%s\n",
                   required->roleId, required->matched, required->matchedPath,
                   required->matchedHash);
        }
    }
    for (i = 0; i < (int)DM2_CDDA_CD_DAT_ENTRY_COUNT; ++i) {
        const DM2_V1_CddaEntry *entry = &cd.entries[i];
        uint8_t track = entry->track;
        int selected_track = -1;
        uint8_t *pcm = NULL;
        int media_verified = 0;
        size_t pcm_size;
        if (track < 2u || track > 7u || seen[track]) continue;
        if (!dm2_v1_boot_music_track_for_level(
                &profile, entry->level, entry->x, entry->y,
                &selected_track) || selected_track != (int)track) {
            fprintf(stderr, "FAIL: PC-9821 CD.DAT track %u did not dispatch\n",
                    (unsigned int)track);
            dm2_v1_boot_cleanup(&profile);
            return 1;
        }
        pcm_size = dm2_v1_boot_load_cdda_track(&profile, (int)track,
                                                &pcm, &media_verified);
        if (!pcm || pcm_size == 0u || !media_verified) {
            fprintf(stderr, "FAIL: original PC-9821 track %u did not decode\n",
                    (unsigned int)track);
            free(pcm);
            dm2_v1_boot_cleanup(&profile);
            return 1;
        }
        free(pcm);
        seen[track] = 1u;
        ++extracted;
    }
    dm2_v1_boot_cleanup(&profile);
    if (extracted == 0) {
        fputs("FAIL: PC-9821 CD.DAT named no extractable audio tracks\n", stderr);
        return 1;
    }
    if (!dm2_v1_boot_startup_launch_alloc(archive, &launch)) {
        fprintf(stderr,
                "FAIL: authenticated PC-9821 media could not enter the DM2 runtime (prepare result %d)\n",
                (int)launch.prepare_result);
        dm2_v1_boot_startup_launch_cleanup(&launch);
        return 1;
    }
    dm2_v1_boot_startup_launch_cleanup(&launch);
    printf("PASS: selected PC-9821 ZIP retained; %d authentic CDDA track(s) extracted; DM2 runtime mounted\n",
           extracted);
    return 0;
}
