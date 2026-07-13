#include "csb_v1_boot.h"
#include "csb_v1_input_command_bridge_pc34_compat.h"
#include "csb_v1_runtime_save_new_game_handoff_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
static int failures;

#define CHECK(condition, message) do { \
    ++checks; \
    if (condition) printf("  PASS: %s\n", message); \
    else { ++failures; printf("  FAIL: %s\n", message); } \
} while (0)

static const char *data_dir(int argc, char **argv, char *fallback,
                            size_t fallback_size)
{
    const char *value = argc > 1 ? argv[1] : getenv("FIRESTAFF_CSB_PC_DATA");
    const char *home;

    if (value && value[0]) return value;
    home = getenv("HOME");
    if (!home || !home[0]) return NULL;
    snprintf(fallback, fallback_size, "%s/.firestaff/data/csb", home);
    return fallback;
}

int main(int argc, char **argv)
{
    CSB_V1_BootProfile profile;
    CSB_V1_InputCommandBridgeResult first_input;
    CSB_V1_RuntimeSaveNewGameHandoffReceipt_PC34 receipt;
    char fallback[1024];
    char save_path[1024];
    const char *root = data_dir(argc, argv, fallback, sizeof(fallback));
    const char *tmp = getenv("TMPDIR");

    printf("=== CSB V1 PC34 save/new-game handoff probe ===\n\n");
    if (!root || !root[0]) {
        printf("SKIP: no PC34 CSB data directory\n");
        return 0;
    }
    if (!tmp || !tmp[0]) tmp = "/tmp";
    snprintf(save_path, sizeof(save_path),
             "%s/firestaff_csb_pc34_new_game_handoff.fsav", tmp);
    remove(save_path);
    csb_v1_boot_profile_init(&profile);
    CHECK(csb_v1_boot_scan_assets(&profile, root) == 0 &&
              profile.variant_id == CSB_V1_VARIANT_PC34_EN &&
              profile.assets_verified && profile.dungeon_verified,
          "hash-verified PC34 GRAPHICS.DAT and DUNGEON.DAT selected");
    if (!profile.assets_verified ||
        profile.variant_id != CSB_V1_VARIANT_PC34_EN) {
        csb_v1_boot_cleanup(&profile);
        return 1;
    }
    CHECK(csb_v1_boot_enter_game(&profile) == 0 &&
              profile.state == CSB_V1_BOOT_STATE_RUNTIME_READY &&
              profile.runtime.dungeon_handle != NULL,
          "new game owns the real PC34 dungeon before its first input");
    memset(&first_input, 0, sizeof(first_input));
    CHECK(CSB_V1_InputCommandBridge_ProcessMenuInputFromBootProfilePc34Compat(
              &profile, M12_MENU_INPUT_TURN_RIGHT, 0, 0, 0, 0, -1,
              &first_input) == 1 && first_input.mapped && first_input.is_turn &&
              first_input.runtime_state_changed,
          "first PC34 turn input reaches the real new-game runtime");
    CHECK(csb_v1_runtime_save_new_game_handoff_after_input_pc34(
              &profile, &first_input, save_path, &receipt) == 1 &&
              receipt.valid && receipt.real_pc34_dungeon &&
              receipt.save_written && receipt.save_header_valid &&
              receipt.save_reloaded && receipt.same_dungeon_owner &&
              receipt.same_dungeon_singleton && receipt.no_legacy_wrappers,
          "native save reload retains the original PC34 new-game dungeon owner");
    remove(save_path);
    csb_v1_boot_cleanup(&profile);
    printf("\nchecks=%d failures=%d\n", checks, failures);
    return failures ? 1 : 0;
}
