#include "dm1_v1_champion_mirror_pc34_compat.h"

/*
 * pass796 - DM1 V1 champion mirror contract test
 * (COMMAND.C:484-488 PC-98/PC C159..C162 champion-name rows;
 * COMMAND.C:1437-1449 F0358 inclusive match; F0380:2158-2162
 * status-box click dispatch). Source-locked against COMMAND.C
 * F0358:1437-1449 + F0380:2158-2162 + DEFS.H C159..C162.
 */

#include <stdio.h>

static int gTests;
static int gPasses;

#define CHECK_ANCHOR(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static DM1_V1_ChampionMirrorClickStatePc34 base_state(void)
{
    DM1_V1_ChampionMirrorClickStatePc34 state;
    DM1_V1_ChampionMirror_InitClickStatePc34(&state);
    state.partyChampionCount = 4;
    state.inventoryChampionOrdinal = 1u;
    return state;
}

static void test_c159_name_route_without_candidate_panel(void)
{
    DM1_V1_ChampionMirrorClickStatePc34 state = base_state();
    DM1_V1_ChampionMirrorClickResultPc34 result;
    int changed;

    state.partyChampionCount = 1;
    state.inventoryChampionOrdinal = 1u;
    state.leaderIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;

    changed = DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34(
        &state,
        DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT,
        1,
        5,
        DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_ANCHOR(result.dispatchesStatusBoxClick == 1,
                 "C012 status-box click dispatches when G0299 is clear",
                 "COMMAND.C:2158-2162");
    CHECK_ANCHOR(result.attemptedSetLeader == 1,
                 "inventory champion C159 path reaches F0368",
                 "CLIKCHAM.C:24-25");
    CHECK_ANCHOR(changed == 1 && result.leaderChanged == 1,
                 "F0368 changes G0411 for a live non-leader champion",
                 "CLIKCHAM.C:51-72");
    CHECK_ANCHOR(state.leaderIndex == 0 && result.newLeaderIndex == 0,
                 "champion 0 becomes leader after C159 click",
                 "CLIKCHAM.C:66-72");
    CHECK_ANCHOR(result.nestedCommand == DM1_V1_COMMAND_NONE_PC34_COMPAT,
                 "inventory champion branch does not need nested G0455 scan",
                 "CLIKCHAM.C:24-27");
}

static void test_c159_name_route_blocked_while_c040_live(void)
{
    DM1_V1_ChampionMirrorClickStatePc34 state = base_state();
    DM1_V1_ChampionMirrorClickResultPc34 result;
    int changed;

    state.partyChampionCount = 1;
    state.inventoryChampionOrdinal = 1u;
    state.candidateChampionOrdinal = 1u;
    state.leaderIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;

    changed = DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34(
        &state,
        DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT,
        1,
        5,
        DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_ANCHOR(changed == 0,
                 "C159 status-box click does not rotate leader while G0299 owns C040",
                 "COMMAND.C:2158-2162; PANEL.C:1654-1656");
    CHECK_ANCHOR(result.ignoredByCandidatePanel == 1,
                 "nonzero G0299 blocks the C012 status-box dispatch",
                 "COMMAND.C:2158-2162");
    CHECK_ANCHOR(result.dispatchesStatusBoxClick == 0,
                 "F0367 is not called while the candidate panel is live",
                 "COMMAND.C:2158-2162; CLIKCHAM.C:24-35");
    CHECK_ANCHOR(result.attemptedSetLeader == 0,
                 "blocked C159 click never reaches F0368",
                 "CLIKCHAM.C:24-35; CLIKCHAM.C:51-72");
    CHECK_ANCHOR(result.nestedCommand == DM1_V1_COMMAND_NONE_PC34_COMPAT,
                 "blocked C159 click does not scan G0455 name rows",
                 "COMMAND.C:484-488; COMMAND.C:1379-1449");
    CHECK_ANCHOR(state.leaderIndex == DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT,
                 "G0411 remains unchanged under C040 candidate-panel guard",
                 "CLIKCHAM.C:51-72");
    CHECK_ANCHOR(state.candidateChampionOrdinal == 1u,
                 "G0299 candidate ordinal is preserved by ignored C159 click",
                 "PANEL.C:1654-1656");
}

static void test_nested_c019_name_route_without_candidate_panel(void)
{
    DM1_V1_ChampionMirrorClickStatePc34 state = base_state();
    DM1_V1_ChampionMirrorClickResultPc34 result;
    int changed;

    state.inventoryChampionOrdinal = 2u;
    state.leaderIndex = 0;

    changed = DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34(
        &state,
        DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT,
        208,
        5,
        DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_ANCHOR(result.dispatchesStatusBoxClick == 1,
                 "outer status-box dispatch reaches F0367 when G0299 is clear",
                 "COMMAND.C:2158-2162");
    CHECK_ANCHOR(result.nestedCommand == DM1_V1_COMMAND_SET_LEADER_3_PC34_COMPAT,
                 "x=208,y=5 maps to C019 champion-3 name row",
                 "COMMAND.C:484-488; COMMAND.C:1437-1449");
    CHECK_ANCHOR(result.targetLeaderIndex == 3,
                 "C019 maps to leader target index 3",
                 "CLIKCHAM.C:27-30");
    CHECK_ANCHOR(changed == 1 && result.leaderChanged == 1,
                 "nested C019 route changes leader when target is alive",
                 "CLIKCHAM.C:51-72");
    CHECK_ANCHOR(state.leaderIndex == 3,
                 "G0411 records champion 3 after C019 route",
                 "CLIKCHAM.C:66-72");
}

static void test_nested_c019_route_blocked_while_c040_live(void)
{
    DM1_V1_ChampionMirrorClickStatePc34 state = base_state();
    DM1_V1_ChampionMirrorClickResultPc34 result;
    int changed;

    state.inventoryChampionOrdinal = 2u;
    state.candidateChampionOrdinal = 1u;
    state.leaderIndex = 0;

    changed = DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34(
        &state,
        DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT,
        208,
        5,
        DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_ANCHOR(changed == 0,
                 "C019 name-row rotation is blocked while C040 is live",
                 "COMMAND.C:2158-2162; PANEL.C:1654-1656");
    CHECK_ANCHOR(result.ignoredByCandidatePanel == 1,
                 "G0299 prevents the outer status-box owner from dispatching",
                 "COMMAND.C:2158-2162");
    CHECK_ANCHOR(result.nestedCommand == DM1_V1_COMMAND_NONE_PC34_COMPAT,
                 "blocked C019 route never enters F0358/G0455",
                 "COMMAND.C:484-488; COMMAND.C:1379-1449");
    CHECK_ANCHOR(result.targetLeaderIndex == DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT,
                 "blocked C019 route has no F0368 target",
                 "CLIKCHAM.C:27-30; CLIKCHAM.C:51-72");
    CHECK_ANCHOR(state.leaderIndex == 0,
                 "existing leader is preserved by the C040 panel guard",
                 "CLIKCHAM.C:51-72");
}

static void test_f0358_edges_and_f0368_dead_target(void)
{
    DM1_V1_ChampionMirrorClickStatePc34 state = base_state();
    DM1_V1_ChampionMirrorClickResultPc34 result;
    int changed;

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34(
            249, 6, DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT) ==
            DM1_V1_COMMAND_SET_LEADER_3_PC34_COMPAT,
        "F0358 uses inclusive right/bottom edges for C019",
        "COMMAND.C:484-488; COMMAND.C:1437-1449");
    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34(
            249, 7, DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT) ==
            DM1_V1_COMMAND_NONE_PC34_COMPAT,
        "F0358 rejects points below the C019 name row",
        "COMMAND.C:484-488; COMMAND.C:1437-1449");
    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34(
            249, 6, 0u) == DM1_V1_COMMAND_NONE_PC34_COMPAT,
        "F0358 requires the left mouse button bit",
        "COMMAND.C:484-488; COMMAND.C:1437-1449");

    state.champions[3].currentHealth = 0;
    state.inventoryChampionOrdinal = 2u;
    state.leaderIndex = 0;
    changed = DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34(
        &state,
        DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT,
        208,
        5,
        DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_ANCHOR(result.nestedCommand == DM1_V1_COMMAND_SET_LEADER_3_PC34_COMPAT,
                 "dead champion target is still selected by C019 before F0368 rejects it",
                 "CLIKCHAM.C:27-30");
    CHECK_ANCHOR(changed == 0 && result.ignoredDeadTarget == 1,
                 "F0368 ignores dead non-none leader targets",
                 "CLIKCHAM.C:51-53");
    CHECK_ANCHOR(state.leaderIndex == 0,
                 "dead target rejection leaves G0411 unchanged",
                 "CLIKCHAM.C:51-53");
}

static void test_f0172_front_wall_sensor_receipt(void)
{
    DM1_V1_ChampionMirrorFrontWallReceiptPc34 receipt;
    DM1_V1_ChampionMirrorRenderReceiptPc34 render;
    DM1_V1_ChampionMirrorThingLayerBoundaryReceiptPc34 boundary;
    DM1_V1_ChampionMirrorThingLayerConsumerReceiptPc34 consumer;
    DM1V1D1LD1RF0115RuntimeThingReceiptPc34 floorThing;
    DM1V1D1LD1RF0115RuntimeThingReceiptPc34 projectileThing;
    const DM1V1D1LD1RF0115LanePc34Data *lane;

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            127, 13, 4, 2, 2, &receipt) == 1 &&
            receipt.valid == 1 &&
            receipt.isFrontMirror == 1 &&
            receipt.championPortraitOrdinal == 14 &&
            receipt.championPortraitRenderIndex == 13 &&
            receipt.wallOrnamentOrdinal == 4,
        "C127 matching wall cell yields source ordinal and render index",
        "DUNGEON.C:2573,2608-2612; COMPILE.H:1038");

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            127, 13, 4, 1, 2, &receipt) == 1 &&
            receipt.valid == 1 &&
            receipt.isFrontMirror == 0 &&
            receipt.championPortraitOrdinal ==
                DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT &&
            receipt.championPortraitRenderIndex ==
                DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT,
        "C127 on a non-visible wall face is ignored",
        "DUNGEON.C:2573");

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            12, 13, 4, 2, 2, &receipt) == 1 &&
            receipt.valid == 1 &&
            receipt.isFrontMirror == 0,
        "non-C127 sensors stay outside champion mirror receipt",
        "DUNGEON.C:2608-2612");

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            127, 13, 4, 2, 2, NULL) == 0,
        "front mirror receipt rejects NULL output",
        "DUNGEON.C:2608-2612");

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            127, 13, 4, 2, 2, &receipt) == 1 &&
            DM1_V1_ChampionMirror_BuildRenderReceiptPc34(
                &receipt, &render) == 1 &&
            render.valid == 1 &&
            render.drawChampionPortrait == 1 &&
            render.drawMirrorBacking == 1 &&
            render.suppressChampionPortrait == 0 &&
            render.sourceOrdinal == 14 &&
            render.renderIndex == 13 &&
            render.graphicIndex ==
                DM1_V1_CHAMPION_MIRROR_PORTRAIT_GRAPHIC_PC34_COMPAT,
        "front mirror render receipt owns C026/C346 draw decision",
        "DUNVIEW.C:3913-3928");

    CHECK_ANCHOR(
        render.sourceX == 160 &&
            render.sourceY == 29 &&
            render.width ==
                DM1_V1_CHAMPION_MIRROR_PORTRAIT_WIDTH_PC34_COMPAT &&
            render.height ==
                DM1_V1_CHAMPION_MIRROR_PORTRAIT_HEIGHT_PC34_COMPAT,
        "render receipt owns C026 atlas source rect",
        "DUNVIEW.C:3916-3919; DEFS.H:821-826");

    CHECK_ANCHOR(
        render.dstX == DM1_V1_CHAMPION_MIRROR_PORTRAIT_DST_X_PC34_COMPAT &&
            render.dstY == DM1_V1_CHAMPION_MIRROR_PORTRAIT_DST_Y_PC34_COMPAT &&
            render.frameLeft ==
                DM1_V1_CHAMPION_MIRROR_PORTRAIT_FRAME_LEFT_PC34_COMPAT &&
            render.frameRight ==
                DM1_V1_CHAMPION_MIRROR_PORTRAIT_FRAME_RIGHT_PC34_COMPAT &&
            render.frameTop ==
                DM1_V1_CHAMPION_MIRROR_PORTRAIT_FRAME_TOP_PC34_COMPAT &&
            render.frameBottom ==
                DM1_V1_CHAMPION_MIRROR_PORTRAIT_FRAME_BOTTOM_PC34_COMPAT &&
            render.transparentColor ==
                DM1_V1_CHAMPION_MIRROR_TRANSPARENT_COLOR_PC34_COMPAT,
        "render receipt owns G0109 destination frame and transparency",
        "DUNVIEW.C:525; DUNVIEW.C:3916-3928");

    CHECK_ANCHOR(
        render.backingGraphicIndex == 346 &&
            render.backingDstX == 80 &&
            render.backingDstY == 29 &&
            render.backingWidth == 64 &&
            render.backingHeight == 43 &&
            render.backingTransparentColor == 10 &&
            render.backingPaletteMapValid == 1,
        "render receipt owns C346 mirror backing material",
        "DUNVIEW.C:3922-3928; DUNVIEW.C G0205 coord-set 5");

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_BuildThingLayerBoundaryReceiptPc34(
            &render, &boundary) == 1 &&
            boundary.valid == 1 &&
            boundary.consumedRenderReceipt == 1 &&
            boundary.drawChampionPortraitAsWallOverlay == 1 &&
            boundary.sourceOrdinal == 14 &&
            boundary.renderIndex == 13 &&
            boundary.graphicIndex ==
                DM1_V1_CHAMPION_MIRROR_PORTRAIT_GRAPHIC_PC34_COMPAT,
        "thing-layer boundary consumes mirror render receipt as wall overlay",
        "DUNVIEW.C:3913-3928");

    CHECK_ANCHOR(
        boundary.suppressMirrorAsFloorItem == 1 &&
            boundary.suppressMirrorAsProjectile == 1 &&
            boundary.suppressMirrorAsSpellEffect == 1 &&
            boundary.suppressMaterializedItemPayload == 1 &&
            boundary.thingLayerSafe == 1,
        "champion mirror payload cannot leak into floor item/projectile layers",
        "DUNVIEW.C:3913-3928; DUNVIEW.C:4547-4581");

    CHECK_ANCHOR(
        boundary.allowIndependentFloorObjects == 1 &&
            boundary.requireRuntimeProjectileReceipt == 1,
        "receipt suppresses mirror payload without blocking real floor objects",
        "DUNVIEW.C:4547-4581; DUNVIEW.C:5668-5683");

    {
        DM1_V1_ChampionMirrorHostDrawReceiptPc34 hostDraw;

        CHECK_ANCHOR(
            DM1_V1_ChampionMirror_BuildHostDrawReceiptPc34(
                &render, 0, 1, &hostDraw) == 1 &&
                hostDraw.valid == 1 &&
                hostDraw.consumedRenderReceipt == 1 &&
                hostDraw.drawChampionPortrait == 1 &&
                hostDraw.drawMirrorBackingAsset == 1 &&
                hostDraw.drawMirrorBackingFallbackRect == 0 &&
                hostDraw.drawInvariantBackingRect == 1 &&
                hostDraw.suppressHostFallbackVisuals == 1 &&
                hostDraw.portraitGraphicIndex ==
                    DM1_V1_CHAMPION_MIRROR_PORTRAIT_GRAPHIC_PC34_COMPAT &&
                hostDraw.backingGraphicIndex == 346,
            "host draw receipt owns normal C346+C026 mirror route",
            "DUNVIEW.C:3913-3928");

        CHECK_ANCHOR(
            DM1_V1_ChampionMirror_BuildHostDrawReceiptPc34(
                &render, 0, 0, &hostDraw) == 1 &&
                hostDraw.valid == 1 &&
                hostDraw.drawMirrorBackingAsset == 0 &&
                hostDraw.drawMirrorBackingFallbackRect == 1 &&
                hostDraw.drawInvariantBackingRect == 1 &&
                hostDraw.suppressHostFallbackVisuals == 1,
            "host draw receipt owns missing C346 backing fallback",
            "DUNVIEW.C:3922-3928");

        CHECK_ANCHOR(
            DM1_V1_ChampionMirror_BuildHostDrawReceiptPc34(
                &render, 1, 1, &hostDraw) == 1 &&
                hostDraw.valid == 1 &&
                hostDraw.candidatePanelOwnsCell == 1 &&
                hostDraw.drawChampionPortrait == 1 &&
                hostDraw.drawMirrorBackingAsset == 0 &&
                hostDraw.drawMirrorBackingFallbackRect == 0 &&
                hostDraw.drawInvariantBackingRect == 0 &&
                hostDraw.suppressWallOrnamentAsset == 1 &&
                hostDraw.suppressHostFallbackVisuals == 1,
            "host draw receipt owns C040 panel mirror suppression",
            "REVIVE.C F0280; DUNVIEW.C:3913-3928");
    }

    lane = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0);
    CHECK_ANCHOR(
        lane != NULL &&
            dm1_v1_viewport_d1l_d1r_f0115_runtime_thing_receipt_pc34(
                lane, 5, 1, 1, 0, &floorThing) == 1 &&
            floorThing.valid == 1 &&
            floorThing.input_valid == 1 &&
            floorThing.draw_item == 1 &&
            floorThing.suppress_item == 0,
        "F0115 runtime receipt keeps a real floor object drawable",
        "DUNVIEW.C:4547-4581; DUNVIEW.C:5075");

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_BuildThingLayerConsumerReceiptPc34(
            &boundary, &floorThing, &consumer) == 1 &&
            consumer.valid == 1 &&
            consumer.consumedBoundaryReceipt == 1 &&
            consumer.consumedRuntimeThingReceipt == 1 &&
            consumer.drawChampionPortraitAsWallOverlay == 1 &&
            consumer.wallOverlayOnly == 1 &&
            consumer.suppressMirrorAsFloorItem == 1 &&
            consumer.suppressMirrorAsProjectile == 1 &&
            consumer.suppressMirrorAsSpellEffect == 1 &&
            consumer.drawFloorObject == 1 &&
            consumer.drawRuntimeProjectile == 0 &&
            consumer.thingLayerSafe == 1,
        "consumer routes mirror to wall overlay while allowing floor object",
        "DUNVIEW.C:3913-3928; DUNVIEW.C:4547-4581; DUNVIEW.C:5075");

    CHECK_ANCHOR(
        dm1_v1_viewport_d1l_d1r_f0115_runtime_thing_receipt_pc34(
            lane, 14, 1, 1, 1, &projectileThing) == 1 &&
            projectileThing.valid == 1 &&
            projectileThing.input_valid == 1 &&
            projectileThing.draw_projectile == 1 &&
            projectileThing.suppress_projectile == 0,
        "F0115 runtime receipt keeps a real projectile drawable",
        "DUNVIEW.C:4547-4581; DUNVIEW.C:5668-5683");

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_BuildThingLayerConsumerReceiptPc34(
            &boundary, &projectileThing, &consumer) == 1 &&
            consumer.valid == 1 &&
            consumer.drawChampionPortraitAsWallOverlay == 1 &&
            consumer.suppressMirrorAsProjectile == 1 &&
            consumer.drawFloorObject == 0 &&
            consumer.drawRuntimeProjectile == 1 &&
            consumer.runtimeProjectileReceiptRequired == 1 &&
            consumer.thingLayerSafe == 1,
        "consumer suppresses mirror projectile leak while allowing projectile",
        "DUNVIEW.C:3913-3928; DUNVIEW.C:5668-5683");

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            127, 13, 4, 1, 2, &receipt) == 1 &&
            DM1_V1_ChampionMirror_BuildRenderReceiptPc34(
                &receipt, &render) == 1 &&
            render.valid == 1 &&
            render.drawChampionPortrait == 0 &&
            render.suppressChampionPortrait == 1 &&
            render.suppressMaterializedItemPayload == 1 &&
            render.renderIndex ==
                DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT,
        "non-front mirror suppresses portrait and stale payload render",
        "DUNGEON.C:2573; DUNVIEW.C:3913");

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_BuildViewportRenderReceiptPc34(
            1, &receipt, &render) == 1 &&
            render.valid == 1 &&
            render.consumedWallSquareReceipt == 1 &&
            render.drawChampionPortrait == 0 &&
            render.suppressChampionPortrait == 1 &&
            render.clearStaleChampionPortraitOrdinal == 1 &&
            render.clearStaleMaterializedItemPayload == 1,
        "wall-square viewport receipt clears stale HoC mirror payload",
        "DUNGEON.C:2558; DUNVIEW.C:3913-3928");

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_BuildThingLayerBoundaryReceiptPc34(
            &render, &boundary) == 1 &&
            boundary.valid == 1 &&
            boundary.drawChampionPortraitAsWallOverlay == 0 &&
            boundary.suppressMirrorAsFloorItem == 1 &&
            boundary.suppressMirrorAsProjectile == 1 &&
            boundary.thingLayerSafe == 1,
        "suppressed viewport mirror still protects thing/projectile layers",
        "DUNGEON.C:2558; DUNVIEW.C:3913-3928");

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_BuildViewportRenderReceiptPc34(
            0, &receipt, &render) == 1 &&
            render.consumedWallSquareReceipt == 0 &&
            render.clearStaleChampionPortraitOrdinal == 0 &&
            render.suppressChampionPortrait == 1,
        "non-wall viewport receipt does not invent a G0289 clear",
        "DUNGEON.C:2558 BUG0_75");

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_BuildRenderReceiptPc34(NULL, &render) == 0 &&
            DM1_V1_ChampionMirror_BuildRenderReceiptPc34(&receipt, NULL) == 0 &&
            DM1_V1_ChampionMirror_BuildViewportRenderReceiptPc34(
                1, NULL, &render) == 0 &&
            DM1_V1_ChampionMirror_BuildViewportRenderReceiptPc34(
                1, &receipt, NULL) == 0 &&
            DM1_V1_ChampionMirror_BuildThingLayerBoundaryReceiptPc34(
                NULL, &boundary) == 0 &&
            DM1_V1_ChampionMirror_BuildThingLayerBoundaryReceiptPc34(
                &render, NULL) == 0 &&
            DM1_V1_ChampionMirror_BuildThingLayerConsumerReceiptPc34(
                NULL, &floorThing, &consumer) == 0 &&
            DM1_V1_ChampionMirror_BuildThingLayerConsumerReceiptPc34(
                &boundary, NULL, &consumer) == 0 &&
            DM1_V1_ChampionMirror_BuildThingLayerConsumerReceiptPc34(
                &boundary, &floorThing, NULL) == 0,
        "render receipt rejects NULL inputs",
        "DM1 receipt guard");
}

int main(void)
{
    test_c159_name_route_without_candidate_panel();
    test_c159_name_route_blocked_while_c040_live();
    test_nested_c019_name_route_without_candidate_panel();
    test_nested_c019_route_blocked_while_c040_live();
    test_f0358_edges_and_f0368_dead_target();
    test_f0172_front_wall_sensor_receipt();

    CHECK_ANCHOR(DM1_V1_ChampionMirror_SourceEvidencePc34() != NULL,
                 "source evidence string is available",
                 "COMMAND.C:484-488; CLIKCHAM.C:24-72");

    printf("PASS dm1_v1_champion_mirror_pc34_compat %d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
