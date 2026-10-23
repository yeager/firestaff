/* Real PC34 HoC wall material receipt regression.
 * DUNVIEW.C F0096 materializes the map wall set before F0115/F0128 consumes
 * wall panels; F0172/F0107 route C127 through C346/C026, not host art. */

#include "dm1_v1_champion_mirror_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_ornament_cache_owner_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_wall_ornament_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kFramebufferWidth = 320, kFramebufferHeight = 200 };

static int dx(int direction) { return (direction & 3) == 1 ? 1 : ((direction & 3) == 3 ? -1 : 0); }
static int dy(int direction) { return (direction & 3) == 2 ? 1 : ((direction & 3) == 0 ? -1 : 0); }

static int find_front_mirror(M11_GameViewState *state,
                             unsigned char *framebuffer,
                             int *out_render_index)
{
    const struct DungeonMapDesc_Compat *map = &state->world.dungeon->maps[0];
    int y;
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            unsigned short thing;
            int safety = 0;
            thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                state->world.dungeon, state->world.things, 0, x, y);
            while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                    int sensorIndex = (int)THING_GET_INDEX(thing);
                    int direction = ((int)THING_GET_CELL(thing) + 2) & 3;
                    int partyX = x - dx(direction);
                    int partyY = y - dy(direction);
                    if (sensorIndex >= 0 && sensorIndex < state->world.things->sensorCount &&
                        state->world.things->sensors[sensorIndex].sensorType == 127 &&
                        partyX >= 0 && partyY >= 0 &&
                        partyX < (int)map->width && partyY < (int)map->height) {
                        /* DUNGEON.C F0172 publishes C127 through the Thing's
                         * packed wall cell. Whether the front square is an
                         * effective wall (including a closed fake wall) is
                         * the M11/F0172 runtime decision, not a raw-tile
                         * fixture precondition. */
                        state->world.party.mapIndex = 0;
                        state->world.party.mapX = partyX;
                        state->world.party.mapY = partyY;
                        state->world.party.direction = direction;
                        memset(framebuffer, 0, kFramebufferWidth * kFramebufferHeight);
                        M11_GameView_Draw(state, framebuffer,
                                          kFramebufferWidth, kFramebufferHeight);
                        if (M11_GameView_GetFrontMirrorOrdinal(state) !=
                            (int)state->world.things->sensors[sensorIndex].sensorData) {
                            thing = F0512_DUNGEON_GetThingNext_Compat(
                                state->world.things, thing);
                            continue;
                        }
                        if (out_render_index) {
                            *out_render_index =
                                (int)state->world.things->sensors[sensorIndex].sensorData;
                        }
                        return 1;
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
            }
        }
    }
    return 0;
}

static int verify_front_mirror_backing_pixel(
    M11_GameViewState *state,
    const unsigned char *framebuffer,
    int renderIndex)
{
    DM1_FrontMirrorRenderPlanPc34 plan = {0};
    M11_Dm1HoCMirrorHostPresentationReceipt presented;
    const M11_AssetSlot *backing;
    int opaque = 0;
    if (!state || !framebuffer || renderIndex < 0 || renderIndex >= 24)
        return 0;
    /* Authentic PC C696 1093={0,4,112,51}, C346=48x43;
     * F0107 D1C keeps native indices. C737={15,4,0,35} selects C026.
     * Do not use the legacy G0205/D2 host plan as the pixel oracle. */
    plan.ornament.graphicIndex = 346;
    plan.ornament.transparentColor = 10;
    plan.backingSourceWidth = plan.backingWidth = 48;
    plan.backingSourceHeight = plan.backingHeight = 43;
    plan.backingDstX = 88; plan.backingDstY = 29;
    plan.portraitGraphicIndex = 26;
    plan.portraitSrcX = (renderIndex % 8)*32;
    plan.portraitSrcY = (renderIndex / 8)*29;
    plan.portraitDstX = 96; plan.portraitDstY = 35;
    plan.portraitWidth = 32; plan.portraitHeight = 29;
    plan.portraitTransparentColor = 1;
    backing = M11_AssetLoader_Load(&state->assetLoader,
                                   (unsigned int)plan.ornament.graphicIndex);
    if (!backing || !backing->loaded || !backing->pixels ||
        plan.ornament.srcX < 0 || plan.ornament.srcY < 0 ||
        plan.backingSourceWidth <= 0 || plan.backingSourceHeight <= 0 ||
        plan.backingWidth <= 0 || plan.backingHeight <= 0 ||
        plan.ornament.srcX + plan.backingSourceWidth > (int)backing->width ||
        plan.ornament.srcY + plan.backingSourceHeight > (int)backing->height) {
        return 0;
    }
    memset(&presented, 0, sizeof(presented));
    M11_GameView_GetDm1HoCMirrorHostPresentationReceipt(&presented);
    if (!presented.valid || presented.renderIndex != renderIndex ||
        presented.backingGraphicIndex != plan.ornament.graphicIndex ||
        presented.backingSourceWidth != plan.backingSourceWidth ||
        presented.backingSourceHeight != plan.backingSourceHeight ||
        presented.backingDestinationX != plan.backingDstX ||
        presented.backingDestinationY != 33 + plan.backingDstY ||
        presented.backingWidth != plan.backingWidth ||
        presented.backingHeight != plan.backingHeight ||
        presented.backingTransparentColor != plan.ornament.transparentColor ||
        presented.backingPaletteMapValid != plan.ornament.paletteMapValid ||
        (presented.backingPaletteMapValid &&
         memcmp(presented.backingPaletteMap, plan.ornament.paletteMap,
                sizeof(presented.backingPaletteMap)) != 0) ||
        presented.portraitGraphicIndex != plan.portraitGraphicIndex ||
        presented.portraitSourceX != plan.portraitSrcX ||
        presented.portraitSourceY != plan.portraitSrcY ||
        presented.portraitDestinationX != plan.portraitDstX ||
        presented.portraitDestinationY != 33 + plan.portraitDstY ||
        presented.portraitWidth != plan.portraitWidth ||
        presented.portraitHeight != plan.portraitHeight ||
        presented.portraitTransparentColor != plan.portraitTransparentColor) {
        return 0;
    }
    {
        int y;
        for (y = 0; y < plan.backingHeight; ++y) {
            int x;
            for (x = 0; x < plan.backingWidth; ++x) {
                const int dstX = plan.backingDstX + x;
                const int dstY = plan.backingDstY + y;
                const int sourceX = plan.ornament.srcX +
                    (x * plan.backingSourceWidth) / plan.backingWidth;
                const int sourceY = plan.ornament.srcY +
                    (y * plan.backingSourceHeight) / plan.backingHeight;
                unsigned char expected = backing->pixels[
                    sourceY * (int)backing->width + sourceX];

                if (expected == (unsigned char)plan.ornament.transparentColor ||
                    (dstX >= plan.portraitDstX &&
                     dstX < plan.portraitDstX + plan.portraitWidth &&
                     dstY >= plan.portraitDstY &&
                     dstY < plan.portraitDstY + plan.portraitHeight)) {
                    continue;
                }
                if (plan.ornament.paletteMapValid) {
                    expected = plan.ornament.paletteMap[expected & 0x0f];
                }
                if (framebuffer[(33 + dstY) * kFramebufferWidth + dstX] !=
                    (unsigned char)(16 | expected)) return 0;
                ++opaque;
            }
        }
    }
    return opaque > 0;
}

/* A receipt alone is not visual proof: F0107 may have selected the correct
 * GRAPHICS.DAT surface but still fail to put its palette-mapped pixels on
 * screen.  Find one opaque source pixel and verify the exact F0791-scaled
 * destination pixel for each real HoC ornament. */
static int verify_wall_ornament_pixel(
    M11_GameViewState *state,
    const unsigned char *framebuffer,
    const M11_Dm1WallOrnamentHostPresentationReceipt *receipt)
{
    const M11_AssetSlot *slot;
    int destinationY;
    int drawWidth;
    int drawHeight;

    if (!state || !framebuffer || !receipt || !receipt->valid ||
        receipt->graphicIndex < 0 || receipt->destinationX < 0 ||
        receipt->destinationY < 0 || receipt->width <= 0 ||
        receipt->height <= 0 ||
        receipt->destinationX + receipt->width > kFramebufferWidth ||
        receipt->destinationY + receipt->height > kFramebufferHeight) {
        return 0;
    }
    slot = M11_AssetLoader_Load(&state->assetLoader,
                                (unsigned int)receipt->graphicIndex);
    if (!slot || !slot->loaded || !slot->pixels ||
        slot->width == 0 || slot->height == 0) {
        return 0;
    }
    drawWidth = ((int)slot->width * receipt->sourceScaleX32 + 16) >> 5;
    drawHeight = ((int)slot->height * receipt->sourceScaleY32 + 16) >> 5;
    if (drawWidth <= 0 || drawHeight <= 0) return 0;
    for (destinationY = 0; destinationY < drawHeight &&
                            destinationY < receipt->height; ++destinationY) {
        int destinationX;
        for (destinationX = 0; destinationX < drawWidth &&
                                destinationX < receipt->width; ++destinationX) {
            int sourceX = receipt->flipHorizontal
                ? ((drawWidth - 1 - destinationX) * (int)slot->width) /
                      drawWidth
                : (destinationX * (int)slot->width) / drawWidth;
            int sourceY = (destinationY * (int)slot->height) / drawHeight;
            unsigned char expected = slot->pixels[
                sourceY * (int)slot->width + sourceX];
            if (expected == (unsigned char)receipt->transparentColor) {
                continue;
            }
            if (receipt->paletteMapValid) {
                expected = receipt->paletteMap[expected & 0x0f];
            }
            return framebuffer[(receipt->destinationY + destinationY) *
                                   kFramebufferWidth +
                               receipt->destinationX + destinationX] ==
                (unsigned char)(16 | expected);
        }
    }
    return 0;
}

static int verify_all_ordinary_hoc_wall_ornaments(
    M11_GameViewState *state,
    unsigned char *framebuffer)
{
    const struct DungeonMapDesc_Compat *map;
    int expectedGlobalByOrdinal[DM1_WALL_ORN_MAX + 1];
    int ordinal;
    int partyY;

    if (!state || !state->world.dungeon || !framebuffer) {
        return 0;
    }
    memset(expectedGlobalByOrdinal, -1, sizeof(expectedGlobalByOrdinal));
    map = &state->world.dungeon->maps[0];

    /* Find every real non-mirror/non-inscription F0107 selector first.  The
     * old regression returned after one arbitrary ornament, which could miss
     * a different Hall ornament such as the wall torch. */
    for (partyY = 0; partyY < (int)map->height; ++partyY) {
        int partyX;
        for (partyX = 0; partyX < (int)map->width; ++partyX) {
            int direction;
            for (direction = 0; direction < 4; ++direction) {
                int forward;
                state->world.party.mapIndex = 0;
                state->world.party.mapX = partyX;
                state->world.party.mapY = partyY;
                state->world.party.direction = direction;
                for (forward = 1; forward <= 3; ++forward) {
                    int side;
                    for (side = -1; side <= 1; ++side) {
                        int element = -1;
                        int ornament = -1;
                        int portrait = -1;
                        int inscription = -1;
                        if (!M11_GameView_ProbeViewportRenderMetadata(
                                state, forward, side, NULL, NULL, &element,
                                &ornament, &portrait, &inscription, NULL) ||
                            element != DUNGEON_ELEMENT_WALL ||
                            ornament <= 0 || portrait >= 0 || inscription >= 0) {
                            continue;
                        }
                        if (ornament > DM1_WALL_ORN_MAX ||
                            !dm1_v1_ornament_cache_global_index_pc34(
                                state->ornamentCacheLoaded[0] ? 1 : 0,
                                state->wallOrnamentIndices[0],
                                DM1_WALL_ORN_MAX, ornament,
                                &expectedGlobalByOrdinal[ornament]) ||
                            expectedGlobalByOrdinal[ornament] < 0) {
                            fprintf(stderr,
                                    "HoC ornament discovery failed: ordinal=%d cache=%d map=%d square=%d,%d dir=%d view=%d,%d\n",
                                    ornament, state->ornamentCacheLoaded[0],
                                    state->world.party.mapIndex, partyX, partyY,
                                    direction, forward, side);
                            return 0;
                        }
                    }
                }
            }
        }
    }

    for (ordinal = 1; ordinal <= DM1_WALL_ORN_MAX; ++ordinal) {
        int presented = 0;
        if (expectedGlobalByOrdinal[ordinal] < 0) {
            continue;
        }
        for (partyY = 0; partyY < (int)map->height && !presented; ++partyY) {
            int partyX;
            for (partyX = 0; partyX < (int)map->width && !presented; ++partyX) {
                int direction;
                for (direction = 0; direction < 4 && !presented; ++direction) {
                    int forward;
                    state->world.party.mapIndex = 0;
                    state->world.party.mapX = partyX;
                    state->world.party.mapY = partyY;
                    state->world.party.direction = direction;
                    for (forward = 1; forward <= 3 && !presented; ++forward) {
                        int side;
                        for (side = -1; side <= 1; ++side) {
                            int element = -1;
                            int ornament = -1;
                            int portrait = -1;
                            int inscription = -1;
                            M11_Dm1WallOrnamentHostPresentationReceipt receipt;

                            if (!M11_GameView_ProbeViewportRenderMetadata(
                                    state, forward, side, NULL, NULL, &element,
                                    &ornament, &portrait, &inscription, NULL) ||
                                element != DUNGEON_ELEMENT_WALL ||
                                ornament != ordinal || portrait >= 0 ||
                                inscription >= 0) {
                                continue;
                            }
                            memset(framebuffer, 0,
                                   kFramebufferWidth * kFramebufferHeight);
                            M11_GameView_Draw(state, framebuffer,
                                              kFramebufferWidth, kFramebufferHeight);
                            memset(&receipt, 0, sizeof(receipt));
                            M11_GameView_GetDm1WallOrnamentHostPresentationReceipt(
                                &receipt);
                            if (receipt.valid &&
                                receipt.globalOrnamentIndex ==
                                    expectedGlobalByOrdinal[ordinal] &&
                                receipt.graphicIndex >= 0 && receipt.width > 0 &&
                                receipt.height > 0 &&
                                verify_wall_ornament_pixel(state, framebuffer,
                                                            &receipt)) {
                                presented = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (!presented) {
            fprintf(stderr,
                    "real PC34 HoC wall ornament ordinal %d (global %d) was not presented by M11\n",
                    ordinal, expectedGlobalByOrdinal[ordinal]);
            return 0;
        }
    }

    /* A reference HoC normally has at least one ordinary ornament. Keep the
     * previous regression's positive-data invariant as well. */
    for (ordinal = 1; ordinal <= DM1_WALL_ORN_MAX; ++ordinal) {
        if (expectedGlobalByOrdinal[ordinal] >= 0) {
            return 1;
        }
    }
    return 0;
}

/* The torch beside the first HoC stairs is a map-owned wall ornament with
 * an associated source Thing and sensor chain. Verify its exact GRAPHICS.DAT
 * material at the actual wall cell; a generic ornament sample could miss it. */
static int verify_hoc_stair_torch_wall_asset(
    M11_GameViewState *state,
    unsigned char *framebuffer)
{
    const struct DungeonMapDesc_Compat *map;
    unsigned short thing;
    int torchFound = 0;
    int y;

    if (!state || !state->world.dungeon || !state->world.things ||
        state->world.dungeon->header.mapCount <= 0) return 0;
    map = &state->world.dungeon->maps[0];
    if (map->width <= 3 || map->height <= 15 ||
        ((state->world.dungeon->tiles[0].squareData[
             3 * (int)map->height + 14] & DUNGEON_SQUARE_MASK_TYPE) >> 5) !=
             DUNGEON_ELEMENT_WALL ||
        ((state->world.dungeon->tiles[0].squareData[
             3 * (int)map->height + 15] & DUNGEON_SQUARE_MASK_TYPE) >> 5) !=
             DUNGEON_ELEMENT_STAIRS) {
        fputs("PC34 HoC stair/torch fixture does not match the original map\n", stderr);
        return 0;
    }

    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        state->world.dungeon, state->world.things, 0, 3, 14);
    for (int guard = 0; thing != THING_NONE && thing != THING_ENDOFLIST &&
         guard < 64; ++guard) {
        if (THING_GET_TYPE(thing) == THING_TYPE_WEAPON &&
            dm1_v1_dungeon_get_object_icon_index_pc34(
                state->world.things, thing, 0) == 4) {
            torchFound = 1;
            break;
        }
        thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
    }
    if (!torchFound) {
        fputs("PC34 HoC stair wall has no original TORCH Thing\n", stderr);
        return 0;
    }

    /* From the adjacent original corridor pose, this wall is in the D1R
     * sample. Its C013 storage sensor must not hide the real F0115 object. */
    {
        int itemX = -1, itemY = -1, element = -1;
        int itemCount = 0, summaryCount = 0;
        state->world.party.mapIndex = 0;
        state->world.party.mapX = 4;
        state->world.party.mapY = 13;
        state->world.party.direction = 2;
        if (!M11_GameView_ProbeViewportFloorItemCounts(
                state, 1, 1, &itemX, &itemY, &element,
                &itemCount, &summaryCount) ||
            itemX != 3 || itemY != 14 || element != DUNGEON_ELEMENT_WALL ||
            itemCount != 1 || summaryCount != 1) {
            fprintf(stderr,
                    "HoC stair TORCH was filtered from F0115 candidates: cell=(%d,%d) element=%d items=%d summary=%d\n",
                    itemX, itemY, element, itemCount, summaryCount);
            return 0;
        }
    }

    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int direction;
            const unsigned char partySquare =
                state->world.dungeon->tiles[0].squareData[
                    x * (int)map->height + y];
            const int partyElement =
                (partySquare & DUNGEON_SQUARE_MASK_TYPE) >> 5;
            if (partyElement != DUNGEON_ELEMENT_CORRIDOR &&
                partyElement != DUNGEON_ELEMENT_STAIRS) continue;
            for (direction = 0; direction < 4; ++direction) {
                int forward;
                state->world.party.mapIndex = 0;
                state->world.party.mapX = x;
                state->world.party.mapY = y;
                state->world.party.direction = direction;
                for (forward = 1; forward <= 3; ++forward) {
                    int side;
                    for (side = -1; side <= 1; ++side) {
                        int mapX = -1, mapY = -1, element = -1;
                        int ordinal = -1, portrait = -1, inscription = -1;
                        M11_Dm1WallOrnamentHostPresentationReceipt receipt;
                        if (!M11_GameView_ProbeViewportRenderMetadata(
                                state, forward, side, &mapX, &mapY, &element,
                                &ordinal, &portrait, &inscription, NULL) ||
                            mapX != 3 || mapY != 14 ||
                            element != DUNGEON_ELEMENT_WALL || ordinal <= 0 ||
                            portrait >= 0 || inscription >= 0) continue;

                        memset(framebuffer, 0,
                               kFramebufferWidth * kFramebufferHeight);
                        M11_GameView_Draw(state, framebuffer,
                                          kFramebufferWidth, kFramebufferHeight);
                        memset(&receipt, 0, sizeof(receipt));
                        M11_GameView_GetDm1WallOrnamentHostPresentationReceipt(
                            &receipt);
                        if (receipt.valid && receipt.globalOrnamentIndex == 32 &&
                            verify_wall_ornament_pixel(state, framebuffer,
                                                      &receipt)) {
                            printf("ok: critical PC34 HoC stair TORCH wall ornament rendered from original GRAPHICS.DAT at party=(%d,%d,d%d)\n",
                                   x, y, direction);
                            return 1;
                        }
                    }
                }
            }
        }
    }
    fputs("critical PC34 HoC stair TORCH wall ornament was not rendered from original media\n",
          stderr);
    return 0;
}

/* The original torch only helps on the dark level if the player can actually
 * take it from the wall. Exercise the real PC34 C080 pointer path at the
 * wall's C05 target and require the original Thing to reach the leader hand. */
static int verify_hoc_stair_torch_pickup(
    M11_GameViewState *state,
    unsigned char *framebuffer)
{
    unsigned short torch = THING_NONE;
    unsigned short thing;
    int mirrorOrdinal = -1;
    int torchStillStored = 0;

    if (!state || !state->world.dungeon || !state->world.things) return 0;
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        state->world.dungeon, state->world.things, 0, 3, 14);
    for (int guard = 0; thing != THING_NONE && thing != THING_ENDOFLIST &&
         guard < 64; ++guard) {
        if (THING_GET_TYPE(thing) == THING_TYPE_WEAPON &&
            dm1_v1_dungeon_get_object_icon_index_pc34(
                state->world.things, thing, 0) == 4) {
            torch = thing;
            break;
        }
        thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
    }
    if (torch == THING_NONE) return 0;

    /* A source C013 storage sensor ignores wall clicks until a champion is
     * leading. Admit one through the original HoC C127 mirror path so the
     * test exercises playable runtime state, not a fabricated roster. */
    if (!find_front_mirror(state, framebuffer, &mirrorOrdinal) ||
        !M11_GameView_SelectFrontMirrorCandidate(state) ||
        !M11_GameView_ConfirmMirrorCandidate(state, 0) ||
        state->world.party.activeChampionIndex < 0 ||
        state->world.party.championCount < 1) {
        fputs("PC34 HoC TORCH test could not admit an original C127 champion\n",
              stderr);
        return 0;
    }

    /* ReDMCSB C080's C05 wall zone touches sensors on the front wall cell. */
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 4;
    state->world.party.mapY = 14;
    state->world.party.direction = 3;
    memset(framebuffer, 0, kFramebufferWidth * kFramebufferHeight);
    M11_GameView_Draw(state, framebuffer,
                      kFramebufferWidth, kFramebufferHeight);
    if (M11_GameView_HandlePointerButton(
            state, 112, 82, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(state) != torch ||
        M11_GameView_GetObjectIconIndexForThing(state, torch) != 4) {
        fprintf(stderr,
                "PC34 HoC stair TORCH click did not put original Thing %u in leader hand\n",
                (unsigned int)torch);
        return 0;
    }
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        state->world.dungeon, state->world.things, 0, 3, 14);
    for (int guard = 0; thing != THING_NONE && thing != THING_ENDOFLIST &&
         guard < 64; ++guard) {
        if (thing == torch) {
            torchStillStored = 1;
            break;
        }
        thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
    }
    if (torchStillStored) {
        fputs("PC34 HoC stair TORCH remained in wall after C080 pickup\n", stderr);
        return 0;
    }
    /* C080 picks the wall Thing up into the mouse hand. The actual F0337
     * light calculation reads champion hand slots, so equip the torch in
     * the source C508 action-hand slot before descending. */
    {
        int slotX = 0, slotY = 0, slotW = 0, slotH = 0;
        int zoneResult;
        int clickResult;
        /* This fixture opens the source panel directly so it can focus on
         * the genuine C508 pointer transaction after C127 admission. */
        state->inventoryPanelActive = 1;
        state->dm1InventoryChampionOrdinal =
            state->world.party.activeChampionIndex + 1;
        zoneResult = M11_GameView_GetV1InventorySourceSlotBoxZone(
            9, &slotX, &slotY, &slotW, &slotH);
        clickResult = zoneResult ? M11_GameView_HandlePointerButton(
            state, slotX + slotW / 2, 33 + slotY + slotH / 2,
            DM1_V1_MOUSE_MASK_LEFT_PC34) : M11_GAME_INPUT_IGNORED;
        if (!zoneResult || clickResult != M11_GAME_INPUT_REDRAW ||
            state->leaderHandObjectPresent ||
            state->world.party.champions[
                state->world.party.activeChampionIndex]
                .inventory[CHAMPION_SLOT_ACTION_HAND] != torch) {
            fputs("PC34 HoC stair TORCH could not be equipped in C508\n", stderr);
            return 0;
        }
    }
    /* The torch matters because the first descent is dark. Use the original
     * PC34 stairs transition rather than assigning the destination level in
     * the fixture, then exercise F0337 with the same live party inventory. */
    if (state->world.dungeon->header.mapCount < 2 ||
        state->world.dungeon->maps[1].width == 0 ||
        state->world.dungeon->maps[1].height == 0) {
        fputs("PC34 HoC first dark floor is unavailable for torch-light check\n",
              stderr);
        return 0;
    }
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 3;
    state->world.party.mapY = 15;
    state->inventoryPanelActive = 0;
    state->dm1InventoryChampionOrdinal = 0;
    if (M11_GameView_HandleInput(state, M12_MENU_INPUT_USE_STAIRS) !=
            M11_GAME_INPUT_REDRAW ||
        state->world.party.mapIndex != 1 ||
        M11_GameView_GetDungeonPaletteIndex(state) >= 5 ||
        M11_GameView_GetLightLevel(state) <= 0) {
        fprintf(stderr,
                "PC34 HoC stair transition/TORCH did not provide light on map 1: map=%d level=%d palette=%d\n",
                state->world.party.mapIndex,
                M11_GameView_GetLightLevel(state),
                M11_GameView_GetDungeonPaletteIndex(state));
        return 0;
    }
    printf("ok: original PC34 HoC stair transition, TORCH pickup, and C508 equip carry light to map 1: level=%d palette=%d\n",
           M11_GameView_GetLightLevel(state), M11_GameView_GetDungeonPaletteIndex(state));
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 4;
    state->world.party.mapY = 13;
    state->world.party.direction = 2;
    state->inventoryPanelActive = 0;
    state->dm1InventoryChampionOrdinal = 0;
    return 1;
}

int main(void)
{
    const char *dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    M11_GameViewState state;
    DM1_ViewportWallHostMaterialReceiptPc34 receipt;
    DM1_ViewportSideWallHostReceiptPc34 sideReceipt;
    DM1_ViewportD3SideWallHostHandoffPc34 d3Handoff;
    DM1_ViewportLaneVisibilityReceiptPc34 visibility;
    const M11_AssetSlot *slot;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    int mirrorRenderIndex = -1;
    int mapWallSet;
    const int visible[3] = {1, 1, 1};
    static const struct { int graphic, width, height; } kPanels[] = {
        {97, 160, 111}, {102, 106, 74}, {107, 70, 49}
    };
    size_t i;

    if (!dataDir || !dataDir[0]) {
        puts("SKIP: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 77;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir)) {
        M11_GameView_Shutdown(&state);
        fputs("configured PC34 corpus could not start\n", stderr);
        return 1;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    mapWallSet = (int)state.world.dungeon->maps[0].wallSet;
    visibility = dm1_viewport_3d_lane_visibility_from_cells_pc34(
        visible, visible, visible, visible, visible);
    for (i = 0; i < sizeof(kPanels) / sizeof(kPanels[0]); ++i) {
        if (!dm1_viewport_3d_wall_host_material_receipt_pc34(
                mapWallSet, kPanels[i].graphic, -1, false,
                kPanels[i].width, kPanels[i].height, &receipt) || !receipt.valid ||
            receipt.graphic_index < 0 || receipt.transparent_color != -1 ||
            receipt.expected_width != kPanels[i].width ||
            receipt.expected_height != kPanels[i].height) goto fail;
        slot = M11_AssetLoader_Load(&state.assetLoader,
                                    (unsigned int)receipt.graphic_index);
        if (!slot || !slot->loaded || !slot->pixels ||
            slot->width != (unsigned int)receipt.expected_width ||
            slot->height != (unsigned int)receipt.expected_height) goto fail;
    }
    /* ReDMCSB DUNVIEW.C F0128: D3L/D3R select their C705/C706 material
     * before the center square. The receipt owns both the lane decision and
     * F0096 wall-set material; this test uses the installed PC34 asset, not
     * a fabricated bitmap. D4 invokes F0115 only, then exits on depth > 3. */
    memset(&d3Handoff, 0, sizeof(d3Handoff));
    if (!dm1_viewport_3d_build_d3_side_wall_host_handoff_pc34(
            DM1_VIEW_SQUARE_D3L, false, true, false, &d3Handoff) ||
        !d3Handoff.handled || !d3Handoff.draw_wall ||
        d3Handoff.falls_through_to_f0115 ||
        d3Handoff.pc34_zone != DM1_PC34_ZONE_WALL_D3L ||
        d3Handoff.transparent_color != 10 ||
        d3Handoff.selected_wall >= DM1_WALL_SET_COUNT) goto fail;
    if (!dm1_viewport_3d_build_d3_side_wall_host_handoff_pc34(
            DM1_VIEW_SQUARE_D3R, true, true, false, &d3Handoff) ||
        !d3Handoff.draw_wall || !d3Handoff.flip_horizontally ||
        d3Handoff.pc34_zone != DM1_PC34_ZONE_WALL_D3R) goto fail;
    if (!dm1_viewport_3d_build_d3_side_wall_host_handoff_pc34(
            DM1_VIEW_SQUARE_D3L, false, false, false, &d3Handoff) ||
        d3Handoff.draw_wall || !d3Handoff.falls_through_to_f0115) goto fail;
    if (!dm1_viewport_3d_build_side_wall_host_receipt_pc34(
            DM1_VIEW_SQUARE_D3L, mapWallSet, false, true, false, 3,
            &visibility, &sideReceipt) || !sideReceipt.handled ||
        !sideReceipt.draw_wall || !sideReceipt.material.valid ||
        sideReceipt.material.transparent_color != 10) goto fail;
    slot = M11_AssetLoader_Load(&state.assetLoader,
                                (unsigned int)sideReceipt.material.graphic_index);
    if (!slot || !slot->loaded || !slot->pixels ||
        slot->width != (unsigned int)sideReceipt.material.expected_width ||
        slot->height != (unsigned int)sideReceipt.material.expected_height) goto fail;
    if (dm1_viewport_3d_build_side_wall_host_receipt_pc34(
            DM1_VIEW_SQUARE_D4L, mapWallSet, false, true, false, 3,
            &visibility, &sideReceipt)) goto fail;
    if (!dm1_viewport_3d_build_side_wall_host_receipt_pc34(
            DM1_VIEW_SQUARE_D3R, mapWallSet, false, true, false, 2,
            &visibility, &sideReceipt) || sideReceipt.draw_wall ||
        !sideReceipt.handled) goto fail;
    slot = M11_AssetLoader_Load(&state.assetLoader,
                                (unsigned int)dm1_v1_graphic_champion_portraits_pc34());
    if (!slot || !slot->loaded || !slot->pixels ||
        !dm1_v1_graphic_validate_champion_portrait_atlas_pc34(
            (int)slot->width, (int)slot->height)) goto fail;
    if (!find_front_mirror(&state, framebuffer, &mirrorRenderIndex)) {
        fprintf(stderr, "real PC34 C127 sensor was not presented by M11\n");
        goto fail;
    }
    if (!verify_front_mirror_backing_pixel(&state, framebuffer, mirrorRenderIndex)) {
        fprintf(stderr, "real PC34 C127 presented without exact C346/C026 pixels\n");
        goto fail;
    }
    if (!verify_hoc_stair_torch_wall_asset(&state, framebuffer)) {
        fprintf(stderr, "real PC34 HoC stair torch wall material was not presented by M11\n");
        goto fail;
    }
    if (!verify_hoc_stair_torch_pickup(&state, framebuffer)) {
        fprintf(stderr, "real PC34 HoC stair torch could not be picked up through M11\n");
        goto fail;
    }
    if (!verify_all_ordinary_hoc_wall_ornaments(&state, framebuffer)) {
        fprintf(stderr, "real PC34 HoC wall ornaments were not all presented by M11\n");
        goto fail;
    }
    printf("ok: real PC34 HoC F0096 wall receipts and exact C346/C026 host route\n");
    M11_GameView_Shutdown(&state);
    return 0;
fail:
    fprintf(stderr, "real PC34 HoC wall material receipt or C127 route failed\n");
    M11_GameView_Shutdown(&state);
    return 1;
}
