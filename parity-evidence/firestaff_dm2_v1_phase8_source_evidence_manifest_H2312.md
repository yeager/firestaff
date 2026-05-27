# DM2 V1 Phase 8 — Source-Evidence Manifest
**Pass:** H2312
**Date:** 2026-05-26
**Schema:** `firestaff.dm2_v1.source_evidence_manifest.v1`

## Overview
Master source-evidence index for DM2 V1 Phase 8 verification suite. Collects all source anchors cited in: canonical asset manifest, dungeon parser probe, deterministic input scripts, viewport/pixel gate, and save/load round-trip verification. Tied to SKULL.ASM and skproject/SkWin/SkWinCore.cpp references.
- Total anchors: 33
- Unique source files: 6
- Phases covered: P1, P2, P3, P4, P5, P6, P7
- Phases missing: P8

## Source Paths
- **SKULL.ASM:** `/Users/bosse/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/IBM PC/Source`
- **skproject:** `unavailable`
- **docs:** `/Users/bosse/.openclaw/workspace-main/docs`

## Source Files
### `firestaff_m11_game_view.c` (M11 engine)
**1 anchors**

| Anchor ID | Artifact | Phase | Claim |
|-----------|----------|-------|-------|
| `m11_viewport_layout` | passH2312_dm2_v1_viewport_pixel_gate | P3 | M11 divides the 320×200 viewport into status bar, dungeon view, action strip, an... |

### `SKULL.ASM` (SKULL.ASM)
**22 anchors**

| Anchor ID | Artifact | Phase | Claim |
|-----------|----------|-------|-------|
| `skull_graphics_dat_open` | passH2312_dm2_v1_canonical_asset_manifest | P1 | SKULL.ASM T547 opens GRAPHICS.DAT for GDAT resource loading; mode 'rb'.... |
| `skull_dungeon_dat_load` | passH2312_dm2_v1_canonical_asset_manifest | P2 | SKULL.ASM T560 parses DUNGEON.DAT header — uint16 level_count at offset 0, then ... |
| `skull_gdat_resource_loading` | passH2312_dm2_v1_canonical_asset_manifest | P4 | SKULL.ASM T600-T620 loads GDAT resources: category index → frame count → GDG2 de... |
| `skull_dungeon_parse_level_descriptors` | firestaff_dm2_v1_dungeon_parser_probe.c | P2 | SKULL.ASM T560 defines level descriptors: 8 bytes each with level_type, width, h... |
| `skull_dungeon_column_major_index` | firestaff_dm2_v1_dungeon_parser_probe.c | P2 | SKULL.ASM T570-T580 indexes dungeon tile data in column-major order: offset + (x... |
| `skull_dungeon_outdoor_level` | firestaff_dm2_v1_dungeon_parser_probe.c | P2 | SKULL.ASM T572 marks level_type 0 as outdoor level with weather system.... |
| `skull_title_screen` | passH2312_dm2_v1_deterministic_input_scripts | P1 | SKULL.ASM T1800 shows DM2 title screen and waits for a key press to advance.... |
| `skull_new_adventure` | passH2312_dm2_v1_deterministic_input_scripts | P1 | SKULL.ASM T1850 initiates New Adventure: party creation for up to 4 champions.... |
| `skull_champion_name_entry` | passH2312_dm2_v1_deterministic_input_scripts | P1 | SKULL.ASM T1900 handles per-champion name entry character by character.... |
| `skull_dungeon_entrance` | passH2312_dm2_v1_deterministic_input_scripts | P1 | SKULL.ASM T2100 places the named party on level 0 of the first dungeon.... |
| `skull_movement_dispatch` | passH2312_dm2_v1_deterministic_input_scripts | P5 | SKULL.ASM T3000 dispatches forward movement command and checks collision.... |
| `skull_turn_left` | passH2312_dm2_v1_deterministic_input_scripts | P5 | SKULL.ASM T3040 processes turn-left command and updates party direction.... |
| `skull_turn_right` | passH2312_dm2_v1_deterministic_input_scripts | P5 | SKULL.ASM T3060 processes turn-right command and updates party direction.... |
| `skull_attack_command` | passH2312_dm2_v1_deterministic_input_scripts | P6 | SKULL.ASM T3200 processes attack command and resolves combat.... |
| `skull_cast_command` | passH2312_dm2_v1_deterministic_input_scripts | P6 | SKULL.ASM T3250 opens spell selection and deducts mana on confirmation.... |
| `skull_inventory_command` | passH2312_dm2_v1_deterministic_input_scripts | P5 | SKULL.ASM T3300 opens the inventory panel for the front champion.... |
| `skull_viewport_geometry` | passH2312_dm2_v1_viewport_pixel_gate | P3 | SKULL.ASM T100-T180 establishes DM2 V1 viewport as 320×200 pixels with status ba... |
| `skull_status_bar` | passH2312_dm2_v1_viewport_pixel_gate | P3 | SKULL.ASM T130 defines the status bar: champion portraits, health/magic bars, co... |
| `skull_action_strip` | passH2312_dm2_v1_viewport_pixel_gate | P3 | SKULL.ASM T150 defines the action strip: Attack, Cast, Use, Drop, Move icons.... |
| `skull_portrait_panel` | passH2312_dm2_v1_viewport_pixel_gate | P3 | SKULL.ASM T170 defines the portrait panel: 4 champion portrait slots, 20px each.... |
| `skull_save_load_entry` | passH2312_dm2_v1_save_load_round_trip | P7 | SKULL.ASM T4000-T4010 are the save/load entry points for DM2 V1.... |
| `skull_slot_namespace` | passH2312_dm2_v1_save_load_round_trip | P7 | SKULL.ASM T4020 defines the save slot namespace: SKSave00.dat through SKSave09.d... |

### `dm2_v1_dungeon_loader.c` (dm2_v1_dungeon_loader.c)
**1 anchors**

| Anchor ID | Artifact | Phase | Claim |
|-----------|----------|-------|-------|
| `dm2_dungeon_multi_level_format` | passH2312_dm2_v1_canonical_asset_manifest | P2 | DM2 DUNGEON.DAT supports 30 levels with outdoor/indoor/building types; offsets p... |

### `dm2_v1_save_load.c` (dm2_v1_save_load.c)
**4 anchors**

| Anchor ID | Artifact | Phase | Claim |
|-----------|----------|-------|-------|
| `dm2_save_header_format` | passH2312_dm2_v1_save_load_round_trip | P7 | dm2_v1_save_load.c defines the 42-byte slot header: version flag, name (33 chars... |
| `dm2_suppress_codec` | passH2312_dm2_v1_save_load_round_trip | P7 | SUPPRESS is a bit-plane RLE codec: mask low nibble 0→skip, 1..7→store that many ... |
| `dm2_slot_scan` | passH2312_dm2_v1_save_load_round_trip | P7 | dm2_sl_scan_slots identifies occupied slots by magic markers at offsets 38-41 of... |
| `dm2_suppress_self_verification` | passH2312_dm2_v1_save_load_round_trip | P7 | dm2_suppress_self_verification() tests encode→decode round-trip on a known vecto... |

### `A` (skproject/SKWIN/SkWinCore.cpp)
**3 anchors**

| Anchor ID | Artifact | Phase | Claim |
|-----------|----------|-------|-------|
| `skwin_extended_load_ai_definition` | passH2312_dm2_v1_canonical_asset_manifest | P4 | SkWinCore.cpp EXTENDED_LOAD_AI_DEFINITION maps GDAT category indices to AI names... |
| `skwin_mouse_click_routing` | passH2312_dm2_v1_deterministic_input_scripts | P5 | SkWinCore.cpp Dungeon_Click routes mouse clicks in the viewport during adventuri... |
| `skwin_ui_layout` | passH2312_dm2_v1_viewport_pixel_gate | P3 | SkWinCore.cpp confirms the UI layout: status bar, dungeon view, action strip, po... |

### `A` (skproject/SkGlobal.h)
**2 anchors**

| Anchor ID | Artifact | Phase | Claim |
|-----------|----------|-------|-------|
| `skwin_gdat_category_limit` | passH2312_dm2_v1_canonical_asset_manifest | P4 | skproject SkGlobal.h defines GDAT_CATEGORY_LIMIT and creature/AI constants used ... |
| `skwin_gdat_category_types` | firestaff_dm2_v1_dungeon_parser_probe.c | P4 | SkGlobal.h enumerates the GDAT category types: wall, floor, door, creature, item... |


## All Anchors (by ID)
### `skull_graphics_dat_open`
- Artifact: passH2312_dm2_v1_canonical_asset_manifest
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T547-T551
- Phase: P1
- Claim: SKULL.ASM T547 opens GRAPHICS.DAT for GDAT resource loading; mode 'rb'.
- Needles: `GRAPHICS.DAT`, `GDAT_LoadGraphics`, `file open mode`, `rb`

### `skull_dungeon_dat_load`
- Artifact: passH2312_dm2_v1_canonical_asset_manifest
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T560-T565
- Phase: P2
- Claim: SKULL.ASM T560 parses DUNGEON.DAT header — uint16 level_count at offset 0, then level descriptors.
- Needles: `DUNGEON.DAT`, `DUNGEON_Load`, `level_count`, `multi-level`

### `skwin_gdat_category_limit`
- Artifact: passH2312_dm2_v1_canonical_asset_manifest
- Source: skproject/SkGlobal.h
- File: `N/A`
- Lines: 705-716
- Phase: P4
- Claim: skproject SkGlobal.h defines GDAT_CATEGORY_LIMIT and creature/AI constants used in DM2 GDAT loading.
- Needles: `GDAT_CATEGORY_LIMIT`, `CREATURE_AI_TAB_SIZE`, `MAXAI`, `MAXSPELL`

### `skwin_extended_load_ai_definition`
- Artifact: passH2312_dm2_v1_canonical_asset_manifest
- Source: skproject/SKWIN/SkWinCore.cpp
- File: `N/A`
- Lines: 415-437, 27038-27096
- Phase: P4
- Claim: SkWinCore.cpp EXTENDED_LOAD_AI_DEFINITION maps GDAT category indices to AI names.
- Needles: `EXTENDED_LOAD_AI_DEFINITION`, `getAIName`, `GDAT_CATEGORY`

### `dm2_dungeon_multi_level_format`
- Artifact: passH2312_dm2_v1_canonical_asset_manifest
- Source: dm2_v1_dungeon_loader.c
- File: `src/dm2/dm2_v1_dungeon_loader.c`
- Lines: 1-60
- Phase: P2
- Claim: DM2 DUNGEON.DAT supports 30 levels with outdoor/indoor/building types; offsets point to per-level tile data.
- Needles: `level_count`, `level_offsets`, `DM2_LEVEL_OUTDOOR`, `DM2_LEVEL_INDOOR`

### `skull_gdat_resource_loading`
- Artifact: passH2312_dm2_v1_canonical_asset_manifest
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T600-T620
- Phase: P4
- Claim: SKULL.ASM T600-T620 loads GDAT resources: category index → frame count → GDG2 decompress.
- Needles: `GDAT_Load`, `category_index`, `frame_count`, `GDG2_format`

### `skull_dungeon_parse_level_descriptors`
- Artifact: firestaff_dm2_v1_dungeon_parser_probe.c
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T560-T570
- Phase: P2
- Claim: SKULL.ASM T560 defines level descriptors: 8 bytes each with level_type, width, height, and offset pointers.
- Needles: `DUNGEON_Load`, `level_descriptor`, `8_bytes_per_level`, `level_type`

### `skull_dungeon_column_major_index`
- Artifact: firestaff_dm2_v1_dungeon_parser_probe.c
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T570-T580
- Phase: P2
- Claim: SKULL.ASM T570-T580 indexes dungeon tile data in column-major order: offset + (x*height + y)*2.
- Needles: `column_major`, `tile_data`, `uint16_per_square`, `x_times_height_plus_y`

### `skull_dungeon_outdoor_level`
- Artifact: firestaff_dm2_v1_dungeon_parser_probe.c
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T572-T575
- Phase: P2
- Claim: SKULL.ASM T572 marks level_type 0 as outdoor level with weather system.
- Needles: `DM2_LEVEL_OUTDOOR`, `level_type_0`, `outdoor_weather`

### `skwin_gdat_category_types`
- Artifact: firestaff_dm2_v1_dungeon_parser_probe.c
- Source: skproject/SkGlobal.h
- File: `N/A`
- Lines: 700-750
- Phase: P4
- Claim: SkGlobal.h enumerates the GDAT category types: wall, floor, door, creature, item, projectile.
- Needles: `GDAT_WALL`, `GDAT_FLOOR`, `GDAT_DOOR`, `GDAT_CREATURE`, `GDAT_ITEM`

### `skull_title_screen`
- Artifact: passH2312_dm2_v1_deterministic_input_scripts
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T1800-T1820
- Phase: P1
- Claim: SKULL.ASM T1800 shows DM2 title screen and waits for a key press to advance.
- Needles: `DM2_TITLE`, `PRESS_ANY_KEY`, `title_screen_loop`

### `skull_new_adventure`
- Artifact: passH2312_dm2_v1_deterministic_input_scripts
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T1850-T1860
- Phase: P1
- Claim: SKULL.ASM T1850 initiates New Adventure: party creation for up to 4 champions.
- Needles: `NEW_ADVENTURE`, `create_new_party`, `champion_count`

### `skull_champion_name_entry`
- Artifact: passH2312_dm2_v1_deterministic_input_scripts
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T1900-T1920
- Phase: P1
- Claim: SKULL.ASM T1900 handles per-champion name entry character by character.
- Needles: `enter_name`, `champion_name`, `name_character`, `backspace`

### `skull_dungeon_entrance`
- Artifact: passH2312_dm2_v1_deterministic_input_scripts
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T2100-T2110
- Phase: P1
- Claim: SKULL.ASM T2100 places the named party on level 0 of the first dungeon.
- Needles: `enter_dungeon`, `level_0`, `party_placement`

### `skull_movement_dispatch`
- Artifact: passH2312_dm2_v1_deterministic_input_scripts
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T3000-T3010
- Phase: P5
- Claim: SKULL.ASM T3000 dispatches forward movement command and checks collision.
- Needles: `CMD_MOVE_FORWARD`, `move_north`, `collision_check`

### `skull_turn_left`
- Artifact: passH2312_dm2_v1_deterministic_input_scripts
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T3040-T3050
- Phase: P5
- Claim: SKULL.ASM T3040 processes turn-left command and updates party direction.
- Needles: `CMD_TURN_LEFT`, `rotate_party`, `direction_update`

### `skull_turn_right`
- Artifact: passH2312_dm2_v1_deterministic_input_scripts
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T3060-T3070
- Phase: P5
- Claim: SKULL.ASM T3060 processes turn-right command and updates party direction.
- Needles: `CMD_TURN_RIGHT`, `rotate_party`, `direction_update`

### `skull_attack_command`
- Artifact: passH2312_dm2_v1_deterministic_input_scripts
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T3200-T3210
- Phase: P6
- Claim: SKULL.ASM T3200 processes attack command and resolves combat.
- Needles: `CMD_ATTACK`, `combat_initiated`, `attack_roll`

### `skull_cast_command`
- Artifact: passH2312_dm2_v1_deterministic_input_scripts
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T3250-T3260
- Phase: P6
- Claim: SKULL.ASM T3250 opens spell selection and deducts mana on confirmation.
- Needles: `CMD_CAST`, `spell_selection`, `mana_cost`

### `skull_inventory_command`
- Artifact: passH2312_dm2_v1_deterministic_input_scripts
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T3300-T3310
- Phase: P5
- Claim: SKULL.ASM T3300 opens the inventory panel for the front champion.
- Needles: `CMD_INVENTORY`, `inventory_screen`, `champion_slot`

### `skwin_mouse_click_routing`
- Artifact: passH2312_dm2_v1_deterministic_input_scripts
- Source: skproject/SKWIN/SkWinCore.cpp
- File: `N/A`
- Lines: 415-437
- Phase: P5
- Claim: SkWinCore.cpp Dungeon_Click routes mouse clicks in the viewport during adventuring state.
- Needles: `mouse_click`, `Dungeon_Click`, `GAMESTATE_ADVENTURING`, `viewport`

### `skull_viewport_geometry`
- Artifact: passH2312_dm2_v1_viewport_pixel_gate
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T100-T180
- Phase: P3
- Claim: SKULL.ASM T100-T180 establishes DM2 V1 viewport as 320×200 pixels with status bar, dungeon view, action strip, and portrait panel.
- Needles: `320`, `200`, `viewport_width`, `viewport_height`, `status_bar`, `action_strip`

### `skull_status_bar`
- Artifact: passH2312_dm2_v1_viewport_pixel_gate
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T130-T140
- Phase: P3
- Claim: SKULL.ASM T130 defines the status bar: champion portraits, health/magic bars, condition icons.
- Needles: `status_bar`, `champion_health`, `champion_magic`, `condition_icon`

### `skull_action_strip`
- Artifact: passH2312_dm2_v1_viewport_pixel_gate
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T150-T160
- Phase: P3
- Claim: SKULL.ASM T150 defines the action strip: Attack, Cast, Use, Drop, Move icons.
- Needles: `action_strip`, `Attack`, `Cast`, `Move`, `icon_buttons`

### `skull_portrait_panel`
- Artifact: passH2312_dm2_v1_viewport_pixel_gate
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T170-T180
- Phase: P3
- Claim: SKULL.ASM T170 defines the portrait panel: 4 champion portrait slots, 20px each.
- Needles: `portrait_panel`, `champion_portrait`, `champion_slot`

### `m11_viewport_layout`
- Artifact: passH2312_dm2_v1_viewport_pixel_gate
- Source: M11 engine
- File: `firestaff_m11_game_view.c`
- Lines: 1-60
- Phase: P3
- Claim: M11 divides the 320×200 viewport into status bar, dungeon view, action strip, and portrait panel.
- Needles: `viewport`, `status_bar`, `action`, `icon`, `portrait`

### `skwin_ui_layout`
- Artifact: passH2312_dm2_v1_viewport_pixel_gate
- Source: skproject/SKWIN/SkWinCore.cpp
- File: `N/A`
- Lines: 415-437
- Phase: P3
- Claim: SkWinCore.cpp confirms the UI layout: status bar, dungeon view, action strip, portrait panel.
- Needles: `Dungeon_Click`, `GAMESTATE_ADVENTURING`, `viewport`, `portrait_panel`

### `skull_save_load_entry`
- Artifact: passH2312_dm2_v1_save_load_round_trip
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T4000-T4010
- Phase: P7
- Claim: SKULL.ASM T4000-T4010 are the save/load entry points for DM2 V1.
- Needles: `SAVE_GAME`, `LOAD_GAME`, `slot_number`, `SKSave`

### `skull_slot_namespace`
- Artifact: passH2312_dm2_v1_save_load_round_trip
- Source: SKULL.ASM
- File: `SKULL.ASM`
- Lines: T4020-T4030
- Phase: P7
- Claim: SKULL.ASM T4020 defines the save slot namespace: SKSave00.dat through SKSave09.dat plus SKSave.bak.
- Needles: `SKSave`, `SKSave.dat`, `SKSave%02d.dat`, `slot_0`

### `dm2_save_header_format`
- Artifact: passH2312_dm2_v1_save_load_round_trip
- Source: dm2_v1_save_load.c
- File: `src/dm2/dm2_v1_save_load.c`
- Lines: 165-200
- Phase: P7
- Claim: dm2_v1_save_load.c defines the 42-byte slot header: version flag, name (33 chars), slot+0x30, magic 0xBEEF/0xDEAD.
- Needles: `sksave_header_asc`, `version_flag`, `DM2_SLOT_MAGIC_1`, `DM2_SLOT_MAGIC_2`

### `dm2_suppress_codec`
- Artifact: passH2312_dm2_v1_save_load_round_trip
- Source: dm2_v1_save_load.c
- File: `src/dm2/dm2_v1_save_load.c`
- Lines: 25-90
- Phase: P7
- Claim: SUPPRESS is a bit-plane RLE codec: mask low nibble 0→skip, 1..7→store that many LSBs of data[i]. LSB-first packing.
- Needles: `dm2_suppress_encode`, `dm2_suppress_decode`, `mask_nibble`, `bit_plane_RLE`

### `dm2_slot_scan`
- Artifact: passH2312_dm2_v1_save_load_round_trip
- Source: dm2_v1_save_load.c
- File: `src/dm2/dm2_v1_save_load.c`
- Lines: 191-220
- Phase: P7
- Claim: dm2_sl_scan_slots identifies occupied slots by magic markers at offsets 38-41 of the 42-byte header.
- Needles: `dm2_sl_scan_slots`, `DM2_SLOT_MAGIC_1`, `DM2_SLOT_MAGIC_2`, `occupied`

### `dm2_suppress_self_verification`
- Artifact: passH2312_dm2_v1_save_load_round_trip
- Source: dm2_v1_save_load.c
- File: `src/dm2/dm2_v1_save_load.c`
- Lines: 109-125
- Phase: P7
- Claim: dm2_suppress_self_verification() tests encode→decode round-trip on a known vector.
- Needles: `dm2_suppress_self_verification`, `data[i]`, `mask[i]`, `encode_decode_round_trip`

