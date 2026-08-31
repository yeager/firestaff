/* Real PC34 F0115 alcove-object presentation regression.
 *
 * F0121/F0124 call F0115 after F0107's real alcove wall material.  This
 * walks every loaded DM1 map/party pose and requires one genuine C2548
 * object blit to reach the dedicated wall-alcove receipt. */

#include "m11_game_view.h"
#include "main_loop_m11.h"
#include "menu_startup_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_inventory_slot_placement_pc34_compat.h"
#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

enum { kFramebufferWidth = 320, kFramebufferHeight = 200 };

/* The production launcher accepts the preserved PC34 ZIP directly and keeps
 * its members in memory.  This regression used to call StartDm1() only,
 * unintentionally requiring an extracted DATA directory even when the real
 * archive is available.  Exercise the same M12 -> M11 handoff for a regular
 * archive file; retain the direct helper for an explicitly supplied loose
 * corpus used by older local test setups. */
static int start_real_dm1_corpus(M11_GameViewState *state, const char *path)
{
    struct stat st;

    if (!state || !path || !path[0]) return 0;
    if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
        M12_StartupMenuState menu;
        int opened;

        M12_StartupMenu_InitWithDataDir(&menu, path, "dm1");
        if (!M11_PrepareDirectLaunchForGame(&menu, "dm1")) {
            M12_StartupMenu_Destroy(&menu);
            return 0;
        }
        opened = M11_GameView_OpenSelectedMenuEntry(state, &menu);
        M12_StartupMenu_Destroy(&menu);
        return opened;
    }
    return M11_GameView_StartDm1(state, path);
}

static int square_is_walkable(const M11_GameViewState *state,
                              int mapIndex, int x, int y)
{
    const struct DungeonMapDesc_Compat *map;
    unsigned char square;
    int index;

    if (!state || !state->world.dungeon || !state->world.dungeon->tiles ||
        mapIndex < 0 || mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return 0;
    }
    map = &state->world.dungeon->maps[mapIndex];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height ||
        !state->world.dungeon->tiles[mapIndex].squareData) {
        return 0;
    }
    index = x * (int)map->height + y;
    square = state->world.dungeon->tiles[mapIndex].squareData[index];
    return ((square & DUNGEON_SQUARE_MASK_TYPE) >> 5) ==
        DUNGEON_ELEMENT_CORRIDOR;
}

static int front_square_is_wall(const M11_GameViewState *state,
                                int mapIndex, int x, int y, int direction)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    const struct DungeonMapDesc_Compat *map;
    int frontX;
    int frontY;
    int index;

    if (!state || !state->world.dungeon || !state->world.dungeon->tiles ||
        mapIndex < 0 || mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return 0;
    }
    map = &state->world.dungeon->maps[mapIndex];
    frontX = x + dx[direction & 3];
    frontY = y + dy[direction & 3];
    if (frontX < 0 || frontY < 0 || frontX >= (int)map->width ||
        frontY >= (int)map->height || !state->world.dungeon->tiles[mapIndex].squareData) {
        return 0;
    }
    index = frontX * (int)map->height + frontY;
    return ((state->world.dungeon->tiles[mapIndex].squareData[index] &
             DUNGEON_SQUARE_MASK_TYPE) >> 5) == DUNGEON_ELEMENT_WALL;
}

/* ReDMCSB DUNGEON.C:F0161/F0163/F0164 store the authoritative floor
 * ownership in the square's Thing chain.  Do not infer success from a HUD
 * message or the inventory mirror: inspect the loaded compact table after
 * each live transaction. */
static int square_contains_thing_exactly_once(const M11_GameViewState *state,
                                              int map_index, int x, int y,
                                              unsigned short target)
{
    unsigned short current;
    unsigned short target_identity;
    int count = 0;
    int guard = 0;

    if (!state || !state->world.dungeon || !state->world.things) return 0;
    current = F0511_DUNGEON_GetSquareFirstThing_Compat(
        state->world.dungeon, state->world.things, map_index, x, y);
    target_identity = (unsigned short)(target & 0x3fffu);
    while (current != THING_NONE && current != THING_ENDOFLIST &&
           guard++ < 64) {
        if ((current & 0x3fffu) == target_identity) ++count;
        current = F0512_DUNGEON_GetThingNext_Compat(
            state->world.things, current);
    }
    return count == 1 && guard < 64;
}

int main(void)
{
    const char *dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    M11_GameViewState state;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    int mapIndex;

    if (!dataDir || !dataDir[0]) {
        puts("SKIP: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!start_real_dm1_corpus(&state, dataDir)) {
        M11_GameView_Shutdown(&state);
        fputs("configured DM1 corpus is unavailable\n", stderr);
        return 1;
    }
    /* OBJECT.C F0031 resolves display names from M564 by icon index.  A
     * verified PC34 launch must therefore own the decoded original table,
     * rather than falling back to the legacy subtype-name bridge. */
    if (!state.dm1ObjectNameTableValid ||
        state.dm1ObjectNames[0][0] == '\0') {
        fprintf(stderr, "DM1 M564 object-name table was not loaded\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;

    for (mapIndex = 0;
         mapIndex < (int)state.world.dungeon->header.mapCount;
         ++mapIndex) {
        const struct DungeonMapDesc_Compat *map = &state.world.dungeon->maps[mapIndex];
        int y;
        for (y = 0; y < (int)map->height; ++y) {
            int x;
            for (x = 0; x < (int)map->width; ++x) {
                int direction;
                if (!square_is_walkable(&state, mapIndex, x, y)) continue;
                for (direction = 0; direction < 4; ++direction) {
                    M11_Dm1FloorItemHostPresentationReceipt receipt;
                    state.world.party.mapIndex = mapIndex;
                    state.world.party.mapX = x;
                    state.world.party.mapY = y;
                    state.world.party.direction = direction;
                    ++state.world.gameTick;
                    memset(framebuffer, 0, sizeof(framebuffer));
                    M11_GameView_Draw(&state, framebuffer,
                                      kFramebufferWidth, kFramebufferHeight);
                    memset(&receipt, 0, sizeof(receipt));
                    M11_GameView_GetDm1AlcoveItemHostPresentationReceipt(&receipt);
                    if (receipt.valid && !receipt.floorItemLane &&
                        receipt.usesF0791Blit && receipt.transparentColor == 10 &&
                        receipt.sourceZone >= 2548 && receipt.destinationW > 0 &&
                        receipt.destinationH > 0 && receipt.graphicsId > 0 &&
                        front_square_is_wall(&state, mapIndex, x, y, direction)) {
                        int clickX = receipt.destinationX + receipt.destinationW / 2;
                        int clickY = receipt.destinationY + receipt.destinationH / 2;
                        M11_GameInputResult clickResult =
                            M11_GameView_HandlePointer(&state, clickX, clickY, 1);
                        if (clickResult != M11_GAME_INPUT_REDRAW ||
                            DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) ==
                                THING_NONE) {
                            fprintf(stderr,
                                    "real alcove item was not pickable at (%d,%d) size=%dx%d result=%d hand=%u inventory=%d\\n",
                                    clickX, clickY, receipt.destinationW, receipt.destinationH,
                                    clickResult,
                                    (unsigned int)DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state),
                                    state.inventoryPanelActive);
                            M11_GameView_Shutdown(&state);
                            return 1;
                        }
                        {
                            unsigned short picked =
                                DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state);
                            int pickedIconIndex;
                            DM1_V1_InventorySlotBoxZonePc34 targetZone;
                            int targetSlot = -1;
                            int targetBox = 0;
                            unsigned int allowedSlots =
                                dm1_v1_dungeon_get_object_allowed_slots_pc34(
                                    state.world.things, picked);
                            if (allowedSlots & 0x0200u) {
                                targetSlot = CHAMPION_SLOT_HAND_LEFT;
                            } else if (allowedSlots & 0x0040u) {
                                targetSlot = CHAMPION_SLOT_QUIVER_1;
                            } else if (allowedSlots & 0x0080u) {
                                targetSlot = CHAMPION_SLOT_QUIVER_2;
                            } else if (allowedSlots & 0x0100u) {
                                targetSlot = CHAMPION_SLOT_POUCH_1;
                            } else if (allowedSlots & 0xFFFFu) {
                                targetSlot = CHAMPION_SLOT_BACKPACK_1;
                            }
                            targetBox = dm1_v1_inventory_source_slot_box_for_champion_slot_pc34(
                                targetSlot);
                            if (targetSlot < 0 || targetBox <= 0 ||
                                !dm1_v1_inventory_source_slot_box_zone_pc34(
                                    targetBox, &targetZone) ||
                                !M11_GameView_ToggleInventoryPanel(&state)) {
                                fprintf(stderr, "real alcove item inventory panel did not open\n");
                                M11_GameView_Shutdown(&state);
                                return 1;
                            }
                            clickResult = M11_GameView_HandlePointer(
                                &state,
                                DM1_VIEWPORT_X + targetZone.x + targetZone.w / 2,
                                DM1_VIEWPORT_Y + targetZone.y + targetZone.h / 2,
                                1);
                            if (clickResult != M11_GAME_INPUT_REDRAW ||
                                DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != THING_NONE ||
                                state.world.party.champions[0].inventory[targetSlot] != picked) {
                                fprintf(stderr, "real item did not place into legal source slot result=%d held=%u target=%u allowed=0x%x icon=%d panel=%d point=(%d,%d)\n",
                                        clickResult,
                                        (unsigned int)DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state),
                                        (unsigned int)state.world.party.champions[0].inventory[targetSlot],
                                        allowedSlots,
                                        dm1_v1_dungeon_get_object_icon_index_pc34(state.world.things, picked, 0),
                                        state.inventoryPanelActive,
                                        DM1_VIEWPORT_X + targetZone.x + targetZone.w / 2,
                                        DM1_VIEWPORT_Y + targetZone.y + targetZone.h / 2);
                                M11_GameView_Shutdown(&state);
                                return 1;
                            }
                            pickedIconIndex = dm1_v1_dungeon_get_object_icon_index_pc34(
                                state.world.things, picked, 0);
                            if (!state.dm1ObjectNameTableValid ||
                                pickedIconIndex < 0 ||
                                state.dm1ObjectNames[pickedIconIndex][0] == '\0') {
                                fprintf(stderr, "real placed item lost its M564 name\n");
                                M11_GameView_Shutdown(&state);
                                return 1;
                            }
                            /* ReDMCSB PANEL.C:F0302 first moves a selected
                             * inventory object into G4055 (the mouse hand).
                             * The keyboard drop command is not allowed to
                             * invent a direct quiver-to-floor shortcut, so
                             * execute that real UI leg before F0374/F0373.
                             * This is the user-visible failure mode behind
                             * disappearing/duplicated objects: after the
                             * legal transaction, dropping must create
                             * exactly one floor owner.  The generic keyboard
                             * pickup command intentionally selects the
                             * source pile-top object, which can be a
                             * different real Thing on an occupied square;
                             * its targeted pointer route is covered by the
                             * original alcove pickup above. */
                            clickResult = M11_GameView_HandlePointer(
                                &state,
                                DM1_VIEWPORT_X + targetZone.x + targetZone.w / 2,
                                DM1_VIEWPORT_Y + targetZone.y + targetZone.h / 2,
                                1);
                            if (clickResult != M11_GAME_INPUT_REDRAW ||
                                state.world.party.champions[0].inventory[targetSlot] !=
                                    THING_NONE ||
                                DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) !=
                                    picked) {
                                fprintf(stderr,
                                        "real item did not return from legal slot to mouse hand\n");
                                M11_GameView_Shutdown(&state);
                                return 1;
                            }
                            if (!M11_GameView_DropItem(&state) ||
                                DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) !=
                                    THING_NONE ||
                                !square_contains_thing_exactly_once(
                                    &state, state.world.party.mapIndex,
                                    state.world.party.mapX,
                                    state.world.party.mapY, picked)) {
                                fprintf(stderr,
                                        "real item floor drop did not retain one source owner\n");
                                M11_GameView_Shutdown(&state);
                                return 1;
                            }
                            printf("ok: real PC34 alcove pickup/place thing=%u target=C%d allowed=0x%x\n",
                                   (unsigned int)picked,
                                   targetZone.zoneId,
                                   allowedSlots);
                        }
                        printf("ok: real PC34 alcove item map=%d party=(%d,%d,%d) graphic=%d zone=%d\n",
                               mapIndex, x, y, direction, receipt.graphicsId,
                               receipt.sourceZone);
                        M11_GameView_Shutdown(&state);
                        return 0;
                    }
                }
            }
        }
    }

    fprintf(stderr, "no real PC34 F0115 alcove object was presented\n");
    M11_GameView_Shutdown(&state);
    return 1;
}
