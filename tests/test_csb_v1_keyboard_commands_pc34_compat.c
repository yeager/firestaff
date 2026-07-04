#include <stdio.h>
#include <string.h>

#include "csb_v1_boot.h"
#include "csb_v1_keyboard_commands_pc34_compat.h"
#include "m11_game_view.h"

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_map(const char* label,
                      CsbV1KeyboardKeyPc34Compat key,
                      int ctrlDown,
                      M12_MenuInput want)
{
    M12_MenuInput got = M12_MENU_INPUT_NONE;
    int ok = CSB_V1_KeyboardCommandToMenuInputPc34Compat(key, ctrlDown, &got);
    if (!ok || got != want) {
        fprintf(stderr, "FAIL %s ok=%d got=%d want=%d\n",
                label, ok, (int)got, (int)want);
        return 0;
    }
    return 1;
}

static void seed_csb_view(M11_GameViewState* view)
{
    int i;
    memset(view, 0, sizeof(*view));
    view->active = 1;
    strcpy(view->sourceId, "csb");
    view->world.party.championCount = 4;
    view->world.party.activeChampionIndex = 0;
    for (i = 0; i < 4; ++i) {
        char label[8];
        view->world.party.champions[i].present = 1;
        memset(view->world.party.champions[i].name, ' ',
               sizeof(view->world.party.champions[i].name));
        snprintf(label, sizeof(label), "CSB%d", i + 1);
        memcpy(view->world.party.champions[i].name, label, strlen(label));
    }
}

static void test_put_le16(unsigned char* buf, int offset, unsigned int value)
{
    buf[offset + 0] = (unsigned char)(value & 0xffu);
    buf[offset + 1] = (unsigned char)((value >> 8) & 0xffu);
}

static int real_format_square_offset(int x, int y)
{
    return x * 3 + y;
}

static unsigned int make_sensor_target(int x, int y, int cell)
{
    return (unsigned int)(((cell & 3) << 4) |
                          ((x & 0x1f) << 6) |
                          ((y & 0x1f) << 11));
}

static void make_csb_wall_click_dungeon(CSB_V1_DungeonData* dungeon,
                                        unsigned char* raw,
                                        size_t rawSize)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, rawSize);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->level_offsets[0] = 0;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)rawSize;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[3] = 68;
    dungeon->thing_type_counts[3] = 1;

    raw[real_format_square_offset(1, 0)] = 0x10u; /* front wall + thing list */
    raw[real_format_square_offset(2, 0)] = (unsigned char)((4u << 5) | 4u);
    test_put_le16(raw, 60 + 1 * 2, 0);
    test_put_le16(raw, 66, (3u << 10)); /* C03 sensor thing, cell 0 */
    test_put_le16(raw, 68, 0xfffeu);
    test_put_le16(raw, 70, 1u); /* C001 wall-ornament click */
    test_put_le16(raw, 72, 0u); /* DM1_EFFECT_SET << 3 */
    test_put_le16(raw, 74, make_sensor_target(2, 0, 0));
}

int main(void)
{
    M11_GameViewState view;
    M12_MenuInput plainS = M12_MENU_INPUT_NONE;
    CSB_V1_BootProfile boot;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];
    int ok = 1;

    printf("probe=csb_v1_keyboard_commands_pc34_compat\n");
    printf("sourceEvidence=%s\n", CSB_V1_KeyboardCommandSourceEvidencePc34Compat());

    ok &= expect_map("F1 toggles champion 1 inventory",
                     CSB_V1_KEYBOARD_KEY_F1, 0,
                     M12_MENU_INPUT_CHAMPION_1_INVENTORY);
    ok &= expect_map("F2 toggles champion 2 inventory",
                     CSB_V1_KEYBOARD_KEY_F2, 0,
                     M12_MENU_INPUT_CHAMPION_2_INVENTORY);
    ok &= expect_map("F3 toggles champion 3 inventory",
                     CSB_V1_KEYBOARD_KEY_F3, 0,
                     M12_MENU_INPUT_CHAMPION_3_INVENTORY);
    ok &= expect_map("F4 toggles champion 4 inventory",
                     CSB_V1_KEYBOARD_KEY_F4, 0,
                     M12_MENU_INPUT_CHAMPION_4_INVENTORY);
    ok &= expect_map("Escape freezes/unfreezes",
                     CSB_V1_KEYBOARD_KEY_ESCAPE, 0,
                     M12_MENU_INPUT_FREEZE_TOGGLE);
    ok &= expect_map("Return wakes while resting",
                     CSB_V1_KEYBOARD_KEY_RETURN, 0,
                     M12_MENU_INPUT_ACCEPT);
    ok &= expect_map("Enter wakes while resting",
                     CSB_V1_KEYBOARD_KEY_ENTER, 0,
                     M12_MENU_INPUT_ACCEPT);
    ok &= expect_map("Ctrl-S opens disk menu",
                     CSB_V1_KEYBOARD_KEY_S, 1,
                     M12_MENU_INPUT_DISK_MENU);
    ok &= expect_int("plain S is not a CSB command",
                     CSB_V1_KeyboardCommandToMenuInputPc34Compat(
                         CSB_V1_KEYBOARD_KEY_S, 0, &plainS),
                     0);
    ok &= expect_map("Insert turns left",
                     CSB_V1_KEYBOARD_KEY_INSERT, 0,
                     M12_MENU_INPUT_TURN_LEFT);
    ok &= expect_map("Up arrow moves forward",
                     CSB_V1_KEYBOARD_KEY_UP, 0,
                     M12_MENU_INPUT_UP);
    ok &= expect_map("Clr Home turns right",
                     CSB_V1_KEYBOARD_KEY_CLR_HOME, 0,
                     M12_MENU_INPUT_TURN_RIGHT);
    ok &= expect_map("Left arrow moves left",
                     CSB_V1_KEYBOARD_KEY_LEFT, 0,
                     M12_MENU_INPUT_STRAFE_LEFT);
    ok &= expect_map("Down arrow moves backward",
                     CSB_V1_KEYBOARD_KEY_DOWN, 0,
                     M12_MENU_INPUT_DOWN);
    ok &= expect_map("Right arrow moves right",
                     CSB_V1_KEYBOARD_KEY_RIGHT, 0,
                     M12_MENU_INPUT_STRAFE_RIGHT);

    seed_csb_view(&view);
    ok &= expect_int("F3 inventory redraw",
                     M11_GameView_HandleInput(&view,
                         M12_MENU_INPUT_CHAMPION_3_INVENTORY),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("F3 selects champion slot 2",
                     view.world.party.activeChampionIndex, 2);
    ok &= expect_int("F3 opens inventory", view.inventoryPanelActive, 1);
    ok &= expect_int("F3 again redraws",
                     M11_GameView_HandleInput(&view,
                         M12_MENU_INPUT_CHAMPION_3_INVENTORY),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("F3 again closes inventory", view.inventoryPanelActive, 0);

    ok &= expect_int("Escape freezes CSB",
                     M11_GameView_HandleInput(&view,
                         M12_MENU_INPUT_FREEZE_TOGGLE),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("CSB frozen flag set", view.csbGameFrozen, 1);
    ok &= expect_int("movement ignored while frozen",
                     M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP),
                     M11_GAME_INPUT_IGNORED);
    ok &= expect_int("Escape unfreezes CSB",
                     M11_GameView_HandleInput(&view,
                         M12_MENU_INPUT_FREEZE_TOGGLE),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("CSB frozen flag cleared", view.csbGameFrozen, 0);

    ok &= expect_int("Ctrl-S opens CSB disk menu",
                     M11_GameView_HandleInput(&view, M12_MENU_INPUT_DISK_MENU),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("CSB disk menu flag set", view.csbDiskMenuActive, 1);
    ok &= expect_int("movement ignored while disk menu is open",
                     M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP),
                     M11_GAME_INPUT_IGNORED);
    ok &= expect_int("Accept closes disk menu",
                     M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("CSB disk menu flag cleared", view.csbDiskMenuActive, 0);

    view.resting = 1;
    ok &= expect_int("Return wakes resting CSB",
                     M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("CSB resting cleared", view.resting, 0);

    make_csb_wall_click_dungeon(&dungeon, raw, sizeof(raw));
    csb_v1_boot_profile_init(&boot);
    boot.runtime.chaos_magic.magic_initialized = 1;
    boot.runtime.dungeon_handle = &dungeon;
    boot.runtime.current_level = 0;
    boot.runtime.party_x = 1;
    boot.runtime.party_y = 1;
    boot.runtime.party_dir = 0;
    seed_csb_view(&view);
    view.sourceKind = M11_GAME_SOURCE_CSB_BOOT;
    view.csbBootProfile = &boot;
    view.csbState.level_loaded = 1;
    view.csbState.party_x = boot.runtime.party_x;
    view.csbState.party_y = boot.runtime.party_y;
    view.csbState.party_dir = boot.runtime.party_dir;
    ok &= expect_int("CSB M11 wall ornament click redraw",
                     M11_GameView_HandlePointer(&view, 100, 73, 1),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("CSB M11 wall ornament click queues sensor event",
                     boot.runtime.timeline_queue.eventCount, 1);
    ok &= expect_int("CSB queued wall click event fires",
                     csb_v1_runtime_tick_v1(&boot.runtime), 1);
    ok &= expect_int("CSB wall click opens target door",
                     raw[real_format_square_offset(2, 0)] & 7, 3);
    view.csbBootProfile = NULL;

    return ok ? 0 : 1;
}
