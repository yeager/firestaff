/* Real PC34 HoC wall material receipt regression.
 * DUNVIEW.C F0096 materializes the map wall set before F0115/F0128 consumes
 * wall panels; F0172/F0107 route C127 through C346/C026, not host art. */

#include "dm1_v1_champion_mirror_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
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
    DM1_FrontMirrorRenderPlanPc34 plan;
    M11_Dm1HoCMirrorHostPresentationReceipt presented;
    const M11_AssetSlot *backing;

    if (!state || !framebuffer ||
        !dm1_v1_front_mirror_render_plan_pc34(renderIndex, &plan)) {
        return 0;
    }
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
                return framebuffer[(33 + dstY) * kFramebufferWidth + dstX] ==
                    expected;
            }
        }
    }
    return 1;
}

static int find_ordinary_hoc_wall_ornament(
    M11_GameViewState *state,
    unsigned char *framebuffer)
{
    const struct DungeonMapDesc_Compat *map;
    int partyY;

    if (!state || !state->world.dungeon || !framebuffer) {
        return 0;
    }
    map = &state->world.dungeon->maps[0];
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
                        M11_Dm1WallOrnamentHostPresentationReceipt receipt;

                        if (!M11_GameView_ProbeViewportRenderMetadata(
                                state, forward, side, NULL, NULL, &element,
                                &ornament, &portrait, &inscription, NULL) ||
                            element != DUNGEON_ELEMENT_WALL ||
                            ornament <= 0 || portrait >= 0 || inscription >= 0) {
                            continue;
                        }
                        memset(framebuffer, 0,
                               kFramebufferWidth * kFramebufferHeight);
                        M11_GameView_Draw(state, framebuffer,
                                          kFramebufferWidth, kFramebufferHeight);
                        memset(&receipt, 0, sizeof(receipt));
                        M11_GameView_GetDm1WallOrnamentHostPresentationReceipt(
                            &receipt);
                        if (receipt.valid && receipt.globalOrnamentIndex > 0 &&
                            receipt.graphicIndex >= 0 && receipt.width > 0 &&
                            receipt.height > 0) {
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

int main(void)
{
    const char *dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char *home;
    char defaultDataDir[1024];
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
        home = getenv("HOME");
        if (!home || !home[0]) return 0;
        snprintf(defaultDataDir, sizeof(defaultDataDir), "%s/.firestaff/data/dm1", home);
        dataDir = defaultDataDir;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir)) {
        M11_GameView_Shutdown(&state);
        return getenv("FIRESTAFF_DM1_DATA_DIR") ? 1 : 0;
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
    if (!find_ordinary_hoc_wall_ornament(&state, framebuffer)) {
        fprintf(stderr, "real PC34 HoC ordinary wall ornament was not presented by M11\n");
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
