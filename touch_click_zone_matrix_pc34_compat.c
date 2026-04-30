#include "touch_click_zone_matrix_pc34_compat.h"
#include <string.h>

/* Source-locked starter matrix for the touchscreen/click abstraction.
 *
 * Scope is intentionally narrow: active in-game DM1 V1 mouse/touch hit boxes
 * that are already source-backed by ReDMCSB COMMAND.C route tables and the
 * I34E layout-696 ZONES table.  The matrix is evidence/probe data only; it
 * does not replace keyboard routing or the existing M11 V1 command bridge.
 *
 * Coordinate modes mirror COMMAND.C: CM1 screen-relative entries are 320x200
 * screen boxes; CM2 viewport-relative inventory entries stay viewport-local
 * so callers must opt into screen translation before dispatch. */
static const TouchClickZonePc34Compat kTouchClickZones[] = {
    /* Movement list: COMMAND.C G0448 C001/C003/C002/C006/C005/C004. */
    {  1u,  68u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    234, 125,  28,  21, "movement.turn_left",      "COMMAND.C:396-405 maps movement commands to C068..C073; layout-696 C009/C065..C073 gives screen arrow boxes" },
    {  3u,  70u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    263, 125,  27,  21, "movement.forward",        "COMMAND.C:397-398 maps C003 move-forward to C070; layout-696 C066/C070 gives x=263 y=125 w=27 h=21" },
    {  2u,  69u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    291, 125,  28,  21, "movement.turn_right",     "COMMAND.C:399 maps C002 turn-right to C069; layout-696 C067/C069 gives x=291 y=125 w=28 h=21" },
    {  6u,  73u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    234, 147,  28,  21, "movement.left",           "COMMAND.C:400 maps C006 move-left to C073; layout-696 C065/C073 gives x=234 y=147 w=28 h=21" },
    {  5u,  72u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    263, 147,  27,  21, "movement.backward",       "COMMAND.C:401 maps C005 move-backward to C072; layout-696 C066/C072 gives x=263 y=147 w=27 h=21" },
    {  4u,  71u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    291, 147,  28,  21, "movement.right",          "COMMAND.C:402 maps C004 move-right to C071; layout-696 C067/C071 gives x=291 y=147 w=28 h=21" },
    { 80u,   7u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,      0,  33, 224, 136, "viewport.dungeon",       "COMMAND.C:403 maps C080 to C007_ZONE_VIEWPORT; layout-696 C003/C007 gives x=0 y=33 w=224 h=136" },
    { 83u,   2u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,      0,   0, 320, 200, "inventory.toggle_leader", "COMMAND.C:404 maps right-button C083 toggle-inventory-leader to C002_ZONE_SCREEN" },

    /* Primary interface list: champion HUD, spell parent, action parent. */
    {  7u, 151u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,      0,   0,  67,  29, "champion0.toggle_box",    "COMMAND.C:375-387 maps C007/C012 to C151; layout-696 C150/C151 gives slot0 status box" },
    {  8u, 152u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,     69,   0,  67,  29, "champion1.toggle_box",    "COMMAND.C:376-386 maps C008/C013 to C152; layout-696 C150/C152 gives slot1 status box" },
    {  9u, 153u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,    138,   0,  67,  29, "champion2.toggle_box",    "COMMAND.C:377-386 maps C009/C014 to C153; layout-696 C150/C153 gives slot2 status box" },
    { 10u, 154u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,    207,   0,  67,  29, "champion3.toggle_box",    "COMMAND.C:378-387 maps C010/C015 to C154; layout-696 C150/C154 gives slot3 status box" },
    {100u,  13u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    233,  42,  87,  33, "spell.parent",           "COMMAND.C:392 maps C100 to C013_ZONE_SPELL_AREA; layout-696 C012/C013 gives x=233 y=42 w=87 h=33" },
    {111u,  11u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    233,  77,  87,  45, "action.parent",          "COMMAND.C:393 maps C111 to C011_ZONE_ACTION_AREA; layout-696 C010/C011 gives x=233 y=77 w=87 h=45" },

    /* Action-menu subroutes: COMMAND.C G0452/G0453. */
    {112u,  98u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    285,  77,  35,   7, "action.pass",            "COMMAND.C:461-466 maps C112 pass to C098; layout-696 C097/C098 gives x=285 y=77 w=35 h=7" },
    {113u,  82u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    234,  86,  85,  11, "action.row0",            "COMMAND.C:463 maps C113 action-row0 to C082; layout-696 C081/C082 gives x=234 y=86 w=85 h=11" },
    {114u,  83u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    234,  98,  85,  11, "action.row1",            "COMMAND.C:464 maps C114 action-row1 to C083; layout-696 C081/C083 gives x=234 y=98 w=85 h=11" },
    {115u,  84u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    234, 110,  85,  11, "action.row2",            "COMMAND.C:465 maps C115 action-row2 to C084; layout-696 C081/C084 gives x=234 y=110 w=85 h=11" },
    {116u,  89u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    233,  86,  20,  35, "action.icon0",           "COMMAND.C:467-471 maps C116 to C089; layout-696 C088/C089 gives champion0 action icon cell" },
    {117u,  90u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    255,  86,  20,  35, "action.icon1",           "COMMAND.C:469 maps C117 to C090; layout-696 C088/C090 gives champion1 action icon cell" },
    {118u,  91u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    277,  86,  20,  35, "action.icon2",           "COMMAND.C:470 maps C118 to C091; layout-696 C088/C091 gives champion2 action icon cell" },
    {119u,  92u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    299,  86,  20,  35, "action.icon3",           "COMMAND.C:471 maps C119 to C092; layout-696 C088/C092 gives champion3 action icon cell" },

    /* Spell subroutes: COMMAND.C G0454. */
    {109u, 221u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    233,  42,  87,   8, "spell.caster",           "COMMAND.C:474 maps C109 set-magic-caster to C221; layout-696 C220/C221 gives x=233 y=42 w=87 h=8" },
    {101u, 245u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    235,  51,  13,  11, "spell.symbol1",          "COMMAND.C:475 maps C101 to C245; layout-696 C244/C245 starts the six 13x11 available-rune boxes" },
    {102u, 246u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    249,  51,  13,  11, "spell.symbol2",          "COMMAND.C:476 maps C102 to C246; layout-696 C244/C246 gives x=249 y=51 w=13 h=11" },
    {103u, 247u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    263,  51,  13,  11, "spell.symbol3",          "COMMAND.C:477 maps C103 to C247; layout-696 C244/C247 gives x=263 y=51 w=13 h=11" },
    {104u, 248u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    277,  51,  13,  11, "spell.symbol4",          "COMMAND.C:478 maps C104 to C248; layout-696 C244/C248 gives x=277 y=51 w=13 h=11" },
    {105u, 249u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    291,  51,  13,  11, "spell.symbol5",          "COMMAND.C:479 maps C105 to C249; layout-696 C244/C249 gives x=291 y=51 w=13 h=11" },
    {106u, 250u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    305,  51,  13,  11, "spell.symbol6",          "COMMAND.C:480 maps C106 to C250; layout-696 C244/C250 gives x=305 y=51 w=13 h=11" },
    {108u, 252u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    234,  63,  70,  11, "spell.cast",             "COMMAND.C:481 maps C108 cast to C252; layout-696 C251/C252 gives x=234 y=63 w=70 h=11" },
    {107u, 254u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    305,  63,  14,  11, "spell.recant",           "COMMAND.C:482 maps C107 recant to C254; layout-696 C253/C254 gives x=305 y=63 w=14 h=11" },

    /* Champion names and status hand slots: COMMAND.C G0455. */
    { 16u, 159u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,      0,   0,  43,   7, "champion0.name",         "COMMAND.C:484-488 maps C016..C019 set-leader to C159..C162; layout-696 C155/C159 gives slot0 name box" },
    { 17u, 160u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,     69,   0,  43,   7, "champion1.name",         "COMMAND.C:486 maps C017 set-leader to C160; layout-696 C156/C160 gives slot1 name box" },
    { 18u, 161u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    138,   0,  43,   7, "champion2.name",         "COMMAND.C:487 maps C018 set-leader to C161; layout-696 C157/C161 gives slot2 name box" },
    { 19u, 162u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    207,   0,  43,   7, "champion3.name",         "COMMAND.C:488 maps C019 set-leader to C162; layout-696 C158/C162 gives slot3 name box" },
    { 20u, 211u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,      4,  10,  16,  16, "champion0.ready_hand",   "COMMAND.C:489-496 maps C020..C027 hand slots to C211..C218; layout-696 C207/C211 gives slot0 ready hand" },
    { 21u, 212u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,     24,  10,  16,  16, "champion0.action_hand",  "COMMAND.C:490 maps C021 to C212; layout-696 C207/C212 gives slot0 action hand" },
    { 22u, 213u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,     73,  10,  16,  16, "champion1.ready_hand",   "COMMAND.C:491 maps C022 to C213; layout-696 C208/C213 gives slot1 ready hand" },
    { 23u, 214u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,     93,  10,  16,  16, "champion1.action_hand",  "COMMAND.C:492 maps C023 to C214; layout-696 C208/C214 gives slot1 action hand" },
    { 24u, 215u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    142,  10,  16,  16, "champion2.ready_hand",   "COMMAND.C:493 maps C024 to C215; layout-696 C209/C215 gives slot2 ready hand" },
    { 25u, 216u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    162,  10,  16,  16, "champion2.action_hand",  "COMMAND.C:494 maps C025 to C216; layout-696 C209/C216 gives slot2 action hand" },
    { 26u, 217u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    211,  10,  16,  16, "champion3.ready_hand",   "COMMAND.C:495 maps C026 to C217; layout-696 C210/C217 gives slot3 ready hand" },
    { 27u, 218u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    231,  10,  16,  16, "champion3.action_hand",  "COMMAND.C:496 maps C027 to C218; layout-696 C210/C218 gives slot3 action hand" },

    /* Inventory-mode source-backed toggles/slots remain viewport-relative. */
    { 11u,   2u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,      0,   0, 320, 200, "inventory.close_right",  "COMMAND.C:412 maps right-button C011 close-inventory to C002_ZONE_SCREEN" },
    {140u, 562u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  179,   2,  11,  11, "inventory.save",         "COMMAND.C:413 maps C140 save to C562; layout-696 C561/C562 gives viewport-relative icon box" },
    {145u, 564u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  190,   2,  19,  11, "inventory.rest",         "COMMAND.C:414 maps C145 rest to C564; layout-696 C563/C564 gives viewport-relative icon box" },
    { 11u, 566u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  209,   2,  11,  11, "inventory.close_icon",   "COMMAND.C:415 maps C011 close-inventory to C566; layout-696 C565/C566 gives viewport-relative icon box" },
    {141u, 568u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  168,   3,   9,   9, "inventory.music",        "COMMAND.C:416-417 maps C141 toggle-music to M701; layout-696 C567/C568 is the I34E viewport icon box" },
    { 28u, 507u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    6,  53,  16,  16, "inventory.ready_hand","COMMAND.C:419 maps C028 to C507 inventory ready hand; DATA.C:986 and layout-696 C105/C507 give viewport-local 16x16 slot" },
    { 29u, 508u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   62,  53,  16,  16, "inventory.action_hand","COMMAND.C:420 maps C029 to C508 inventory action hand; DATA.C:987 and layout-696 C105/C508 give viewport-local 16x16 slot" },
    { 30u, 509u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   34,  26,  16,  16, "inventory.head","COMMAND.C:421 maps C030 to C509 inventory head; DATA.C:988 and layout-696 C105/C509 give viewport-local 16x16 slot" },
    { 31u, 510u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   34,  46,  16,  16, "inventory.torso","COMMAND.C:422 maps C031 to C510 inventory torso; DATA.C:989 and layout-696 C105/C510 give viewport-local 16x16 slot" },
    { 32u, 511u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   34,  66,  16,  16, "inventory.legs","COMMAND.C:423 maps C032 to C511 inventory legs; DATA.C:990 and layout-696 C105/C511 give viewport-local 16x16 slot" },
    { 33u, 512u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   34,  86,  16,  16, "inventory.feet","COMMAND.C:424 maps C033 to C512 inventory feet; DATA.C:991 and layout-696 C105/C512 give viewport-local 16x16 slot" },
    { 34u, 513u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    6,  90,  16,  16, "inventory.pouch_2","COMMAND.C:425 maps C034 to C513 inventory pouch 2; DATA.C:992 and layout-696 C105/C513 give viewport-local 16x16 slot" },
    { 35u, 514u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   79,  73,  16,  16, "inventory.quiver_line2_1","COMMAND.C:428 maps C035 to C514 inventory quiver line2 1; DATA.C:993 and layout-696 C105/C514 give viewport-local 16x16 slot" },
    { 36u, 515u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   62,  90,  16,  16, "inventory.quiver_line1_2","COMMAND.C:429 maps C036 to C515 inventory quiver line1 2; DATA.C:994 and layout-696 C105/C515 give viewport-local 16x16 slot" },
    { 37u, 516u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   79,  90,  16,  16, "inventory.quiver_line2_2","COMMAND.C:430 maps C037 to C516 inventory quiver line2 2; DATA.C:995 and layout-696 C105/C516 give viewport-local 16x16 slot" },
    { 38u, 517u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    6,  33,  16,  16, "inventory.neck","COMMAND.C:431 maps C038 to C517 inventory neck; DATA.C:996 and layout-696 C105/C517 give viewport-local 16x16 slot" },
    { 39u, 518u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,    6,  73,  16,  16, "inventory.pouch_1","COMMAND.C:432 maps C039 to C518 inventory pouch 1; DATA.C:997 and layout-696 C105/C518 give viewport-local 16x16 slot" },
    { 40u, 519u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   62,  73,  16,  16, "inventory.quiver_line1_1","COMMAND.C:433 maps C040 to C519 inventory quiver line1 1; DATA.C:998 and layout-696 C105/C519 give viewport-local 16x16 slot" },
    { 41u, 520u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   66,  33,  16,  16, "inventory.backpack_line1_1","COMMAND.C:434 maps C041 to C520 inventory backpack line1 1; DATA.C:999 and layout-696 C105/C520 give viewport-local 16x16 slot" },
    { 42u, 521u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   83,  16,  16,  16, "inventory.backpack_line2_2","COMMAND.C:435 maps C042 to C521 inventory backpack line2 2; DATA.C:1000 and layout-696 C105/C521 give viewport-local 16x16 slot" },
    { 43u, 522u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  100,  16,  16,  16, "inventory.backpack_line2_3","COMMAND.C:436 maps C043 to C522 inventory backpack line2 3; DATA.C:1001 and layout-696 C105/C522 give viewport-local 16x16 slot" },
    { 44u, 523u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  117,  16,  16,  16, "inventory.backpack_line2_4","COMMAND.C:437 maps C044 to C523 inventory backpack line2 4; DATA.C:1002 and layout-696 C105/C523 give viewport-local 16x16 slot" },
    { 45u, 524u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  134,  16,  16,  16, "inventory.backpack_line2_5","COMMAND.C:438 maps C045 to C524 inventory backpack line2 5; DATA.C:1003 and layout-696 C105/C524 give viewport-local 16x16 slot" },
    { 46u, 525u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  151,  16,  16,  16, "inventory.backpack_line2_6","COMMAND.C:439 maps C046 to C525 inventory backpack line2 6; DATA.C:1004 and layout-696 C105/C525 give viewport-local 16x16 slot" },
    { 47u, 526u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  168,  16,  16,  16, "inventory.backpack_line2_7","COMMAND.C:440 maps C047 to C526 inventory backpack line2 7; DATA.C:1005 and layout-696 C105/C526 give viewport-local 16x16 slot" },
    { 48u, 527u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  185,  16,  16,  16, "inventory.backpack_line2_8","COMMAND.C:441 maps C048 to C527 inventory backpack line2 8; DATA.C:1006 and layout-696 C105/C527 give viewport-local 16x16 slot" },
    { 49u, 528u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  202,  16,  16,  16, "inventory.backpack_line2_9","COMMAND.C:442 maps C049 to C528 inventory backpack line2 9; DATA.C:1007 and layout-696 C105/C528 give viewport-local 16x16 slot" },
    { 50u, 529u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   83,  33,  16,  16, "inventory.backpack_line1_2","COMMAND.C:443 maps C050 to C529 inventory backpack line1 2; DATA.C:1008 and layout-696 C105/C529 give viewport-local 16x16 slot" },
    { 51u, 530u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  100,  33,  16,  16, "inventory.backpack_line1_3","COMMAND.C:444 maps C051 to C530 inventory backpack line1 3; DATA.C:1009 and layout-696 C105/C530 give viewport-local 16x16 slot" },
    { 52u, 531u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  117,  33,  16,  16, "inventory.backpack_line1_4","COMMAND.C:445 maps C052 to C531 inventory backpack line1 4; DATA.C:1010 and layout-696 C105/C531 give viewport-local 16x16 slot" },
    { 53u, 532u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  134,  33,  16,  16, "inventory.backpack_line1_5","COMMAND.C:446 maps C053 to C532 inventory backpack line1 5; DATA.C:1011 and layout-696 C105/C532 give viewport-local 16x16 slot" },
    { 54u, 533u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  151,  33,  16,  16, "inventory.backpack_line1_6","COMMAND.C:447 maps C054 to C533 inventory backpack line1 6; DATA.C:1012 and layout-696 C105/C533 give viewport-local 16x16 slot" },
    { 55u, 534u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  168,  33,  16,  16, "inventory.backpack_line1_7","COMMAND.C:448 maps C055 to C534 inventory backpack line1 7; DATA.C:1013 and layout-696 C105/C534 give viewport-local 16x16 slot" },
    { 56u, 535u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  185,  33,  16,  16, "inventory.backpack_line1_8","COMMAND.C:449 maps C056 to C535 inventory backpack line1 8; DATA.C:1014 and layout-696 C105/C535 give viewport-local 16x16 slot" },
    { 57u, 536u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  202,  33,  16,  16, "inventory.backpack_line1_9","COMMAND.C:450 maps C057 to C536 inventory backpack line1 9; DATA.C:1015 and layout-696 C105/C536 give viewport-local 16x16 slot" },
    { 58u, 537u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   37,   7,  16,  16, "inventory.chest_1","COMMAND.C:499 maps C058 to C537 inventory chest 1; DATA.C:1016 and layout-696 C106/C537 give viewport-local 16x16 slot" },
    { 59u, 538u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   26,  24,  16,  16, "inventory.chest_2","COMMAND.C:500 maps C059 to C538 inventory chest 2; DATA.C:1017 and layout-696 C106/C538 give viewport-local 16x16 slot" },
    { 60u, 539u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   31,  41,  16,  16, "inventory.chest_3","COMMAND.C:501 maps C060 to C539 inventory chest 3; DATA.C:1018 and layout-696 C106/C539 give viewport-local 16x16 slot" },
    { 61u, 540u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   48,  46,  16,  16, "inventory.chest_4","COMMAND.C:502 maps C061 to C540 inventory chest 4; DATA.C:1019 and layout-696 C106/C540 give viewport-local 16x16 slot" },
    { 62u, 541u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   65,  49,  16,  16, "inventory.chest_5","COMMAND.C:503 maps C062 to C541 inventory chest 5; DATA.C:1020 and layout-696 C106/C541 give viewport-local 16x16 slot" },
    { 63u, 542u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   82,  51,  16,  16, "inventory.chest_6","COMMAND.C:504 maps C063 to C542 inventory chest 6; DATA.C:1021 and layout-696 C106/C542 give viewport-local 16x16 slot" },
    { 64u, 543u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   99,  52,  16,  16, "inventory.chest_7","COMMAND.C:505 maps C064 to C543 inventory chest 7; DATA.C:1022 and layout-696 C106/C543 give viewport-local 16x16 slot" },
    { 65u, 544u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  116,  53,  16,  16, "inventory.chest_8","COMMAND.C:506 maps C065 to C544 inventory chest 8; DATA.C:1023 and layout-696 C106/C544 give viewport-local 16x16 slot" },
    { 70u, 545u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   56,  13,  16,  16, "inventory.mouth",        "COMMAND.C:426 and 2314-2319 map C070 mouth to C545 and F0349_INVENTORY_ProcessCommand70_ClickOnMouth" },
    { 71u, 546u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,   12,  13,  16,  16, "inventory.eye",          "COMMAND.C:427 and 2318-2320 map C071 eye to C546 and F0352_INVENTORY_ProcessCommand71_ClickOnEye" }
};

unsigned int TOUCHCLICK_Compat_GetZoneCount(void) {
    return (unsigned int)(sizeof(kTouchClickZones) / sizeof(kTouchClickZones[0]));
}

int TOUCHCLICK_Compat_GetZone(unsigned int ordinal, TouchClickZonePc34Compat* outZone) {
    if (!outZone || ordinal >= TOUCHCLICK_Compat_GetZoneCount()) return 0;
    *outZone = kTouchClickZones[ordinal];
    return 1;
}

int TOUCHCLICK_Compat_HitTestInCoordMode(int x,
                                             int y,
                                             TouchClickCoordModePc34Compat coordMode,
                                             unsigned int buttonMask,
                                             TouchClickZonePc34Compat* outZone) {
    unsigned int i;
    const TouchClickZonePc34Compat* best = 0;
    int bestArea = 0x7fffffff;
    for (i = 0; i < TOUCHCLICK_Compat_GetZoneCount(); ++i) {
        const TouchClickZonePc34Compat* z = &kTouchClickZones[i];
        int area;
        if (z->coordMode != coordMode) continue;
        if (buttonMask && !(buttonMask & z->buttonMask)) continue;
        if (x < z->x || y < z->y ||
            x >= z->x + z->w || y >= z->y + z->h) continue;
        area = z->w * z->h;
        if (area < bestArea) {
            best = z;
            bestArea = area;
        }
    }
    if (best) {
        if (outZone) *outZone = *best;
        return 1;
    }
    if (outZone) memset(outZone, 0, sizeof(*outZone));
    return 0;
}

int TOUCHCLICK_Compat_HitTestWithButton(int screenX,
                                        int screenY,
                                        unsigned int buttonMask,
                                        TouchClickZonePc34Compat* outZone) {
    return TOUCHCLICK_Compat_HitTestInCoordMode(
        screenX,
        screenY,
        TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,
        buttonMask,
        outZone);
}

int TOUCHCLICK_Compat_HitTest(int screenX, int screenY, TouchClickZonePc34Compat* outZone) {
    return TOUCHCLICK_Compat_HitTestWithButton(screenX, screenY, 0u, outZone);
}

int TOUCHCLICK_Compat_NormalizeScaledScreenPoint(int physicalX,
                                                 int physicalY,
                                                 int surfaceW,
                                                 int surfaceH,
                                                 int* outScreenX,
                                                 int* outScreenY) {
    const int sourceW = 320;
    const int sourceH = 200;
    int drawW;
    int drawH;
    int drawX;
    int drawY;
    int localX;
    int localY;
    if (!outScreenX || !outScreenY || surfaceW <= 0 || surfaceH <= 0) return 0;

    if ((long long)surfaceW * sourceH <= (long long)surfaceH * sourceW) {
        drawW = surfaceW;
        drawH = (int)(((long long)surfaceW * sourceH) / sourceW);
    } else {
        drawH = surfaceH;
        drawW = (int)(((long long)surfaceH * sourceW) / sourceH);
    }
    if (drawW <= 0 || drawH <= 0) return 0;

    drawX = (surfaceW - drawW) / 2;
    drawY = (surfaceH - drawH) / 2;
    if (physicalX < drawX || physicalY < drawY ||
        physicalX >= drawX + drawW || physicalY >= drawY + drawH) {
        *outScreenX = 0;
        *outScreenY = 0;
        return 0;
    }

    localX = physicalX - drawX;
    localY = physicalY - drawY;
    *outScreenX = (int)(((long long)localX * sourceW) / drawW);
    *outScreenY = (int)(((long long)localY * sourceH) / drawH);
    if (*outScreenX < 0) *outScreenX = 0;
    if (*outScreenY < 0) *outScreenY = 0;
    if (*outScreenX >= sourceW) *outScreenX = sourceW - 1;
    if (*outScreenY >= sourceH) *outScreenY = sourceH - 1;
    return 1;
}

int TOUCHCLICK_Compat_HitTestScaledScreenPoint(int physicalX,
                                               int physicalY,
                                               int surfaceW,
                                               int surfaceH,
                                               unsigned int buttonMask,
                                               TouchClickZonePc34Compat* outZone) {
    int screenX;
    int screenY;
    if (!TOUCHCLICK_Compat_NormalizeScaledScreenPoint(physicalX, physicalY, surfaceW, surfaceH, &screenX, &screenY)) {
        if (outZone) memset(outZone, 0, sizeof(*outZone));
        return 0;
    }
    return TOUCHCLICK_Compat_HitTestWithButton(screenX, screenY, buttonMask, outZone);
}


int TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(int physicalX,
                                                     int physicalY,
                                                     int surfaceW,
                                                     int surfaceH,
                                                     unsigned int buttonMask,
                                                     TouchClickDispatchPc34Compat* outDispatch) {
    int screenX;
    int screenY;
    TouchClickZonePc34Compat zone;
    if (!outDispatch) return 0;
    memset(outDispatch, 0, sizeof(*outDispatch));
    if (buttonMask == 0u) return 0;
    if (!TOUCHCLICK_Compat_NormalizeScaledScreenPoint(physicalX, physicalY, surfaceW, surfaceH, &screenX, &screenY)) {
        return 0;
    }
    if (!TOUCHCLICK_Compat_HitTestWithButton(screenX, screenY, buttonMask, &zone)) {
        return 0;
    }
    outDispatch->screenX = screenX;
    outDispatch->screenY = screenY;
    outDispatch->buttonStatus = buttonMask;
    outDispatch->commandId = zone.commandId;
    outDispatch->zoneIndex = zone.zoneIndex;
    outDispatch->coordMode = zone.coordMode;
    outDispatch->groupName = zone.groupName;
    return 1;
}

const char* TOUCHCLICK_Compat_GetSourceEvidence(void) {
    return "COMMAND.C:375-497 defines active in-game mouse command-to-zone/button tables; CEDT026.C:141-161 registers a mouse handler that forwards raw X/Y/button events to F0359_COMMAND_ProcessClick_CPSC; COMMAND.C:1394-1439 F0358_COMMAND_GetCommandFromMouseInput_CPSC matches normalized 320x200 coordinates and P0724_i_ButtonsStatus against the route Button mask; INPUT.C:641-664 forwards left/right button masks 0x0002/0x0001 with screen coordinates to F0359_COMMAND_ProcessClick_CPSC; COMMAND.C:412-451 defines source-backed inventory toggles/slots; COORD.C:2036-2245 and 2451-2505 define runtime layout record resolution; DEFS.H:3748-3937 names C002..M701 zones; zones_h_reconstruction.json is GRAPHICS.DAT C696 layout-696 for DM1 PC 3.4 English/I34E.";
}
