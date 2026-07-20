#include "csb_touch_click_zone_matrix_pc34_compat.h"

#include <string.h>

/* Source-locked CSB per-view click/touch zone inventory — the complete
 * MOUSE_INPUT route-table set of the PC engine build.
 *
 * Route tables: ReDMCSB WIP20210206 Toolchains/Common/Source/COMMAND.C
 * PC-media branch (MEDIA529 covers the I34E family that the PC release
 * builds; the Firestaff CSB lane executes this engine via
 * src/engine/redmcsb_f*.c modules):
 *   - G0445 entrance                  (COMMAND.C:341-353, 5 routes)
 *   - G0446 restart game              (COMMAND.C:354-372, 2 literal routes)
 *   - G0447 primary interface         (COMMAND.C:374-396, 19 routes)
 *   - G0448 secondary movement        (COMMAND.C:397-406, 8 routes)
 *   - G0449 champion inventory        (COMMAND.C:409-451, 38 routes)
 *   - G0452 action-area names         (COMMAND.C:461-466, 4 routes)
 *   - G0453 action-area icons         (COMMAND.C:467-472, 4 routes)
 *   - G0454 spell area                (COMMAND.C:473-483, 9 routes)
 *   - G0455 champion names/hands      (COMMAND.C:484-497, 12 routes)
 *   - G0456 chest panel               (COMMAND.C:497-506, 8 routes)
 *   - G0457 resurrect/reincarnate/cancel (COMMAND.C:507-511, 3 routes)
 *   - G2045 champion rename panel     (COMMAND.C:513-548, 35 routes)
 * Hit-test semantics: F0358_COMMAND_GetCommandFromMouseInput_CPSC
 * (COMMAND.C:1379-1449) — CM1 screen-relative zones test the point
 * directly; CM2 viewport-relative zones test it minus the COORD.C
 * G2067/G2068 viewport origin (0,33); the source returns the first
 * route whose box contains the point and whose Button mask intersects
 * the click status.
 *
 * Geometry provenance (honest, per zone):
 *   - Zone-index routes resolve through the shared I34E layout zone
 *     space (DEFS.H:3748-3937 C002..C701; DEFS.H:3976-4013 MEDIA539
 *     maps M664..M700 to layout zones 570..613).  Rectangles are taken
 *     from the already-extracted layout-696 table (data/
 *     zones_h_reconstruction.json, GRAPHICS.DAT DM1 PC 3.4 English,
 *     SHA256 2c3aa836...) — the same I34E zone indexes the PC CSB
 *     build consumes — and were cross-validated against the Amiga
 *     (G20E/G21E) and Atari ST (A20ED..A22G) CSB literal tables in the
 *     same COMMAND.C, which agree box-for-box on the shared chrome.
 *   - Panel-rooted zones (resurrect/cancel/rename/panel) resolve at
 *     the C100/C101 panel anchor: center (152,89) over the 144x73
 *     panel bitmap gives viewport-local panel origin (80,52).
 *   - C147 freeze-game and the two restart-screen routes are PC
 *     literal boxes in G0447/G0446 (no zone index).
 *   - CSB's own GRAPHICS.DAT (graphic 561 layout) extraction remains
 *     pending: the file is not staged in any permitted location, so
 *     per-file CSB-native rect confirmation is a follow-up
 *     (TODO.md).  CSBWin (Paul Stevens) implements its own dialog
 *     system (EnqueMouseClick/DlgButton) and was not used as a
 *     geometry source for these routes.
 *
 * Coordinate space: 320x200 source screen space; CM2 routes are stored
 * viewport-local (224x136) and translated by the hit-test.
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
    { 27u, 218u, CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 231,  10,  16,  16, "champion3.action_hand",  "COMMAND.C:496 maps C027 to C218; layout-696 C210/C218 gives slot3 action hand; Amiga literal (231,246,10,25) agrees" },

    /* ── G0449 champion inventory (COMMAND.C:409-451, 38 routes) ────
     * Viewport-relative routes keep viewport-local rects (the hit-test
     * subtracts the COORD.C G2067/G2068 viewport origin (0,33)); the
     * right-button close route is the CM1 full-screen box. */
    { 11u,   2u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,   0,   0, 320, 200, "inventory.close_right",  "COMMAND.C:410 maps right-button C011 close-inventory to C002_ZONE_SCREEN (full 320x200)" },
    {140u, 562u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 179,   2,  11,  11, "inventory.save",         "COMMAND.C:411 maps C140 save-game to C562; layout-696 C561/C562 gives viewport-relative icon box; Amiga literal (174,182,36,44) screen agrees modulo grid" },
    {145u, 564u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 190,   2,  19,  11, "inventory.rest",         "COMMAND.C:412 maps C145 rest to C564; layout-696 C563/C564 gives viewport-relative icon box; Amiga literal (188,204,36,44) screen agrees" },
    { 11u, 566u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 209,   2,  11,  11, "inventory.close_icon",   "COMMAND.C:413 maps left-button C011 close-inventory to C566; layout-696 C565/C566 gives viewport-relative icon box; Amiga literal (210,218,36,44) screen agrees" },
    {141u, 568u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 168,   3,   9,   9, "inventory.music",        "COMMAND.C:414-415 maps C141 toggle-music to M701 (PC-only route); layout-696 C567/C568 gives the viewport icon box" },
    { 28u, 507u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   6,  53,  16,  16, "inventory.ready_hand",   "COMMAND.C:416 maps C028 to C507 inventory ready hand; layout-696 C105/C507 gives the viewport-local 16x16 slot; Amiga literal (6,21,86,101) screen agrees" },
    { 29u, 508u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  62,  53,  16,  16, "inventory.action_hand",  "COMMAND.C:417 maps C029 to C508 inventory action hand; layout-696 C105/C508 gives the viewport-local 16x16 slot; Amiga literal (62,77,86,101) screen agrees" },
    { 30u, 509u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  34,  26,  16,  16, "inventory.head",         "COMMAND.C:418 maps C030 to C509 inventory head; layout-696 C105/C509 gives the viewport-local 16x16 slot; Amiga literal (34,49,59,74) screen agrees" },
    { 31u, 510u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  34,  46,  16,  16, "inventory.torso",        "COMMAND.C:419 maps C031 to C510 inventory torso; layout-696 C105/C510 gives the viewport-local 16x16 slot; Amiga literal (34,49,79,94) screen agrees" },
    { 32u, 511u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  34,  66,  16,  16, "inventory.legs",         "COMMAND.C:420 maps C032 to C511 inventory legs; layout-696 C105/C511 gives the viewport-local 16x16 slot; Amiga literal (34,49,99,114) screen agrees" },
    { 33u, 512u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  34,  86,  16,  16, "inventory.feet",         "COMMAND.C:421 maps C033 to C512 inventory feet; layout-696 C105/C512 gives the viewport-local 16x16 slot; Amiga literal (34,49,119,134) screen agrees" },
    { 34u, 513u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   6,  90,  16,  16, "inventory.pouch_2",      "COMMAND.C:422 maps C034 to C513 inventory pouch 2; layout-696 C105/C513 gives the viewport-local 16x16 slot; Amiga literal (6,21,123,138) screen agrees" },
    { 70u, 545u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  56,  13,  16,  16, "inventory.mouth",        "COMMAND.C:423 maps C070 mouth to C545; layout-696 C105/C545 gives the viewport-local 16x16 slot; Amiga literal (56,71,46,61) screen agrees" },
    { 71u, 546u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  12,  13,  16,  16, "inventory.eye",          "COMMAND.C:424 maps C071 eye to C546; layout-696 C105/C546 gives the viewport-local 16x16 slot; Amiga literal (12,27,46,61) screen agrees" },
    { 35u, 514u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  79,  73,  16,  16, "inventory.quiver_line2_1", "COMMAND.C:425 maps C035 to C514 quiver line2 1; layout-696 C105/C514 gives the viewport-local 16x16 slot; Amiga literal (79,94,106,121) screen agrees" },
    { 36u, 515u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  62,  90,  16,  16, "inventory.quiver_line1_2", "COMMAND.C:426 maps C036 to C515 quiver line1 2; layout-696 C105/C515 gives the viewport-local 16x16 slot; Amiga literal (62,77,123,138) screen agrees" },
    { 37u, 516u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  79,  90,  16,  16, "inventory.quiver_line2_2", "COMMAND.C:427 maps C037 to C516 quiver line2 2; layout-696 C105/C516 gives the viewport-local 16x16 slot; Amiga literal (79,94,123,138) screen agrees" },
    { 38u, 517u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   6,  33,  16,  16, "inventory.neck",         "COMMAND.C:428 maps C038 to C517 inventory neck; layout-696 C105/C517 gives the viewport-local 16x16 slot; Amiga literal (6,21,66,81) screen agrees" },
    { 39u, 518u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   6,  73,  16,  16, "inventory.pouch_1",      "COMMAND.C:429 maps C039 to C518 inventory pouch 1; layout-696 C105/C518 gives the viewport-local 16x16 slot; Amiga literal (6,21,106,121) screen agrees" },
    { 40u, 519u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  62,  73,  16,  16, "inventory.quiver_line1_1", "COMMAND.C:430 maps C040 to C519 quiver line1 1; layout-696 C105/C519 gives the viewport-local 16x16 slot; Amiga literal (62,77,106,121) screen agrees" },
    { 41u, 520u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  66,  33,  16,  16, "inventory.backpack_line1_1", "COMMAND.C:431 maps C041 to C520 backpack line1 1; layout-696 C105/C520 gives the viewport-local 16x16 slot; Amiga literal (66,81,66,81) screen agrees" },
    { 42u, 521u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  83,  16,  16,  16, "inventory.backpack_line2_2", "COMMAND.C:432 maps C042 to C521 backpack line2 2; layout-696 C105/C521 gives the viewport-local 16x16 slot; Amiga literal (83,98,49,64) screen agrees" },
    { 43u, 522u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 100,  16,  16,  16, "inventory.backpack_line2_3", "COMMAND.C:433 maps C043 to C522 backpack line2 3; layout-696 C105/C522 gives the viewport-local 16x16 slot" },
    { 44u, 523u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 117,  16,  16,  16, "inventory.backpack_line2_4", "COMMAND.C:434 maps C044 to C523 backpack line2 4; layout-696 C105/C523 gives the viewport-local 16x16 slot" },
    { 45u, 524u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 134,  16,  16,  16, "inventory.backpack_line2_5", "COMMAND.C:435 maps C045 to C524 backpack line2 5; layout-696 C105/C524 gives the viewport-local 16x16 slot" },
    { 46u, 525u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 151,  16,  16,  16, "inventory.backpack_line2_6", "COMMAND.C:436 maps C046 to C525 backpack line2 6; layout-696 C105/C525 gives the viewport-local 16x16 slot" },
    { 47u, 526u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 168,  16,  16,  16, "inventory.backpack_line2_7", "COMMAND.C:437 maps C047 to C526 backpack line2 7; layout-696 C105/C526 gives the viewport-local 16x16 slot" },
    { 48u, 527u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 185,  16,  16,  16, "inventory.backpack_line2_8", "COMMAND.C:438 maps C048 to C527 backpack line2 8; layout-696 C105/C527 gives the viewport-local 16x16 slot" },
    { 49u, 528u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 202,  16,  16,  16, "inventory.backpack_line2_9", "COMMAND.C:439 maps C049 to C528 backpack line2 9; layout-696 C105/C528 gives the viewport-local 16x16 slot" },
    { 50u, 529u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  83,  33,  16,  16, "inventory.backpack_line1_2", "COMMAND.C:440 maps C050 to C529 backpack line1 2; layout-696 C105/C529 gives the viewport-local 16x16 slot" },
    { 51u, 530u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 100,  33,  16,  16, "inventory.backpack_line1_3", "COMMAND.C:441 maps C051 to C530 backpack line1 3; layout-696 C105/C530 gives the viewport-local 16x16 slot" },
    { 52u, 531u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 117,  33,  16,  16, "inventory.backpack_line1_4", "COMMAND.C:442 maps C052 to C531 backpack line1 4; layout-696 C105/C531 gives the viewport-local 16x16 slot" },
    { 53u, 532u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 134,  33,  16,  16, "inventory.backpack_line1_5", "COMMAND.C:443 maps C053 to C532 backpack line1 5; layout-696 C105/C532 gives the viewport-local 16x16 slot" },
    { 54u, 533u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 151,  33,  16,  16, "inventory.backpack_line1_6", "COMMAND.C:444 maps C054 to C533 backpack line1 6; layout-696 C105/C533 gives the viewport-local 16x16 slot" },
    { 55u, 534u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 168,  33,  16,  16, "inventory.backpack_line1_7", "COMMAND.C:445 maps C055 to C534 backpack line1 7; layout-696 C105/C534 gives the viewport-local 16x16 slot" },
    { 56u, 535u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 185,  33,  16,  16, "inventory.backpack_line1_8", "COMMAND.C:446 maps C056 to C535 backpack line1 8; layout-696 C105/C535 gives the viewport-local 16x16 slot" },
    { 57u, 536u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 202,  33,  16,  16, "inventory.backpack_line1_9", "COMMAND.C:447 maps C057 to C536 backpack line1 9; layout-696 C105/C536 gives the viewport-local 16x16 slot" },
    { 81u, 101u, CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  80,  52, 144,  73, "inventory.panel",        "COMMAND.C:448 maps C081 click-in-panel to C101_ZONE_PANEL; layout-696 C100/C101 center anchor (152,89) over the 144x73 panel gives viewport-local (80,52,144,73)" },

    /* ── G0456 chest panel (COMMAND.C:497-506, 8 routes) ──────────── */
    { 58u, 537u, CSB_TOUCH_CLICK_VIEW_PANEL_CHEST_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 117,  59,  16,  16, "inventory.chest_1",      "COMMAND.C:498 maps C058 to C537 chest 1; layout-696 C106/C537 gives viewport-local x=117 y=59 w=16 h=16; Amiga literal (117,132,92,107) screen agrees" },
    { 59u, 538u, CSB_TOUCH_CLICK_VIEW_PANEL_CHEST_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 106,  76,  16,  16, "inventory.chest_2",      "COMMAND.C:499 maps C059 to C538 chest 2; layout-696 C106/C538 gives viewport-local x=106 y=76 w=16 h=16; Amiga literal (106,121,109,124) screen agrees" },
    { 60u, 539u, CSB_TOUCH_CLICK_VIEW_PANEL_CHEST_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 111,  93,  16,  16, "inventory.chest_3",      "COMMAND.C:500 maps C060 to C539 chest 3; layout-696 C106/C539 gives viewport-local x=111 y=93 w=16 h=16; Amiga literal (111,126,126,141) screen agrees" },
    { 61u, 540u, CSB_TOUCH_CLICK_VIEW_PANEL_CHEST_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 128,  98,  16,  16, "inventory.chest_4",      "COMMAND.C:501 maps C061 to C540 chest 4; layout-696 C106/C540 gives viewport-local x=128 y=98 w=16 h=16; Amiga literal (128,143,131,146) screen agrees" },
    { 62u, 541u, CSB_TOUCH_CLICK_VIEW_PANEL_CHEST_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 145, 101,  16,  16, "inventory.chest_5",      "COMMAND.C:502 maps C062 to C541 chest 5; layout-696 C106/C541 gives viewport-local x=145 y=101 w=16 h=16; Amiga literal (145,160,134,149) screen agrees" },
    { 63u, 542u, CSB_TOUCH_CLICK_VIEW_PANEL_CHEST_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 162, 103,  16,  16, "inventory.chest_6",      "COMMAND.C:503 maps C063 to C542 chest 6; layout-696 C106/C542 gives viewport-local x=162 y=103 w=16 h=16; Amiga literal (162,177,136,151) screen agrees" },
    { 64u, 543u, CSB_TOUCH_CLICK_VIEW_PANEL_CHEST_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 179, 104,  16,  16, "inventory.chest_7",      "COMMAND.C:504 maps C064 to C543 chest 7; layout-696 C106/C543 gives viewport-local x=179 y=104 w=16 h=16; Amiga literal (179,194,137,152) screen agrees" },
    { 65u, 544u, CSB_TOUCH_CLICK_VIEW_PANEL_CHEST_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 196, 105,  16,  16, "inventory.chest_8",      "COMMAND.C:505 maps C065 to C544 chest 8; layout-696 C106/C544 gives viewport-local x=196 y=105 w=16 h=16; Amiga literal (196,211,138,153) screen agrees" },

    /* ── G0457 resurrect/reincarnate/cancel panel (COMMAND.C:507-511) ─ */
    {160u, 570u, CSB_TOUCH_CLICK_VIEW_PANEL_RESURRECT_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 104,  53,  55,  57, "panel.resurrect",        "COMMAND.C:508 maps C160 resurrect to M664; DEFS.H:3979 MEDIA539 maps M664 to zone 570; layout-696 C569/C570 gives panel-local (24,1,55,57) at panel origin (80,52); Amiga literal (104,158,86,142) screen agrees" },
    {161u, 571u, CSB_TOUCH_CLICK_VIEW_PANEL_RESURRECT_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 163,  53,  55,  57, "panel.reincarnate",      "COMMAND.C:509 maps C161 reincarnate to M665; DEFS.H:3980 maps M665 to zone 571; layout-696 C569/C571 gives panel-local (83,1,55,57) at panel origin (80,52); Amiga literal (163,217,86,142) screen agrees" },
    {162u, 573u, CSB_TOUCH_CLICK_VIEW_PANEL_RESURRECT_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 104, 113, 114,  11, "panel.cancel",           "COMMAND.C:510 maps C162 cancel to M666; DEFS.H:3981 maps M666 to zone 573; layout-696 C572/C573 gives panel-local (24,61,114,11) at panel origin (80,52); Amiga literal (104,217,146,156) screen agrees" },

    /* ── G0445 entrance (COMMAND.C:341-353, 5 routes; I34E MEDIA731
     * variant carries the C216 quit route) ────────────────────────── */
    {200u, 407u, CSB_TOUCH_CLICK_VIEW_ENTRANCE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 244,  45,  55,  14, "entrance.enter",         "COMMAND.C:342 maps C200 enter-dungeon to C407; layout-696 C406/C407 gives x=244 y=45 w=55 h=14; Amiga literal (244,298,45,58) agrees" },
    {201u, 407u, CSB_TOUCH_CLICK_VIEW_ENTRANCE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, CSB_TOUCH_CLICK_BUTTON_BONUS_DUNGEON_PC34_COMPAT, 244,  45,  55,  14, "entrance.enter_bonus",   "COMMAND.C:343 maps C201 enter-bonus-dungeon to C407 with MASK0x0010_MOUSE_BONUS_DUNGEON; same box as entrance.enter, distinct button" },
    {202u, 409u, CSB_TOUCH_CLICK_VIEW_ENTRANCE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 244,  76,  55,  18, "entrance.resume",        "COMMAND.C:344 maps M566 resume (DEFS.H:382 = 202 for I34E MEDIA405) to C409; layout-696 C408/C409 gives x=244 y=76 w=55 h=18; Amiga literal (244,298,76,93) agrees" },
    {216u, 434u, CSB_TOUCH_CLICK_VIEW_ENTRANCE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 243, 110,  50,  15, "entrance.quit",          "COMMAND.C:345-346 maps C216 quit (I34E MEDIA731 route) to C434; layout-696 C433/C434 gives x=243 y=110 w=50 h=15 (PC-only route, no Amiga literal)" },
    {203u, 411u, CSB_TOUCH_CLICK_VIEW_ENTRANCE_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 248, 186,  46,  14, "entrance.credits",       "COMMAND.C:347 maps M567 draw-credits (DEFS.H:383 = 203 for I34E MEDIA405) to C411; layout-696 C410/C411 bottom-left anchor gives x=248 y=186 w=46 h=14; Amiga literal (248,293,187,199) agrees modulo the PC 46x14 grid" },

    /* ── G0446 restart game (COMMAND.C:354-372; I34E MEDIA730 literal
     * boxes — the only literal-box routes in the PC tables besides
     * freeze) ─────────────────────────────────────────────────────── */
    {215u,   0u, CSB_TOUCH_CLICK_VIEW_RESTART_GAME_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 103, 140, 115,  15, "restart.restart_game",   "COMMAND.C:370 I34E MEDIA730 literal box x=103..217 y=140..154 for C215 restart-game (no zone index)" },
    {216u,   0u, CSB_TOUCH_CLICK_VIEW_RESTART_GAME_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 142, 165,  37,  15, "restart.quit",           "COMMAND.C:371 I34E MEDIA730 literal box x=142..178 y=165..179 for C216 quit (no zone index)" },

    /* ── G2045 champion rename panel (COMMAND.C:513-548, 35 routes) ──
     * Zone numbers via DEFS.H:3982-4013 MEDIA539 (I34E); layout-696
     * records 576-613 are panel-local and resolve at panel origin
     * (80,52) — the 577-615 rename block is present in the extraction. */
    {165u, 577u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 107, 114,  69,   9, "rename.backspace",       "COMMAND.C:514 maps C165 rename-backspace to M667; DEFS.H:3982 maps M667 to zone 577; layout-696 C576/C577 gives panel-local (27,62,69,9) at panel origin (80,52)" },
    {166u, 579u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 197, 114,  19,   9, "rename.ok",              "COMMAND.C:515 maps C166 rename-ok to M668; DEFS.H:3983 maps M668 to zone 579; layout-696 C578/C579 gives panel-local (117,62,19,9)" },
    {167u, 581u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 207,  93,   9,  19, "rename.title",           "COMMAND.C:516 maps C167 rename-title to M669; DEFS.H:3984 maps M669 to zone 581; layout-696 C580/C581 gives panel-local (127,41,9,19)" },
    {168u, 583u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 107,  83,   9,   9, "rename.a",               "COMMAND.C:517 maps C168 rename-A to M670; DEFS.H:3985 maps M670 to zone 583; layout-696 C582/C583 gives panel-local (27,31,9,9)" },
    {169u, 584u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 117,  83,   9,   9, "rename.b",               "COMMAND.C:518 maps C169 to M671 zone 584; layout-696 C582/C584 gives panel-local (37,31,9,9)" },
    {170u, 585u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 127,  83,   9,   9, "rename.c",               "COMMAND.C:519 maps C170 to M672 zone 585; layout-696 C582/C585 gives panel-local (47,31,9,9)" },
    {171u, 586u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 137,  83,   9,   9, "rename.d",               "COMMAND.C:520 maps C171 to M673 zone 586; layout-696 C582/C586 gives panel-local (57,31,9,9)" },
    {172u, 587u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 147,  83,   9,   9, "rename.e",               "COMMAND.C:521 maps C172 to M674 zone 587; layout-696 C582/C587 gives panel-local (67,31,9,9)" },
    {173u, 588u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 157,  83,   9,   9, "rename.f",               "COMMAND.C:522 maps C173 to M675 zone 588; layout-696 C582/C588 gives panel-local (77,31,9,9)" },
    {174u, 589u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 167,  83,   9,   9, "rename.g",               "COMMAND.C:523 maps C174 to M676 zone 589; layout-696 C582/C589 gives panel-local (87,31,9,9)" },
    {175u, 590u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 177,  83,   9,   9, "rename.h",               "COMMAND.C:524 maps C175 to M677 zone 590; layout-696 C582/C590 gives panel-local (97,31,9,9)" },
    {176u, 591u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 187,  83,   9,   9, "rename.i",               "COMMAND.C:525 maps C176 to M678 zone 591; layout-696 C582/C591 gives panel-local (107,31,9,9)" },
    {177u, 592u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 197,  83,   9,   9, "rename.j",               "COMMAND.C:526 maps C177 to M679 zone 592; layout-696 C582/C592 gives panel-local (117,31,9,9)" },
    {178u, 593u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 207,  83,   9,   9, "rename.k",               "COMMAND.C:527 maps C178 to M680 zone 593; layout-696 C582/C593 gives panel-local (127,31,9,9)" },
    {179u, 594u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 107,  93,   9,   9, "rename.l",               "COMMAND.C:528 maps C179 to M681 zone 594; layout-696 C582/C594 gives panel-local (27,41,9,9)" },
    {180u, 595u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 117,  93,   9,   9, "rename.m",               "COMMAND.C:529 maps C180 to M682 zone 595; layout-696 C582/C595 gives panel-local (37,41,9,9)" },
    {181u, 596u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 127,  93,   9,   9, "rename.n",               "COMMAND.C:530 maps C181 to M683 zone 596; layout-696 C582/C596 gives panel-local (47,41,9,9)" },
    {182u, 597u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 137,  93,   9,   9, "rename.o",               "COMMAND.C:531 maps C182 to M684 zone 597; layout-696 C582/C597 gives panel-local (57,41,9,9)" },
    {183u, 598u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 147,  93,   9,   9, "rename.p",               "COMMAND.C:532 maps C183 to M685 zone 598; layout-696 C582/C598 gives panel-local (67,41,9,9)" },
    {184u, 599u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 157,  93,   9,   9, "rename.q",               "COMMAND.C:533 maps C184 to M686 zone 599; layout-696 C582/C599 gives panel-local (77,41,9,9)" },
    {185u, 600u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 167,  93,   9,   9, "rename.r",               "COMMAND.C:534 maps C185 to M687 zone 600; layout-696 C582/C600 gives panel-local (87,41,9,9)" },
    {186u, 601u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 177,  93,   9,   9, "rename.s",               "COMMAND.C:535 maps C186 to M688 zone 601; layout-696 C582/C601 gives panel-local (97,41,9,9)" },
    {187u, 602u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 187,  93,   9,   9, "rename.t",               "COMMAND.C:536 maps C187 to M689 zone 602; layout-696 C582/C602 gives panel-local (107,41,9,9)" },
    {188u, 603u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 197,  93,   9,   9, "rename.u",               "COMMAND.C:537 maps C188 to M690 zone 603; layout-696 C582/C603 gives panel-local (117,41,9,9)" },
    {189u, 604u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 107, 103,   9,   9, "rename.v",               "COMMAND.C:538 maps C189 to M691 zone 604; layout-696 C582/C604 gives panel-local (27,51,9,9)" },
    {190u, 605u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 117, 103,   9,   9, "rename.w",               "COMMAND.C:539 maps C190 to M692 zone 605; layout-696 C582/C605 gives panel-local (37,51,9,9)" },
    {191u, 606u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 127, 103,   9,   9, "rename.x",               "COMMAND.C:540 maps C191 to M693 zone 606; layout-696 C582/C606 gives panel-local (47,51,9,9)" },
    {192u, 607u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 137, 103,   9,   9, "rename.y",               "COMMAND.C:541 maps C192 to M694 zone 607; layout-696 C582/C607 gives panel-local (57,51,9,9)" },
    {193u, 608u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 147, 103,   9,   9, "rename.z",               "COMMAND.C:542 maps C193 to M695 zone 608; layout-696 C582/C608 gives panel-local (67,51,9,9)" },
    {194u, 609u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 157, 103,   9,   9, "rename.comma",           "COMMAND.C:543 maps C194 rename-comma to M696 zone 609; layout-696 C582/C609 gives panel-local (77,51,9,9)" },
    {195u, 610u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 167, 103,   9,   9, "rename.period",          "COMMAND.C:544 maps C195 rename-period to M697 zone 610; layout-696 C582/C610 gives panel-local (87,51,9,9)" },
    {196u, 611u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 177, 103,   9,   9, "rename.semicolon",       "COMMAND.C:545 maps C196 rename-semicolon to M698 zone 611; layout-696 C582/C611 gives panel-local (97,51,9,9)" },
    {197u, 612u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 187, 103,   9,   9, "rename.colon",           "COMMAND.C:546 maps C197 rename-colon to M699 zone 612; layout-696 C582/C612 gives panel-local (107,51,9,9)" },
    {198u, 613u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 197, 103,   9,   9, "rename.space",           "COMMAND.C:547 maps C198 rename-space to M700; DEFS.H:4013 MEDIA539 maps M700 to zone 613; layout-696 C582/C613 gives panel-local (117,51,9,9)" },
    {198u,   2u, CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,   0,   0, 320, 200, "rename.space_right",     "COMMAND.C:548 maps right-button C198 rename-space to C002_ZONE_SCREEN (right-click anywhere types space)" }
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

/* Source viewport origin: COORD.C G2067_i_ViewportScreenX = 0,
 * G2068_i_ViewportScreenY = 33.  F0358 tests CM2 viewport-relative
 * zones against the click point minus this origin. */
#define CSB_VIEWPORT_ORIGIN_X_PC34_COMPAT 0
#define CSB_VIEWPORT_ORIGIN_Y_PC34_COMPAT 33

static int csb_point_in_zone(int x, int y,
                             const CsbTouchClickZonePc34Compat* zone) {
    if (zone->coordMode == TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT) {
        x -= CSB_VIEWPORT_ORIGIN_X_PC34_COMPAT;
        y -= CSB_VIEWPORT_ORIGIN_Y_PC34_COMPAT;
    }
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
        case CSB_TOUCH_CLICK_VIEW_CHAMPION_INVENTORY_PC34_COMPAT:
            return "champion_inventory";
        case CSB_TOUCH_CLICK_VIEW_PANEL_CHEST_PC34_COMPAT:
            return "panel_chest";
        case CSB_TOUCH_CLICK_VIEW_PANEL_RESURRECT_PC34_COMPAT:
            return "panel_resurrect";
        case CSB_TOUCH_CLICK_VIEW_ENTRANCE_PC34_COMPAT:    return "entrance";
        case CSB_TOUCH_CLICK_VIEW_RESTART_GAME_PC34_COMPAT: return "restart_game";
        case CSB_TOUCH_CLICK_VIEW_PANEL_CHAMPION_RENAME_PC34_COMPAT:
            return "panel_champion_rename";
        default: return "unknown";
    }
}

const char* CSB_TOUCHCLICK_Compat_GetSourceEvidence(void) {
    return "ReDMCSB WIP20210206 Toolchains/Common/Source/COMMAND.C PC-media "
           "(MEDIA529/I34E-family) route tables: G0445 entrance "
           "(COMMAND.C:341-353, 5 routes incl. I34E MEDIA731 quit), G0446 "
           "restart game (COMMAND.C:354-372, I34E MEDIA730 literal boxes), "
           "G0447 primary interface (COMMAND.C:374-396, 19 routes), G0448 "
           "secondary movement (COMMAND.C:397-406, 8 routes), G0449 "
           "champion inventory (COMMAND.C:409-451, 38 routes incl. PC-only "
           "C141 music toggle), G0452 action-area names "
           "(COMMAND.C:461-466), G0453 action-area icons "
           "(COMMAND.C:467-472), G0454 spell area (COMMAND.C:473-483), "
           "G0455 champion names/hands (COMMAND.C:484-497), G0456 chest "
           "panel (COMMAND.C:497-506, 8 routes), G0457 "
           "resurrect/reincarnate/cancel panel (COMMAND.C:507-511), "
           "G2045 champion rename panel (COMMAND.C:513-548, 35 routes); "
           "F0358_COMMAND_GetCommandFromMouseInput_CPSC "
           "(COMMAND.C:1379-1449) first-match zone+button hit-test with "
           "CM2 viewport-origin (COORD.C G2067/G2068 = 0,33) translation; "
           "DEFS.H:3748-3937 names C002..C701 zones, DEFS.H:376-383 "
           "MEDIA405 gives I34E command values 201/202/203, DEFS.H:3976-4013 "
           "MEDIA539 maps M664..M700 to layout zones 570..613; zone "
           "rectangles resolved from the shared I34E layout zone space via "
           "the layout-696 extraction (data/zones_h_reconstruction.json, "
           "GRAPHICS.DAT DM1 PC 3.4 English SHA256 2c3aa836...; panel-"
           "rooted zones resolve at panel anchor C100/C101 = viewport-"
           "local (80,52,144,73)) and cross-validated against the Amiga "
           "G20E/G21E and Atari ST A20ED..A22G CSB literal tables in the "
           "same COMMAND.C; C147 freeze-game and the restart-screen boxes "
           "are PC literals; CSB-native GRAPHICS.DAT graphic-561 layout "
           "extraction is a tracked follow-up (file not staged).";
}
