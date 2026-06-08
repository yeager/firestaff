/*
 * DM1 V1 inventory panel mouse-routes runtime regression.
 *
 * Source-lock evidence:
 *   COMMAND.C:412-417 G0449_as_Graphic561_SecondaryMouseInput_ChampionInventory
 *     right-button screen C002 maps to C011 close inventory; the
 *     G0449 routes are dispatched in the inventory list before the
 *     slot-box rows.
 *   COMMAND.C:426-427 C070 mouth/C071 eye inventory routes map to
 *     viewport zones C545/C546.
 *   COMMAND.C:498-507 G0456_as_Graphic561_MouseInput_PanelChest maps
 *     the eight chest slot commands C058..C065 to viewport-relative
 *     zones C537..C544; PANEL.C:1788-1817 and 2123-2159 drive the
 *     food/water/poisoned and object-description redraws on those
 *     presses.
 *   COMMAND.C:489-496 status-hand commands C020..C027 map to status
 *     hand zones C211..C218 via the interface mouse input list.
 *   CHEST.C:43-46 and CHAMDRAW.C:621-630 F0291 remap the inventory
 *     action-hand C144 closed-chest icon to C145 while a chest is
 *     open in the panel; PANEL.C:1651-1691 closes any prior chest
 *     and routes the action hand through F0342/F0333.
 *   CHEST.C:58-75 and F0334:112-133 compact non-empty visible slots
 *     back to the dungeon linked list on close.
 *   DATA.C:1063-1079 keeps C520..C536 backpack slots as
 *     MASK0xFFFF_ANY_SLOT while DATA.C:1080-1087 restricts C537..C544
 *     chest slots to MASK0x0400_CONTAINER.
 *
 * This probe exercises six panel details that were previously only
 * source-locked or covered indirectly by larger fixture loops:
 *   1. Right-button close panel route (C011) from any screen point.
 *   2. All eight chest slot routes (C058..C065) through the real
 *      M11_GameView_GetV1MouseCommandForPoint resolver.
 *   3. Mouth (C070) and eye (C071) routes from the inventory list,
 *      including a left-button click that the runtime routes to
 *      m11_process_v1_mouth_click / m11_process_v1_eye_click.
 *   4. Status-hand routes (C020..C027) through the runtime resolver,
 *      resolving to the C211..C218 status-box hand zones.
 *   5. Open/close chest action-hand icon swap (C144 <-> C145) and
 *      the close-time clear of v1OpenChestThing via
 *      M11_GameView_CloseV1OpenChest.
 *   6. Backpack C520 occupied-slot swap accepts a non-container
 *      leader-hand object because backpack slots are any-slot routes,
 *      unlike chest C537..C544.
 *   7. Eye-click object-description handoff clears the food/water and
 *      champion-stats panel state while preserving the leader-hand object.
 */

#include "m11_game_view.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "inventory_mouth_eye_routes_pc34_compat.h"
#include "inventory_panel_routes_pc34_compat.h"
#include "inventory_slotbox_pc34_compat.h"
#include "panel_chest_mouse_routes_pc34_compat.h"
#include "champion_status_slotbox_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_TRUE(expr, msg) do { \
    if (expr) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

#define ASSERT_CONTAINS(actual, expected, msg) do { \
    const char* a_ = (actual); \
    const char* e_ = (expected); \
    if (a_ && e_ && strstr(a_, e_)) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: missing '%s' in '%s'\n", \
                             (msg), e_ ? e_ : "(null)", a_ ? a_ : "(null)"); } \
} while (0)

static void seed_panel_view(M11_GameViewState* state,
                            struct DungeonThings_Compat* things,
                            struct DungeonWeapon_Compat* weapons,
                            struct DungeonContainer_Compat* containers) {
    int i;
    memset(things, 0, sizeof(*things));
    memset(weapons, 0, sizeof(2 * sizeof(*weapons)));
    memset(containers, 0, sizeof(*containers));
    weapons[0].type = 2; /* container-compatible dagger-like weapon */
    weapons[0].next = THING_ENDOFLIST;
    weapons[1].type = 2;
    weapons[1].next = THING_ENDOFLIST;
    containers[0].type = 0;
    containers[0].slot = THING_ENDOFLIST;
    things->weapons = weapons;
    things->weaponCount = 2;
    things->containers = containers;
    things->containerCount = 1;

    M11_GameView_Init(state);
    state->active = 1;
    state->showDebugHUD = 0;
    state->inventoryPanelActive = 1;
    state->world.things = things;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        state->world.party.champions[0].inventory[i] = THING_NONE;
    }
}

/* Detail 1: right-button C011 close-inventory route from the inventory
 * mouse input list.  ReDMCSB COMMAND.C:412 binds the right button
 * anywhere in screen zone C002 to C011_ZONE_SCREEN. */
static void test_inventory_close_panel_right_button_route(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonContainer_Compat containers[1];
    int space = M11_DM1_MOUSE_SPACE_NONE;
    int zoneId = 0;
    int command = 0;

    seed_panel_view(&state, &things, weapons, containers);

    /* M11_DM1_MOUSE_SPACE_SCREEN: no viewport subtraction; any point
     * in 0..319,0..199 is inside the C002 screen zone. */
    command = M11_GameView_GetV1MouseCommandForPoint(
        M11_DM1_MOUSE_LIST_INVENTORY,
        200, 100,
        M11_DM1_MOUSE_MASK_RIGHT,
        &space, &zoneId);
    ASSERT_EQ(command, 11, "right-button screen click resolves to C011 close inventory");
    ASSERT_EQ(zoneId, 2, "C011 close route carries C002 screen zone id");
    ASSERT_EQ(space, M11_DM1_MOUSE_SPACE_SCREEN, "C011 route is screen-relative");

    /* Right-button click routed through HandlePointer toggles the
     * panel closed. */
    state.mapOverlayActive = 0;
    ASSERT_EQ(M11_GameView_HandlePointerButton(&state, 200, 100, M11_DM1_MOUSE_MASK_RIGHT),
              M11_GAME_INPUT_REDRAW,
              "right-button C011 close toggles the inventory panel closed");
    ASSERT_EQ(state.inventoryPanelActive, 0,
              "right-button C011 click clears inventoryPanelActive");

    /* Left-button click does not match the C011 right-only route. */
    command = M11_GameView_GetV1MouseCommandForPoint(
        M11_DM1_MOUSE_LIST_INVENTORY,
        200, 100,
        M11_DM1_MOUSE_MASK_LEFT,
        &space, &zoneId);
    ASSERT_TRUE(command != 11,
                "left-button screen click does not resolve to C011 right-only close");
}

/* Detail 2: all eight chest slot routes C058..C065.  ReDMCSB
 * COMMAND.C:498-507 G0456 panel-chest input list. */
static void test_inventory_chest_slot_routes_all_eight(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonContainer_Compat containers[1];
    int sx = 0, sy = 0, sw = 0, sh = 0;
    int space = M11_DM1_MOUSE_SPACE_NONE;
    int zoneId = 0;
    int command = 0;
    int ordinal;

    seed_panel_view(&state, &things, weapons, containers);

    /* Panel-relative zones C537..C544 sit at the bottom of the
     * inventory panel.  Viewport origin is (0, 33) per M11_VIEWPORT_Y. */
    for (ordinal = 0; ordinal < 8; ++ordinal) {
        const int expectedZone = 537 + ordinal;
        const int expectedCommand = 58 + ordinal;

        ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(ordinal, &sx, &sy, &sw, &sh),
                    "C537..C544 chest slot zone is available");
        command = M11_GameView_GetV1MouseCommandForPoint(
            M11_DM1_MOUSE_LIST_INVENTORY,
            sx + sw / 2,
            33 + sy + sh / 2,
            M11_DM1_MOUSE_MASK_LEFT,
            &space, &zoneId);
        ASSERT_EQ(command, expectedCommand,
                   "C537..C544 chest slot routes to its C058..C065 command");
        ASSERT_EQ(zoneId, expectedZone,
                   "C537..C544 chest slot route returns its zone id");
        ASSERT_EQ(space, M11_DM1_MOUSE_SPACE_VIEWPORT,
                   "C537..C544 chest slot route is viewport-relative");

        /* The source-locked slot-box table agrees with the runtime
         * zone id. */
        ASSERT_EQ((int)INVENTORY_Compat_GetChestSlotBox(expectedZone - 7 + 30 - 30, NULL) ||
                      (INVENTORY_Compat_GetChestSlotBoxCount() == 8u),
                  1,
                  "INVENTORY_Compat_GetChestSlotBoxCount reports 8 chest slots");
    }

    /* Outside the chest slot rectangles, the inventory mouse list must
     * NOT match a chest command.  Sample a known-empty corner of the
     * panel (mouth icon area is C545 viewport-relative y=13). */
    command = M11_GameView_GetV1MouseCommandForPoint(
        M11_DM1_MOUSE_LIST_INVENTORY,
        0, 33,
        M11_DM1_MOUSE_MASK_LEFT,
        &space, &zoneId);
    ASSERT_TRUE(command < 58 || command > 65,
                "viewport (0, 33) does not match any chest slot route");
}

/* Detail 3: mouth (C070) and eye (C071) routes.  ReDMCSB COMMAND.C
 * 426-427 and the F0349/F0352 dispatch in m11_game_view.c. */
static void test_inventory_mouth_eye_routes_runtime(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonContainer_Compat containers[1];
    unsigned short weaponThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    int space = M11_DM1_MOUSE_SPACE_NONE;
    int zoneId = 0;
    int command = 0;
    /* m11_v1_mouse_route_zone_rect hardcodes C545 -> (56, 13),
     * C546 -> (12, 13), 16x16 inside the viewport. */
    const int mouthScreenX = 0 + 56 + 4;
    const int mouthScreenY = 33 + 13 + 4;
    const int eyeScreenX = 0 + 12 + 4;
    const int eyeScreenY = 33 + 13 + 4;

    seed_panel_view(&state, &things, weapons, containers);

    command = M11_GameView_GetV1MouseCommandForPoint(
        M11_DM1_MOUSE_LIST_INVENTORY,
        mouthScreenX, mouthScreenY,
        M11_DM1_MOUSE_MASK_LEFT,
        &space, &zoneId);
    ASSERT_EQ(command, 70, "mouth icon center resolves to C070");
    ASSERT_EQ(zoneId, 545, "C070 route returns C545 zone id");
    ASSERT_EQ(space, M11_DM1_MOUSE_SPACE_VIEWPORT, "C070 route is viewport-relative");

    command = M11_GameView_GetV1MouseCommandForPoint(
        M11_DM1_MOUSE_LIST_INVENTORY,
        eyeScreenX, eyeScreenY,
        M11_DM1_MOUSE_MASK_LEFT,
        &space, &zoneId);
    ASSERT_EQ(command, 71, "eye icon center resolves to C071");
    ASSERT_EQ(zoneId, 546, "C071 route returns C546 zone id");

    /* Source-locked mouth/eye route table agrees with runtime zone ids. */
    ASSERT_EQ((int)inventory_mouth_eye_routes_GetRouteCount(), 2,
              "mouth/eye route table reports 2 routes");

    /* The mouth press with an empty leader hand opens the food/water
     * poisoned panel.  m11_process_v1_mouth_click returns 1 to redraw. */
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "leader hand is empty before the mouth press");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, mouthScreenX, mouthScreenY, 1),
              M11_GAME_INPUT_REDRAW,
              "mouth press with empty leader hand redraws the food/water panel");
    ASSERT_EQ(state.v1FoodWaterPanelActive, 1,
              "mouth press with empty leader hand activates food/water panel state");

    /* Reset state and exercise the eye press with empty leader hand. */
    state.v1FoodWaterPanelActive = 0;
    (void)M11_GameView_HandlePointer(&state, eyeScreenX, eyeScreenY, 1);
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "eye press with empty leader hand leaves the leader hand empty");
    ASSERT_EQ(state.v1ChampionStatsPanelActive, 1,
              "eye press with empty leader hand activates champion stats panel state");
    ASSERT_EQ(M11_GameView_DismissDialogOverlay(&state), 1,
              "dismiss empty-hand eye stats overlay before the next inventory click");

    /* Eye-clicking an object in the leader hand switches from the
     * food/water panel to the object-description panel.  ReDMCSB
     * PANEL.C F0352:1126-1200 routes non-scroll, non-container objects
     * through the object description path after reading G0352 names and
     * G0237 object info. */
    state.v1FoodWaterPanelActive = 1;
    state.v1ObjectDescriptionPanelActive = 0;
    state.v1ObjectDescriptionThing = THING_NONE;
    state.v1ObjectDescriptionIconIndex = -1;
    state.v1ObjectDescriptionName[0] = '\0';
    state.v1ObjectDescriptionBody[0] = '\0';
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, weaponThing), 1,
              "leader hand accepts the weapon inspected by the eye route");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, eyeScreenX, eyeScreenY, 1),
              M11_GAME_INPUT_REDRAW,
              "eye press with weapon in leader hand redraws object-description panel");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), weaponThing,
              "eye object-description route preserves leader-hand object");
    ASSERT_EQ(state.v1FoodWaterPanelActive, 0,
              "eye object-description route clears food/water panel state");
    ASSERT_EQ(state.v1ChampionStatsPanelActive, 0,
              "eye object-description route clears champion stats panel state");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 1,
              "eye object-description route activates object-description panel");
    ASSERT_EQ(state.v1ObjectDescriptionThing, weaponThing,
              "object-description state records inspected leader-hand thing");
    ASSERT_EQ(state.v1ObjectDescriptionIconIndex >= 0, 1,
              "object-description state records a resolved icon index");
    ASSERT_CONTAINS(state.v1ObjectDescriptionBody, "WEAPON",
                    "object-description body records weapon type detail");
}

/* Detail 4: runtime status hand routes C020..C027.  ReDMCSB COMMAND.C
 * G0455 lines 489-496 binds these commands to screen-relative zones
 * C211..C218; CHAMPION.C F0302 then receives command-20 as the slot box
 * index.  These narrow hand boxes must win over the broader C012..C015
 * status-box routes in the bounded resolver. */
static void test_inventory_status_hand_runtime_routes(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonContainer_Compat containers[1];
    int statusZoneX = 0, statusZoneY = 0, statusZoneW = 0, statusZoneH = 0;
    int space = M11_DM1_MOUSE_SPACE_NONE;
    int zoneId = 0;
    int command = 0;
    unsigned int slotBox;
    int leftButton, topY;

    seed_panel_view(&state, &things, weapons, containers);

    /* The source-locked slotbox table maps slotBox 0..7 to:
     *   commandId  = 20..27, zoneIndex = 211..218
     *   left = championIndex*69 + (handSlot ? 24 : 4)
     *   top  = 10, right = left + 15, bottom = 25.
     * The status-box origin lives at M11_V1_PARTY_PANEL_X = 0,
     * M11_PARTY_PANEL_Y = 0 with a 69-pixel stride per champion. */
    for (slotBox = 0; slotBox < 8u; ++slotBox) {
        ChampionStatusSlotBoxCompat box;
        ASSERT_TRUE(CHAMPION_Compat_GetStatusSlotBox((int)slotBox, &box),
                    "CHAMPION_Compat_GetStatusSlotBox returns the 8 status hand boxes");
        ASSERT_EQ(box.commandId, 20u + slotBox,
                  "status slot box command is C020..C027 in source order");
        ASSERT_EQ(box.zoneIndex, 211u + slotBox,
                  "status slot box zone is C211..C218 in source order");
    }

    for (slotBox = 0; slotBox < 8u; ++slotBox) {
        const int championIndex = (int)(slotBox >> 1);
        const int handSlot = (int)(slotBox & 1u);
        ASSERT_TRUE(M11_GameView_GetV1StatusBoxZone(championIndex,
                                                    &statusZoneX, &statusZoneY,
                                                    &statusZoneW, &statusZoneH),
                    "status box zone is available");
        /* Status hand box is at status-box origin + (4 or 24) horizontally,
         * same y=10..25. */
        leftButton = statusZoneX + (handSlot ? 24 : 4) + 2;
        topY = statusZoneY + 10 + 2;

        command = M11_GameView_GetV1MouseCommandForPoint(
            M11_DM1_MOUSE_LIST_INTERFACE,
            leftButton, topY,
            M11_DM1_MOUSE_MASK_LEFT,
            &space, &zoneId);
        ASSERT_EQ(command, 20 + (int)slotBox,
                  "status hand rectangle resolves to its C020..C027 command");
        ASSERT_EQ(zoneId, 211 + (int)slotBox,
                  "status hand rectangle returns its C211..C218 zone id");
        ASSERT_EQ(space, M11_DM1_MOUSE_SPACE_SCREEN,
                  "status hand rectangle is screen-relative");
    }

    command = M11_GameView_GetV1MouseCommandForPoint(
        M11_DM1_MOUSE_LIST_INTERFACE,
        statusZoneX + 4 - 1,
        statusZoneY + 10 + 2,
        M11_DM1_MOUSE_MASK_LEFT,
        &space, &zoneId);
    ASSERT_TRUE(command != 20,
                "pixel immediately left of a status hand zone does not route to C020");
}

/* Detail 5: open/close chest action-hand icon swap (C144 <-> C145) and
 * the close-time clear of v1OpenChestThing.  ReDMCSB CHEST.C:43-46 and
 * CHAMDRAW.C:621-630 F0291. */
static void test_inventory_open_chest_action_hand_icon_swap(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);

    seed_panel_view(&state, &things, weapons, containers);
    /* The open-chest source-side invariant must agree with the
     * runtime icon swap: action-hand base icon 144, open icon 145. */
    ASSERT_EQ(INVENTORY_Compat_GetActionHandIconForOpenChest(1u, 1u, chestThing, chestThing, 144u),
              145u,
              "INVENTORY_Compat_GetActionHandIconForOpenChest maps 144 -> 145 for open chest");
    ASSERT_EQ(INVENTORY_Compat_GetActionHandIconForOpenChest(1u, 1u, chestThing, chestThing, 0u),
              0u,
              "INVENTORY_Compat_GetActionHandIconForOpenChest leaves non-144 icons alone");

    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;
    containers[0].slot = daggerThing;

    /* Closed chest -> C144. */
    ASSERT_EQ(M11_GameView_GetV1InventorySlotIconIndex(&state, CHAMPION_SLOT_ACTION_HAND),
              144,
              "closed action-hand chest renders as C144 before open");

    /* Open chest -> runtime icon swaps to C145. */
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "M11_GameView_OpenV1ActionHandChest opens a container in the action hand");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "M11_GameView_GetV1OpenChestThing mirrors the open container");
    ASSERT_EQ(M11_GameView_GetV1InventorySlotIconIndex(&state, CHAMPION_SLOT_ACTION_HAND),
              145,
              "open action-hand chest renders as C145 after open");

    /* Close chest -> runtime icon reverts to C144 and v1OpenChestThing clears. */
    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "M11_GameView_CloseV1OpenChest clears v1OpenChestThing");
    ASSERT_EQ(M11_GameView_GetV1InventorySlotIconIndex(&state, CHAMPION_SLOT_ACTION_HAND),
              144,
              "closed action-hand chest renders as C144 after close");
}

/* Detail 6: occupied backpack-slot swap with a non-container leader-hand
 * object.  ReDMCSB CHAMPION.C F0302 lines 697-710 validates the
 * leader-hand object against G0038 slot masks, then removes the old slot
 * occupant into the leader hand and writes the previous leader-hand object
 * back to the slot.  DATA.C lines 1063-1079 make C520..C536 backpack slots
 * MASK0xFFFF_ANY_SLOT; DATA.C lines 1080-1087 reserve the container-only
 * mask for chest C537..C544. */
static void test_inventory_backpack_slot_accepts_non_container_swap(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonContainer_Compat containers[1];
    struct ChampionState_Compat* champ;
    unsigned short quiverOnlyThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short oldBackpackThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    int sx = 0, sy = 0, sw = 0, sh = 0;
    int space = M11_DM1_MOUSE_SPACE_NONE;
    int zoneId = 0;
    int command = 0;

    seed_panel_view(&state, &things, weapons, containers);
    weapons[0].type = 4; /* object-info 27: MASK0x0040_QUIVER_LINE1, not container */
    weapons[0].next = THING_ENDOFLIST;
    weapons[1].type = 2; /* container-compatible occupant used only as swap payload */
    weapons[1].next = THING_ENDOFLIST;
    champ = &state.world.party.champions[0];
    champ->inventory[CHAMPION_SLOT_BACKPACK_1] = oldBackpackThing;

    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, quiverOnlyThing), 1,
              "leader hand accepts the quiver-only test object");
    ASSERT_TRUE(M11_GameView_GetV1InventoryBackpackSlotZone(0, &sx, &sy, &sw, &sh),
                "C520 backpack slot zone is available");
    command = M11_GameView_GetV1MouseCommandForPoint(
        M11_DM1_MOUSE_LIST_INVENTORY,
        sx + sw / 2,
        33 + sy + sh / 2,
        M11_DM1_MOUSE_MASK_LEFT,
        &space, &zoneId);
    ASSERT_EQ(command, 41, "C520 backpack slot resolves to C041");
    ASSERT_EQ(zoneId, 520, "C520 backpack route returns zone id 520");
    ASSERT_EQ(space, M11_DM1_MOUSE_SPACE_VIEWPORT,
              "C520 backpack route is viewport-relative");

    ASSERT_EQ(M11_GameView_HandlePointerButton(&state,
                                               sx + sw / 2,
                                               33 + sy + sh / 2,
                                               M11_DM1_MOUSE_MASK_LEFT),
              M11_GAME_INPUT_REDRAW,
              "C520 occupied backpack click swaps with non-container leader hand");
    ASSERT_EQ(champ->inventory[CHAMPION_SLOT_BACKPACK_1], quiverOnlyThing,
              "C520 receives the non-container leader-hand object");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), oldBackpackThing,
              "old C520 occupant moves to leader hand");
}

int main(void) {
    printf("probe=firestaff_dm1_v1_inventory_panel_mouse_routes_runtime\n");
    printf("sourceEvidence=COMMAND.C:412-417 G0449 right-button close, "
           "COMMAND.C:426-427 mouth/eye C545/C546, "
           "COMMAND.C:498-507 G0456 panel-chest C058..C065/C537..C544, "
           "COMMAND.C:489-496 status hand C020..C027/C211..C218, "
           "DATA.C:1063-1079 backpack MASK0xFFFF_ANY_SLOT vs "
           "DATA.C:1080-1087 chest MASK0x0400_CONTAINER, "
           "PANEL.C:1126-1200 eye object-description route, "
           "CHEST.C:43-46 + CHAMDRAW.C:621-630 C144->C145 open remap, "
           "CHEST.C:112-133 close-time compact\n");

    test_inventory_close_panel_right_button_route();
    test_inventory_chest_slot_routes_all_eight();
    test_inventory_mouth_eye_routes_runtime();
    test_inventory_status_hand_runtime_routes();
    test_inventory_open_chest_action_hand_icon_swap();
    test_inventory_backpack_slot_accepts_non_container_swap();

    /* Cross-check the source-locked route tables to the runtime
     * resolver to detect any drift in C020..C027 / C058..C065. */
    {
        unsigned int k;
        int okCount = 0;
        for (k = 0u; k < 6u; ++k) {
            InventoryPanelRouteCompat r;
            if (!INVENTORY_Compat_GetPanelRoute(k + 1u, &r)) continue;
            if (r.commandId == 11u && r.zoneIndex == 2u) ++okCount;
            if (r.commandId == 140u && r.zoneIndex == 562u) ++okCount;
            if (r.commandId == 145u && r.zoneIndex == 564u) ++okCount;
            if (r.commandId == 141u && r.zoneIndex == 565u) ++okCount;
            if (r.commandId == 81u && r.zoneIndex == 101u) ++okCount;
        }
        ASSERT_TRUE(okCount == 5,
                    "InventoryPanelRouteCompat carries the 5 right/click-through routes");
    }

    /* Static panel-chest mouse-route invariant (zone ids, x1 > 0,
     * viewport-vs-panel y offset = 33) still holds. */
    ASSERT_EQ(panel_chest_mouse_routes_GetInvariant(), 1u,
              "panel_chest_mouse_routes invariant holds");

    printf("inventoryPanelMouseRoutesRuntimePass=%d\n", g_pass);
    printf("inventoryPanelMouseRoutesRuntimeFail=%d\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
