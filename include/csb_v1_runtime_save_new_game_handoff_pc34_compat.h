#ifndef CSB_V1_RUNTIME_SAVE_NEW_GAME_HANDOFF_PC34_COMPAT_H
#define CSB_V1_RUNTIME_SAVE_NEW_GAME_HANDOFF_PC34_COMPAT_H

#include <stdint.h>

#include "csb_v1_input_command_bridge_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB LOADSAVE.C F0435 restores save state into the already-loaded
 * dungeon. CSBWin SaveGame.cpp follows the same ownership rule: a save is
 * not an alternate dungeon source. */
typedef struct CSB_V1_RuntimeSaveNewGameHandoffReceipt_PC34 {
    int valid;
    int real_pc34_dungeon;
    int first_input_consumed;
    int save_written;
    int save_header_valid;
    int save_reloaded;
    int same_dungeon_owner;
    int same_dungeon_singleton;
    int no_legacy_wrappers;
    uint32_t game_time;
    uint16_t game_id;
    int current_level;
    int party_x;
    int party_y;
    int party_dir;
    const char *source_evidence;
} CSB_V1_RuntimeSaveNewGameHandoffReceipt_PC34;

int csb_v1_runtime_save_new_game_handoff_after_input_pc34(
    void *boot_profile,
    const CSB_V1_InputCommandBridgeResult *first_input,
    const char *save_path,
    CSB_V1_RuntimeSaveNewGameHandoffReceipt_PC34 *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
