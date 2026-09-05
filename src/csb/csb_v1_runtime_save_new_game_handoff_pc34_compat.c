#include "csb_v1_runtime_save_new_game_handoff_pc34_compat.h"

#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"

#include <string.h>

int csb_v1_runtime_save_new_game_handoff_after_input_pc34(
    void *boot_profile,
    const CSB_V1_InputCommandBridgeResult *first_input,
    const char *save_path,
    CSB_V1_RuntimeSaveNewGameHandoffReceipt_PC34 *out_receipt)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    CSB_V1_SaveHeader header;
    CSB_V1_DungeonData *dungeon;
    int level;
    int party_x;
    int party_y;
    int party_dir;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB LOADSAVE.C F0435 validates the save header before restoring
     * GLOBAL_DATA into the loaded dungeon. CSBWin SaveGame.cpp likewise
     * resumes against an owned dungeon, never a host-made replacement. */
    if (!profile || !first_input || !save_path || !out_receipt ||
        profile->state != CSB_V1_BOOT_STATE_RUNTIME_READY ||
        profile->variant_id != CSB_V1_VARIANT_REFERENCE_I34_EN ||
        !profile->assets_verified || !profile->dungeon_verified ||
        !profile->runtime.dungeon_handle ||
        csb_v1_dungeon_get_current() != profile->runtime.dungeon_handle ||
        !first_input->mapped || !first_input->runtime_state_changed ||
        (!first_input->is_turn && !first_input->is_forward_move)) return 0;

    dungeon = profile->runtime.dungeon_handle;
    level = profile->runtime.current_level;
    party_x = profile->runtime.party_x;
    party_y = profile->runtime.party_y;
    party_dir = profile->runtime.party_dir;
    memset(&header, 0, sizeof(header));
    if (csb_v1_runtime_save_game_to_path(&profile->runtime, save_path) !=
            CSB_V1_SAVE_OK ||
        csb_v1_load_game(save_path, NULL, 0, &header) != CSB_V1_LOAD_OK ||
        header.Magic != CSB_V1_SAVE_MAGIC_CSB ||
        csb_v1_runtime_load_game_from_path(&profile->runtime, save_path) !=
            CSB_V1_LOAD_OK) return 0;
    if (profile->runtime.dungeon_handle != dungeon ||
        csb_v1_dungeon_get_current() != dungeon ||
        profile->runtime.current_level != level ||
        profile->runtime.party_x != party_x ||
        profile->runtime.party_y != party_y ||
        profile->runtime.party_dir != party_dir) return 0;

    out_receipt->valid = 1;
    out_receipt->real_pc34_dungeon = 1;
    out_receipt->first_input_consumed = 1;
    out_receipt->save_written = 1;
    out_receipt->save_header_valid = 1;
    out_receipt->save_reloaded = 1;
    out_receipt->same_dungeon_owner = 1;
    out_receipt->same_dungeon_singleton = 1;
    out_receipt->no_legacy_wrappers = 1;
    out_receipt->game_time = profile->runtime.game_time;
    out_receipt->game_id = header.GameID;
    out_receipt->current_level = level;
    out_receipt->party_x = party_x;
    out_receipt->party_y = party_y;
    out_receipt->party_dir = party_dir;
    out_receipt->source_evidence =
        "ReDMCSB LOADSAVE.C F0435; SAVEHEAD.C F0429/F0430; "
        "CSBWin SaveGame.cpp resume ownership";
    return 1;
}
