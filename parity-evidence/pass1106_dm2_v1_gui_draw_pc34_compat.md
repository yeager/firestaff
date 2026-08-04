# Pass 1106 -- DM2 V1 GUI Drawing (c_gui_draw.cpp)

## What

Port of skproject `c_gui_draw.cpp` (5860 lines, ~60 functions) to Firestaff pure C
with callback-based architecture. Covers the entire DM2 HUD rendering system:
stat panels, health bars, item icons, inventory, magic map, spell panel,
attack results, container views, and the main right panel update loop.

## Implemented functions

| Function | skproject source | Description |
|----------|-----------------|-------------|
| `dm2_v1_draw_icon_pict_buff` | c_gui_draw.cpp:31 | Blit source bitmap onto buttongroup |
| `dm2_v1_draw_icon_pict_entry` | c_gui_draw.cpp:58 | Query GDAT image and blit |
| `dm2_v1_draw_dialogue_progress` | c_gui_draw.cpp:79 | Loading progress bar |
| `dm2_v1_draw_dialogue_parts_pict` | c_gui_draw.cpp:97 | Dialogue parts blit |
| `dm2_v1_draw_dialogue_pict` | c_gui_draw.cpp:117 | Dialogue image blit |
| `dm2_v1_draw_wake_up_text` | c_gui_draw.cpp:147 | Wake-up text display |
| `dm2_v1_draw_player_3stat_health_bar` | c_gui_draw.cpp:167 | HP/STA/MP bars |
| `dm2_v1_draw_cur_max_hms` | c_gui_draw.cpp:238 | Format "cur/max" text |
| `dm2_v1_draw_player_3stat_text` | c_gui_draw.cpp:260 | Numeric stat text |
| `dm2_v1_draw_player_name_at_cmdslot` | c_gui_draw.cpp:281 | Hero name at cmd slot |
| `dm2_v1_draw_player_damage` | c_gui_draw.cpp:289 | Damage number display |
| `dm2_v1_draw_chip_of_magic_map` | c_gui_draw.cpp:311 | Single map tile chip |
| `dm2_v1_query_gdat_squad_icon` | c_gui_draw.cpp:390 | Squad direction icon |
| `dm2_v1_draw_cryocell_lever` | c_gui_draw.cpp:414 | Cryocell lever state |
| `dm2_v1_draw_charsheet_option_icon` | c_gui_draw.cpp:441 | Character sheet icon |
| `dm2_v1_money_box_survey` | c_gui_draw.cpp:458 | Money box coin survey |
| `dm2_v1_draw_moneybox` | c_gui_draw.cpp:512 | Money box rendering |
| `dm2_v1_guidraw_0b36_0c52` | c_gui_draw.cpp:617 | Buttongroup bitmap setup |
| `dm2_v1_draw_player_3stat_pane` | c_gui_draw.cpp:642 | 3-stat pane background |
| `dm2_v1_guidraw_29ee_00a3` | c_gui_draw.cpp:683 | Buttongroup2 setup |
| `dm2_v1_draw_cmd_slot` | c_gui_draw.cpp:693 | Command slot icon |
| `dm2_v1_draw_spell_to_be_cast` | c_gui_draw.cpp:714 | Spell rune display |
| `dm2_v1_draw_player_attack_dir` | c_gui_draw.cpp:733 | Attack direction indicator |
| `dm2_v1_draw_spell_panel` | c_gui_draw.cpp:805 | Full spell panel |
| `dm2_v1_show_attack_result` | c_gui_draw.cpp:864 | Attack result display |
| `dm2_v1_guidraw_0b36_105b` | c_gui_draw.cpp:1016 | Stretch 4-to-8 blit |
| `dm2_v1_draw_squad_spell_and_leader_icon` | c_gui_draw.cpp:1030 | Squad/leader icons |
| `dm2_v1_guidraw_24a5_0e82` | c_gui_draw.cpp:1126 | Scaled stat bar |
| `dm2_v1_draw_food_water_poison_panel` | c_gui_draw.cpp:1215 | Food/water/poison bars |
| `dm2_v1_draw_item_stats_bar` | c_gui_draw.cpp:1232 | Item stat bar |
| `dm2_v1_guidraw_2405_014a` | c_gui_draw.cpp:1307 | Animated item icon index |
| `dm2_v1_guidraw_2405_011f` | c_gui_draw.cpp:1626 | Rect with inflate |
| `dm2_v1_guidraw_2405_00ec` | c_gui_draw.cpp:1638 | Item blit rect query |
| `dm2_v1_draw_item_in_hand` | c_gui_draw.cpp:1516 | Hand-held item render |
| `dm2_v1_draw_container_panel` | c_gui_draw.cpp:1540 | Container panel |
| `dm2_v1_draw_item_icon` | c_gui_draw.cpp:1656 | Item icon render |
| `dm2_v1_draw_container_survey` | c_gui_draw.cpp:1795 | Container item survey |
| `dm2_v1_draw_eye_mouth_colored_rectangle` | c_gui_draw.cpp:1824 | Eye/mouth color rect |
| `dm2_v1_guidraw_2e62_03b5` | c_gui_draw.cpp:1833 | Hero equipment slot |
| `dm2_v1_draw_scroll_text` | c_gui_draw.cpp:1996 | Scroll text render |
| `dm2_v1_draw_item_survey` | c_gui_draw.cpp:2072 | Item survey panel |
| `dm2_v1_draw_hand_action_icons` | c_gui_draw.cpp:2341 | Hand action icons |
| `dm2_v1_draw_map_chip` | c_gui_draw.cpp:2412 | Map chip (1086 lines) |
| `dm2_v1_guidraw_29ee_1d03` | c_gui_draw.cpp:3499 | Enchantment direction buttons |
| `dm2_v1_guidraw_29ee_1946` | c_gui_draw.cpp:3561 | Magic map tile grid |
| `dm2_v1_draw_majic_map` | c_gui_draw.cpp:3928 | Full magic map |
| `dm2_v1_display_hint_new_line` | c_gui_draw.cpp:4138 | Hint text newline |
| `dm2_v1_display_taken_item_name` | c_gui_draw.cpp:4144 | Taken item name |
| `dm2_v1_highlight_arrow_panel` | c_gui_draw.cpp:4151 | Arrow panel highlight |
| `dm2_v1_display_right_panel_squad_hands` | c_gui_draw.cpp:4181 | Squad hands display |
| `dm2_v1_refresh_player_stat_disp` | c_gui_draw.cpp:4359 | Full stat refresh (~520 lines) |
| `dm2_v1_guidraw_29ee_000f` | c_gui_draw.cpp:4880 | Arrow/movement panel |
| `dm2_v1_guidraw_24a5_1798` | c_gui_draw.cpp:4906 | Hero selection for stats |
| `dm2_v1_update_right_panel` | c_gui_draw.cpp:5182 | Main right panel (~679 lines) |

## Data tables

| Table | Description |
|-------|-------------|
| `dm2_guidraw_table1d275a[32][2]` | Coin scatter positions |
| `dm2_guidraw_table1d69d0[4]` | Hero stat bar color indices |
| `dm2_guidraw_table1d67d9[8]` | Right panel mode flags |
| `dm2_guidraw_v1d1124[2]` | Slash separator string |
| `dm2_guidraw_v1d10f0[2]` | Space string for item survey |

## Callback architecture

~150 function pointers in `DM2_V1_GuiDrawCallbacks` grouped by role:
GDAT queries, graphics/blitting, rect/layout, bitmap management, palette,
hero/party queries, item queries, UI state, buttongroup management,
image system, random, string helpers, and data table queries.

## Test coverage

61 tests: null-safety for all 43 public functions, null-safety for all 10
internal functions, 4 data table verification tests, and 4 callback dispatch tests.
