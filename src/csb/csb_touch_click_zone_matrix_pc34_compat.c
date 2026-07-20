#include "csb_touch_click_zone_matrix_pc34_compat.h"

#include <string.h>

/* Source-locked CSB dungeon-view click/touch zone inventory.
 *
 * Route tables: ReDMCSB WIP20210206 Toolchains/Common/Source/COMMAND.C
 * PC-media branch (MEDIA529 covers the I34E family that the PC release
 * builds; the Firestaff CSB lane executes this engine via
 * src/engine/redmcsb_f*.c):
 *   - G0447 primary interface table   (COMMAND.C:374-396, 19 routes)
 *   - G0448 secondary movement table  (COMMAND.C:397-406, 8 routes)
 *   - G0452 action-area names         (COMMAND.C:461-466, 4 routes)
 *   - G0453 action-area icons         (COMMAND.C:467-472, 4 routes)
 *   - G0454 spell area                (COMMAND.C:473-483, 9 routes)
 *   - G0455 champion names/hands      (COMMAND.C:484-497, 12 routes)
 * Hit-test semantics: F0358_COMMAND_GetCommandFromMouseInput_CPSC
 * (COMMAND.C:1379-1449) — CM1 screen-relative zones test the point
 * directly; the source returns the first route whose box contains the
 * point and whose Button mask intersects the click status.
 *
 * Geometry provenance (honest, per zone):
 *   - Zone-index routes resolve through the shared I34E layout zone
 *     space (DEFS.H:3748-3937 C002..C701).  Rectangles are taken from
 *     the already-extracted layout-696 table (data/
 *     zones_h_reconstruction.json, GRAPHICS.DAT DM1 PC 3.4 English,
 *     SHA256 2c3aa836...) — the same I34E zone indexes the PC CSB
 *     build consumes — and were cross-validated against the Amiga
 *     (G20E/G21E) and Atari ST (A20ED..A22G) CSB literal tables in the
 *     same COMMAND.C, which agree box-for-box on the dungeon chrome.
 *   - C147 freeze-game is a PC literal box in G0447 (no zone index).
 *   - CSB's own GRAPHICS.DAT (graphic 561 layout) extraction remains
 *     pending: the file is not staged in any permitted location this
 *     round, so per-file CSB-native rect confirmation is a follow-up
 *     (TODO.md).  CSBWin (Paul Stevens) implements its own dialog
 *     system (EnqueMouseClick/DlgButton) and was not used as a
 *     geometry source for these routes.
 *
 * Coordinate space: 320x200 source screen space; all dungeon-view
 * routes are CM1 screen-relative so no viewport translation is needed.
 *
 * The matrix is evidence/probe data only; it does not replace keyboard
 * routing or existing command bridges. */
static const CsbTouchClickZonePc34Compat kCsbTouchClickZones[] = {
    /* ── G0447 primary interface (COMMAND.C:374-396) ─────────────── */
    {  7u, 151u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,   0,   0,  67,  29, "champion0.toggle_box",    "COMMAND.C:375 maps right-button C007 toggle-inventory champion 0 to C151; layout-696 C150/C151 gives slot0 status box; Amiga G20E literal (0,42,0,28) agrees" },
    {  8u, 152u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,  69,   0,  67,  29, "champion1.toggle_box",    "COMMAND.C:376 maps right-button C008 toggle-inventory champion 1 to C152; layout-696 C150/C152 gives slot1 status box; Amiga literal (69,111,0,28) agrees" },
    {  9u, 153u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, 138,   0,  67,  29, "champion2.toggle_box",    "COMMAND.C:377 maps right-button C009 toggle-inventory champion 2 to C153; layout-696 C150/C153 gives slot2 status box; Amiga literal (138,180,0,28) agrees" },
    { 10u, 154u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, 207,   0,  67,  29, "champion3.toggle_box",    "COMMAND.C:378 maps right-button C010 toggle-inventory champion 3 to C154; layout-696 C150/C154 gives slot3 status box; Amiga literal (207,249,0,28) agrees" },
    {  7u, 187u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   43,   0,  24,  29, "champion0.bar_graphs_toggle", "COMMAND.C:380 maps left-button C007 toggle-inventory champion 0 to C187; layout-696 C183/C187 gives x=43 y=0 w=24 h=29; Amiga C007 literal (43,66,0,28) agrees" },
    {  8u, 188u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  112,   0,  24,  29, "champion1.bar_graphs_toggle", "COMMAND.C:381 maps left-button C008 to C188; layout-696 C184/C188 gives x=112 y=0 w=24 h=29; Amiga C008 literal (112,135,0,28) agrees" },
    {  9u, 189u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  181,   0,  24,  29, "champion2.bar_graphs_toggle", "COMMAND.C:382 maps left-button C009 to C189; layout-696 C185/C189 gives x=181 y=0 w=24 h=29; Amiga C009 literal (181,204,0,28) agrees" },
    { 10u, 190u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  250,   0,  24,  29, "champion3.bar_graphs_toggle", "COMMAND.C:383 maps left-button C010 to C190; layout-696 C186/C190 gives x=250 y=0 w=24 h=29; Amiga C010 literal (250,273,0,28) agrees" },
    { 12u, 151u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    0,   0,  67,  29, "champion0.status_box", "COMMAND.C:384 maps left-button C012 click-in-champion-0-status-box to C151; layout-696 C150/C151 gives x=0 y=0 w=67 h=29" },
    { 13u, 152u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   69,   0,  67,  29, "champion1.status_box", "COMMAND.C:385 maps left-button C013 to C152; layout-696 C150/C152 gives x=69 y=0 w=67 h=29" },
    { 14u, 153u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  138,   0,  67,  29, "champion2.status_box", "COMMAND.C:386 maps left-button C014 to C153; layout-696 C150/C153 gives x=138 y=0 w=67 h=29" },
    { 15u, 154u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  207,   0,  67,  29, "champion3.status_box", "COMMAND.C:387 maps left-button C015 to C154; layout-696 C150/C154 gives x=207 y=0 w=67 h=29" },
    {125u, 113u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  281,   0,  19,  14, "champion.icon_top_left", "COMMAND.C:388 maps C125 top-left champion icon to C113; layout-696 C112/C113 gives x=281 y=0 w=19 h=14; Amiga literal (274,299,0,13) is the pre-I34E icon grid" },
    {126u, 114u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  301,   0,  19,  14, "champion.icon_top_right", "COMMAND.C:389 maps C126 to C114; layout-696 C112/C114 gives x=301 y=0 w=19 h=14" },
    {127u, 115u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  301,  15,  19,  14, "champion.icon_bottom_right", "COMMAND.C:390 maps C127 to C115; layout-696 C112/C115 gives x=301 y=15 w=19 h=14" },
    {128u, 116u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  281,  15,  19,  14, "champion.icon_bottom_left", "COMMAND.C:391 maps C128 to C116; layout-696 C112/C116 gives x=281 y=15 w=19 h=14" },
    {100u,  13u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  233,  42,  87,  33, "spell.parent",           "COMMAND.C:392 maps C100 click-in-spell-area to C013_ZONE_SPELL_AREA; layout-696 C012/C013 gives x=233 y=42 w=87 h=33; Amiga C100 literal (233,319,42,73) agrees" },
    {111u,  11u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  233,  77,  87,  45, "action.parent",          "COMMAND.C:393 maps C111 click-in-action-area to C011_ZONE_ACTION_AREA; layout-696 C010/C011 gives x=233 y=77 w=87 h=45; Amiga C111 literal (233,319,77,121) agrees" },
    {147u,   0u, CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    0, 198,   2,   2, "system.freeze_game", "COMMAND.C:394 maps C147 freeze-game to an absolute PC literal screen box x=0..1 y=198..199 in G0447 (no zone index)" },

    /* ── G0448 secondary movement (COMMAND.C:397-406) ────────────── */
    {  1u,  68u, CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  234, 125,  28,  21, "movement.turn_left",      "COMMAND.C:397 maps C001 turn-left to C068_ZONE_TURN_LEFT; layout-696 C009/C065..C068 gives screen arrow box; Amiga literal (234,261,125,145) agrees" },
    {  3u,  70u, CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  263, 125,  27,  21, "movement.forward",        "COMMAND.C:398 maps C003 move-forward to C070; layout-696 C066/C070 gives x=263 y=125 w=27 h=21; Amiga literal (263,289,125,145) agrees" },
    {  2u,  69u, CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  291, 125,  28,  21, "movement.turn_right",     "COMMAND.C:399 maps C002 turn-right to C069; layout-696 C067/C069 gives x=291 y=125 w=28 h=21; Amiga literal (291,318,125,145) agrees" },
    {  6u,  73u, CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  234, 147,  28,  21, "movement.left",           "COMMAND.C:400 maps C006 move-left to C073; layout-696 C065/C073 gives x=234 y=147 w=28 h=21; Amiga literal (234,261,147,167) agrees" },
    {  5u,  72u, CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  263, 147,  27,  21, "movement.backward",       "COMMAND.C:401 maps C005 move-backward to C072; layout-696 C066/C072 gives x=263 y=147 w=27 h=21; Amiga literal (263,289,147,167) agrees" },
    {  4u,  71u, CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  291, 147,  28,  21, "movement.right",          "COMMAND.C:402 maps C004 move-right to C071; layout-696 C067/C071 gives x=291 y=147 w=28 h=21; Amiga literal (291,318,147,167) agrees" },
    { 80u,   7u, CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    0,  33, 224, 136, "viewport.dungeon",       "COMMAND.C:403 maps C080 click-in-dungeon-view to C007_ZONE_VIEWPORT; layout-696 C003/C007 gives x=0 y=33 w=224 h=136; COORD.C G2067/G2068 viewport origin (0,33) agrees" },
    { 83u,   2u, CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,   0,   0, 320, 200, "inventory.toggle_leader", "COMMAND.C:404 maps right-button C083 toggle-inventory-leader to C002_ZONE_SCREEN (full 320x200 screen)" },

    /* ── G0452 action-area names (COMMAND.C:461-466) ─────────────── */
    {112u,  98u, CSB_TOUCH_CLICK_VIEW_ACTION_AREA_NAMES_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 285,  77,  35,   7, "action.pass",            "COMMAND.C:462 maps C112 pass to C098; layout-696 C097/C098 gives x=285 y=77 w=35 h=7; Amiga literal (285,318,77,83) agrees" },
    {113u,  82u, CSB_TOUCH_CLICK_VIEW_ACTION_AREA_NAMES_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 234,  86,  85,  11, "action.row0",            "COMMAND.C:463 maps C113 action-row0 to C082; layout-696 C081/C082 gives x=234 y=86 w=85 h=11; Amiga literal (234,318,86,96) agrees" },
    {114u,  83u, CSB_TOUCH_CLICK_VIEW_ACTION_AREA_NAMES_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 234,  98,  85,  11, "action.row1",            "COMMAND.C:464 maps C114 action-row1 to C083; layout-696 C081/C083 gives x=234 y=98 w=85 h=11; Amiga literal (234,318,98,108) agrees" },
    {115u,  84u, CSB_TOUCH_CLICK_VIEW_ACTION_AREA_NAMES_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 234, 110,  85,  11, "action.row2",            "COMMAND.C:465 maps C115 action-row2 to C084; layout-696 C081/C084 gives x=234 y=110 w=85 h=11; Amiga literal (234,318,110,120) agrees" },

    /* ── G0453 action-area icons (COMMAND.C:467-472) ─────────────── */
    {116u,  89u, CSB_TOUCH_CLICK_VIEW_ACTION_AREA_ICONS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 233,  86,  20,  35, "action.icon0",           "COMMAND.C:468 maps C116 champion-0 action icon to C089; layout-696 C088/C089 gives the champion0 action icon cell; Amiga literal (233,252,86,120) agrees" },
    {117u,  90u, CSB_TOUCH_CLICK_VIEW_ACTION_AREA_ICONS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 255,  86,  20,  35, "action.icon1",           "COMMAND.C:469 maps C117 to C090; layout-696 C088/C090 gives the champion1 action icon cell; Amiga literal (255,274,86,120) agrees" },
    {118u,  91u, CSB_TOUCH_CLICK_VIEW_ACTION_AREA_ICONS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 277,  86,  20,  35, "action.icon2",           "COMMAND.C:470 maps C118 to C091; layout-696 C088/C091 gives the champion2 action icon cell; Amiga literal (277,296,86,120) agrees" },
    {119u,  92u, CSB_TOUCH_CLICK_VIEW_ACTION_AREA_ICONS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 299,  86,  20,  35, "action.icon3",           "COMMAND.C:471 maps C119 to C092; layout-696 C088/C092 gives the champion3 action icon cell; Amiga literal (299,318,86,120) agrees" },

    /* ── G0454 spell area (COMMAND.C:473-483) ────────────────────── */
    {109u, 221u, CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  233,  42,  87,   8, "spell.caster",           "COMMAND.C:474 maps C109 set-magic-caster to C221; layout-696 C220/C221 gives x=233 y=42 w=87 h=8 (PC-only route, no Amiga literal)" },
    {101u, 245u, CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  235,  51,  13,  11, "spell.symbol1",          "COMMAND.C:475 maps C101 spell symbol 1 to C245; layout-696 C244/C245 starts the six 13x11 rune boxes; Amiga literal (235,247,51,61) agrees" },
    {102u, 246u, CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  249,  51,  13,  11, "spell.symbol2",          "COMMAND.C:476 maps C102 to C246; layout-696 C244/C246 gives x=249 y=51 w=13 h=11; Amiga literal (249,261,51,61) agrees" },
    {103u, 247u, CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  263,  51,  13,  11, "spell.symbol3",          "COMMAND.C:477 maps C103 to C247; layout-696 C244/C247 gives x=263 y=51 w=13 h=11; Amiga literal (263,275,51,61) agrees" },
    {104u, 248u, CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  277,  51,  13,  11, "spell.symbol4",          "COMMAND.C:478 maps C104 to C248; layout-696 C244/C248 gives x=277 y=51 w=13 h=11; Amiga literal (277,289,51,61) agrees" },
    {105u, 249u, CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  291,  51,  13,  11, "spell.symbol5",          "COMMAND.C:479 maps C105 to C249; layout-696 C244/C249 gives x=291 y=51 w=13 h=11; Amiga literal (291,303,51,61) agrees" },
    {106u, 250u, CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  305,  51,  13,  11, "spell.symbol6",          "COMMAND.C:480 maps C106 to C250; layout-696 C244/C250 gives x=305 y=51 w=13 h=11; Amiga literal (305,317,51,61) agrees" },
    {108u, 252u, CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  234,  63,  70,  11, "spell.cast",             "COMMAND.C:481 maps C108 cast-spell to C252; layout-696 C251/C252 gives x=234 y=63 w=70 h=11; Amiga literal (234,303,63,73) agrees" },
    {107u, 254u, CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  305,  63,  14,  11, "spell.recant",           "COMMAND.C:482 maps C107 recant-symbol to C254; layout-696 C253/C254 gives x=305 y=63 w=14 h=11; Amiga literal (305,318,63,73) agrees" },

    /* ── G0455 champion names/hands (COMMAND.C:484-497) ──────────── */
    { 16u, 159u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   0,   0,  43,   7, "champion0.name",         "COMMAND.C:485 maps C016 set-leader champion 0 to C159; layout-696 C155/C159 gives slot0 name box; Amiga literal (0,42,0,6) agrees" },
    { 17u, 160u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  69,   0,  43,   7, "champion1.name",         "COMMAND.C:486 maps C017 to C160; layout-696 C156/C160 gives slot1 name box; Amiga literal (69,111,0,6) agrees" },
    { 18u, 161u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 138,   0,  43,   7, "champion2.name",         "COMMAND.C:487 maps C018 to C161; layout-696 C157/C161 gives slot2 name box; Amiga literal (138,180,0,6) agrees" },
    { 19u, 162u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 207,   0,  43,   7, "champion3.name",         "COMMAND.C:488 maps C019 to C162; layout-696 C158/C162 gives slot3 name box; Amiga literal (207,249,0,6) agrees" },
    { 20u, 211u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   4,  10,  16,  16, "champion0.ready_hand",   "COMMAND.C:489 maps C020 slot-box-00 ready hand to C211; layout-696 C207/C211 gives slot0 ready hand; Amiga literal (4,19,10,25) agrees" },
    { 21u, 212u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  24,  10,  16,  16, "champion0.action_hand",  "COMMAND.C:490 maps C021 to C212; layout-696 C207/C212 gives slot0 action hand; Amiga literal (24,39,10,25) agrees" },
    { 22u, 213u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  73,  10,  16,  16, "champion1.ready_hand",   "COMMAND.C:491 maps C022 to C213; layout-696 C208/C213 gives slot1 ready hand; Amiga literal (73,88,10,25) agrees" },
    { 23u, 214u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  93,  10,  16,  16, "champion1.action_hand",  "COMMAND.C:492 maps C023 to C214; layout-696 C208/C214 gives slot1 action hand; Amiga literal (93,108,10,25) agrees" },
    { 24u, 215u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 142,  10,  16,  16, "champion2.ready_hand",   "COMMAND.C:493 maps C024 to C215; layout-696 C209/C215 gives slot2 ready hand; Amiga literal (142,157,10,25) agrees" },
    { 25u, 216u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 162,  10,  16,  16, "champion2.action_hand",  "COMMAND.C:494 maps C025 to C216; layout-696 C209/C216 gives slot2 action hand; Amiga literal (162,177,10,25) agrees" },
    { 26u, 217u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 211,  10,  16,  16, "champion3.ready_hand",   "COMMAND.C:495 maps C026 to C217; layout-696 C210/C217 gives slot3 ready hand; Amiga literal (211,226,10,25) agrees" },
    { 27u, 218u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 231,  10,  16,  16, "champion3.action_hand",  "COMMAND.C:496 maps C027 to C218; layout-696 C210/C218 gives slot3 action hand; Amiga literal (231,246,10,25) agrees" }
};

#define CSB_ZONE_COUNT (sizeof(kCsbTouchClickZones) / sizeof(kCsbTouchClickZones[0]))

unsigned int CSB_TOUCHCLICK_Compat_GetZoneCount(void) {
    return (unsigned int)CSB_ZONE_COUNT;
}

int CSB_TOUCHCLICK_Compat_GetZone(unsigned int ordinal,
                                  CsbTouchClickZonePc34Compat* outZone) {
    if (!outZone || ordinal >= CSB_ZONE_COUNT) return 0;
    *outZone = kCsbTouchClickZones[ordinal];
    return 1;
}

unsigned int CSB_TOUCHCLICK_Compat_GetViewZoneCount(
    CsbTouchClickViewPc34Compat view) {
    unsigned int count = 0;
    unsigned int i;
    for (i = 0; i < CSB_ZONE_COUNT; ++i) {
        if (kCsbTouchClickZones[i].view == view) ++count;
    }
    return count;
}

int CSB_TOUCHCLICK_Compat_GetViewZone(CsbTouchClickViewPc34Compat view,
                                      unsigned int viewOrdinal,
                                      CsbTouchClickZonePc34Compat* outZone) {
    unsigned int seen = 0;
    unsigned int i;
    if (!outZone) return 0;
    for (i = 0; i < CSB_ZONE_COUNT; ++i) {
        if (kCsbTouchClickZones[i].view != view) continue;
        if (seen == viewOrdinal) {
            *outZone = kCsbTouchClickZones[i];
            return 1;
        }
        ++seen;
    }
    return 0;
}

static int csb_point_in_zone(int x, int y,
                             const CsbTouchClickZonePc34Compat* zone) {
    return x >= zone->x && x < zone->x + zone->w &&
           y >= zone->y && y < zone->y + zone->h;
}

int CSB_TOUCHCLICK_Compat_HitTestInView(CsbTouchClickViewPc34Compat view,
                                        int screenX, int screenY,
                                        unsigned int buttonMask,
                                        CsbTouchClickZonePc34Compat* outZone) {
    unsigned int i;
    for (i = 0; i < CSB_ZONE_COUNT; ++i) {
        const CsbTouchClickZonePc34Compat* zone = &kCsbTouchClickZones[i];
        if (zone->view != view) continue;
        if ((zone->buttonMask & buttonMask) == 0) continue;
        if (!csb_point_in_zone(screenX, screenY, zone)) continue;
        if (outZone) *outZone = *zone;
        return 1;
    }
    return 0;
}

const char* CSB_TOUCHCLICK_Compat_GetViewName(
    CsbTouchClickViewPc34Compat view) {
    switch (view) {
        case CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT:    return "movement";
        case CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT:   return "interface";
        case CSB_TOUCH_CLICK_VIEW_ACTION_AREA_NAMES_PC34_COMPAT:
            return "action_area_names";
        case CSB_TOUCH_CLICK_VIEW_ACTION_AREA_ICONS_PC34_COMPAT:
            return "action_area_icons";
        case CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT:  return "spell_area";
        case CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT:
            return "champion_names_hands";
        default: return "unknown";
    }
}

const char* CSB_TOUCHCLICK_Compat_GetSourceEvidence(void) {
    return "ReDMCSB WIP20210206 Toolchains/Common/Source/COMMAND.C PC-media "
           "(MEDIA529/I34E-family) route tables: G0447 primary interface "
           "(COMMAND.C:374-396, 19 routes), G0448 secondary movement "
           "(COMMAND.C:397-406, 8 routes), G0452 action-area names "
           "(COMMAND.C:461-466), G0453 action-area icons "
           "(COMMAND.C:467-472), G0454 spell area (COMMAND.C:473-483), "
           "G0455 champion names/hands (COMMAND.C:484-497); "
           "F0358_COMMAND_GetCommandFromMouseInput_CPSC "
           "(COMMAND.C:1379-1449) first-match zone+button hit-test; "
           "DEFS.H:3748-3937 names C002..C701 zones; zone rectangles "
           "resolved from the shared I34E layout zone space via the "
           "layout-696 extraction (data/zones_h_reconstruction.json, "
           "GRAPHICS.DAT DM1 PC 3.4 English SHA256 2c3aa836...) and "
           "cross-validated against the Amiga G20E/G21E and Atari ST "
           "A20ED..A22G CSB literal tables in the same COMMAND.C; "
           "C147 freeze-game is the COMMAND.C:394 PC literal box; "
           "CSB-native GRAPHICS.DAT graphic-561 layout extraction is a "
           "tracked follow-up (file not staged this round).";
}
