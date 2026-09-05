#include "csb_v1_boot.h"
#include "csb_v1_fmtowns_game.h"
#include "csb_v1_inscription_presentation.h"

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
    char expected_mini_path[512];
    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_CSB_FMTOWNS_ARCHIVE not set");
        return 77;
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
    if (!launch.profile->fmtowns_inventory_rectangles_valid ||
        launch.profile->fmtowns_inventory_rectangle_x[0] != 6 ||
        launch.profile->fmtowns_inventory_rectangle_y[0] != 53 ||
        launch.profile->fmtowns_inventory_rectangle_x[1] != 62 ||
        launch.profile->fmtowns_inventory_rectangle_y[1] != 53 ||
        launch.profile->fmtowns_inventory_rectangle_x[29] != 202 ||
        launch.profile->fmtowns_inventory_rectangle_y[29] != 33 ||
        launch.profile->fmtowns_inventory_rectangle_width[0] != 16 ||
        launch.profile->fmtowns_inventory_rectangle_height[29] != 16) {
        fprintf(stderr, "FAIL: packed CSB F31 C696 inventory receipt\n");
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 1;
    }
    puts("PASS: packed CSB F31 C696 owns C017 draw and pointer rectangles");
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
    if (!game.entrance_palette_verified ||
        game.entrance_palette_source_offset == 0u ||
        game.entrance_palette_rgb6[1][0] != 0x1bu ||
        game.entrance_palette_rgb6[1][1] != 0x1bu ||
        game.entrance_palette_rgb6[1][2] != 0x1bu ||
        game.entrance_palette_rgb6[9][0] != 0x3fu ||
        game.entrance_palette_rgb6[9][1] != 0x03u ||
        game.entrance_palette_rgb6[9][2] != 0x03u ||
        game.entrance_palette_rgb6[15][0] != 0x3fu ||
        game.entrance_palette_rgb6[15][1] != 0x3fu ||
        game.entrance_palette_rgb6[15][2] != 0x3fu) {
        fprintf(stderr, "FAIL: packed CSB C28_ENTRANCE_CSB palette\n");
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 1;
    }
    printf("PASS: packed CSB C28 entrance palette is source-owned at 0x%x\n",
           game.entrance_palette_source_offset);
    if (!game.dungeon_palettes_verified ||
        game.dungeon_palettes_source_offset == 0u ||
        game.dungeon_palette_rgb6[0][1][0] != 0x1bu ||
        game.dungeon_palette_rgb6[0][1][1] != 0x1bu ||
        game.dungeon_palette_rgb6[0][1][2] != 0x1bu ||
        game.dungeon_palette_rgb6[5][15][0] > 0x3fu ||
        game.dungeon_palette_rgb6[5][15][1] > 0x3fu ||
        game.dungeon_palette_rgb6[5][15][2] > 0x3fu) {
        fprintf(stderr, "FAIL: packed CSB C00_LIGHT0--C05_LIGHT5 palettes\n");
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 1;
    }
    printf("PASS: packed CSB dungeon palette corpus is source-owned at 0x%x\n",
           game.dungeon_palettes_source_offset);
    if (snprintf(expected_mini_path, sizeof(expected_mini_path), "%s::%s",
                 archive, japanese ? "CJDATA/MINI.DAT" : "CDATA/MINI.DAT") < 0 ||
        strcmp(game.startup_mini_path, expected_mini_path) != 0) {
        fprintf(stderr, "FAIL: packed CSB MINI provenance path: %s\n",
                game.startup_mini_path);
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 1;
    }
    puts("PASS: packed CSB MINI provenance names its original ZIP member");
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
    if (!startup.valid || startup.party_map_index != 4 ||
        startup.party.PartyMapX != 22 || startup.party.PartyMapY != 18 ||
        startup.party.PartyDirection != 2 || startup.party.ChampionCount != 1 ||
        startup.game_time != (japanese ? 88u : 82u) ||
        startup.timeline_queue.eventCount != 23 ||
        startup.timeline_queue.firstUnusedIndex != 23 ||
        startup.active_group_count != 8u ||
        startup.active_group_resolved_count != 8u) {
        fprintf(stderr, "FAIL: packed CSB MINI state does not match retail seed\n");
        csb_v1_fmtowns_game_startup_state_free(&startup);
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 1;
    }
    printf("PASS: packed CSB MINI startup state map=%d x=%d y=%d\n",
           startup.party_map_index, startup.party.PartyMapX,
           startup.party.PartyMapY);
    if (strcmp(startup.party.Champions[0].Name, "HALK") != 0 ||
        startup.party.Champions[0].Slots[0] != THING_NONE ||
        startup.party.Champions[0].Slots[1] != THING_NONE ||
        !startup.active_group_owners[6].valid ||
        startup.active_group_owners[6].map_index != 4 ||
        startup.active_group_owners[6].map_x != 21 ||
        startup.active_group_owners[6].map_y != 18 ||
        !startup.active_group_owners[7].valid ||
        startup.active_group_owners[7].map_index != 4 ||
        startup.active_group_owners[7].map_x != 23 ||
        startup.active_group_owners[7].map_y != 18) {
        fprintf(stderr, "FAIL: packed CSB MINI authentic dark/monster start facts\n");
        csb_v1_fmtowns_game_startup_state_free(&startup);
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 1;
    }
    puts("PASS: packed CSB MINI authentic start owns unlit HALK and adjacent groups");
    if (!csb_v1_fmtowns_game_apply_startup_state(&startup,
                                                  &launch.profile->runtime)) {
        fprintf(stderr, "FAIL: packed CSB MINI state runtime application\n");
        csb_v1_fmtowns_game_startup_state_free(&startup);
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 1;
    }
    if (launch.profile->runtime.current_level != 4 ||
        launch.profile->runtime.party_x != 22 ||
        launch.profile->runtime.party_y != 18 ||
        launch.profile->runtime.party_dir != 2 ||
        launch.profile->runtime.party_state.ChampionCount != 1 ||
        launch.profile->runtime.active_group_state_count != 8u ||
        ((CSB_V1_DungeonData *)launch.profile->runtime.dungeon_handle)
                ->map_difficulty[launch.profile->runtime.current_level] != 4u) {
        fprintf(stderr, "FAIL: packed CSB MINI runtime changed source start facts\n");
        csb_v1_fmtowns_game_startup_state_free(&startup);
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 1;
    }
    puts("PASS: packed CSB MINI runtime preserves map-4 difficulty and group state");
    {
        const CSB_V1_DungeonData *dungeon =
            (const CSB_V1_DungeonData *)launch.profile->runtime.dungeon_handle;
        int index, decoded_count = 0, high_byte_count = 0;
        for (index = 0; dungeon && index < dungeon->thing_type_counts[2] &&
                        index < 1024; ++index) {
            char decoded_text[256];
            size_t byte;
            uint16_t thing = (uint16_t)((2u << 10) | (unsigned)index);
            if (!csb_v1_runtime_decode_visible_inscription_text_pc34(
                    &launch.profile->runtime, thing, decoded_text,
                    (int)sizeof(decoded_text))) continue;
            ++decoded_count;
            for (byte = 0u; decoded_text[byte] != '\0'; ++byte)
                if ((unsigned char)decoded_text[byte] >= 0x80u)
                    ++high_byte_count;
            if (japanese) {
                const size_t decoded_size = strlen(decoded_text) + 1u;
                size_t at = 0u;
                while (at + 1u < decoded_size) {
                    uint8_t line[256];
                    CSB_V1_F31JPrintableSubstringReceipt receipt;
                    if (decoded_text[at] == '\n') { ++at; continue; }
                    if (!csb_v1_f31j_f0646_printable_substring(
                            (const uint8_t *)decoded_text, decoded_size, &at,
                            1000, line, sizeof(line), &receipt) ||
                        !receipt.valid) {
                        fprintf(stderr,
                                "FAIL: selected F31J C02 %d is not a valid F0646 stream at %zu\n",
                                index, at);
                        csb_v1_fmtowns_game_startup_state_free(&startup);
                        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
                        return 1;
                    }
                    if (decoded_text[at] == '\n') ++at;
                }
            }
        }
        if (decoded_count == 0 ||
            (japanese ? high_byte_count == 0 : high_byte_count != 0)) {
            fprintf(stderr,
                    "FAIL: selected F31%s C02/F0168 language route (%d texts, %d high bytes)\n",
                    japanese ? "J" : "E", decoded_count, high_byte_count);
            csb_v1_fmtowns_game_startup_state_free(&startup);
            csb_v1_boot_startup_launch_cleanup_pc34(&launch);
            return 1;
        }
        printf("PASS: selected F31%s owns %d visible C02 strings (%d Shift-JIS bytes)\n",
               japanese ? "J" : "E", decoded_count, high_byte_count);
    }
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
