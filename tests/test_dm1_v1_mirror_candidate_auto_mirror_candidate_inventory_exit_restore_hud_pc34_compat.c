/*
 * DM1 V1 mirror-candidate HUD-overlay restoration across inventory exit
 * gate test.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206, PC 3.4 path, MEDIA009+):
 *   - PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2330 owns the
 *     close-inventory path.
 *   - PANEL.C F0355:2318-2322 calls F0334_INVENTORY_CloseChest and
 *     applies the !G0299_ui_CandidateChampionOrdinal gate that
 *     suppresses the F0292 inventory-champion redraw when the C040
 *     candidate is live. The close path does NOT iterate the other
 *     three champions either, so the HUD overlay for ALL four
 *     champions must remain byte-identical across the inventory exit.
 *   - PANEL.C F0292 (CHAMDRAW.C F0292) is the inventory-champion redraw
 *     the gate suppresses.
 *   - PANEL.C F0293 (CHAMDRAW.C F0293:1117) cascades F0292 for all
 *     four champions and is NOT called on the close path.
 *   - PANEL.C F0296 (CHAMDRAW.C F0296:1185) draws the four champion
 *     icons C113..C116 and is NOT called on the close path.
 *   - PANEL.C F0335 is the chest-data clear and is NOT called on the
 *     close path.
 *   - PANEL.C F0354 (PANEL.C:2195-2242) is the status-box portrait blit
 *     dispatch and is NOT called from F0355 close path.
 *   - PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 owns the panel
 *     re-derivation. The G0299 non-zero check at line 1654 routes to
 *     F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637.
 *   - PANEL.C F0346:1626 sets G0424_i_PanelContent =
 *     M568_PANEL_RESURRECT_REINCARNATE.
 *   - PANEL.C F0395_MENUS_DrawMovementArrows is the post-exit arrow
 *     draw (movement arrows, NOT HUD overlay).
 *   - PANEL.C F0098_DUNGEONVIEW_DrawFloorAndCeiling redraws the
 *     floor/ceiling only (NOT HUD overlay).
 *   - COMMAND.C F0357_COMMAND_DiscardAllInput clears the input queue.
 *   - REVIVE.C F0280:124-132 publishes the candidate; F0282:744-806
 *     consumes/clears it. Neither runs on the close path.
 *   - DEFS.H:2088 C30..C37/C38, G0425/G0426, C040, M568, G0299.
 *   - DEFS.H:712-716 C04_CHAMPION_CLOSE_INVENTORY.
 *   - DEFS.H:5876 G0423_i_InventoryChampionOrdinal.
 *   - DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX.
 *   - DEFS.H:2157 C69_CHAMPION_STATUS_BOX_SPACING.
 *   - DEFS.H:2471 C016_BYTE_WIDTH.
 *   - DEFS.H:1874-1878 M070_HAND_SLOT_INDEX.
 *
 * Contract-only, no-asset fixture. This gate is the *HUD-overlay
 * restoration* lane and does NOT pin the C040 panel pixel survival
 * (covered by panel_redraw_after_inventory_exit_pc34_compat gate), the
 * F0333 chest-open path, the C040 panel-resurrect confirm path, the
 * C045 food/water accept cross-rotation, the C540 scroll-wheel close
 * race, the C028 resurrect-pending non-leader pickup, the lower-arrow
 * owner-ignore guard, the double-open/close guard, the C159 click
 * rotation combo, the C545 food/water accept/drop, save/load,
 * teleporter, party-rotate, leader-rotation, portrait redraw, F0354
 * box-variants, F0292 -> F0354 dispatch, or champion-panel clock-tick
 * / food-water-recompute / name-box-clip / dead-member-hand-refresh /
 * spell-area-clear-on-inventory gates.
 *
 * No real-asset or original-DOS pixel parity claim.
 */

#include "firestaff/dm1/v1/mirror_candidate/auto_mirror_candidate_inventory_exit_restore_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

#define CHECK(ID, GOT, WANT, ANCHOR) \
    do { \
        ++g_assertions; \
        if ((GOT) != (WANT)) { \
            ++g_failures; \
            printf("FAIL %s got=%d want=%d at %s\n", (ID), (int)(GOT), \
                   (int)(WANT), (ANCHOR)); \
        } \
    } while (0)

#define CHECK_U32(ID, GOT, WANT, ANCHOR) \
    do { \
        ++g_assertions; \
        if ((GOT) != (WANT)) { \
            ++g_failures; \
            printf("FAIL %s got=0x%08X want=0x%08X at %s\n", (ID), \
                   (unsigned int)(GOT), (unsigned int)(WANT), (ANCHOR)); \
        } \
    } while (0)

#define CHECK_STR(ID, GOT, NEEDLE, ANCHOR) \
    do { \
        ++g_assertions; \
        if (!(GOT) || strstr((GOT), (NEEDLE)) == NULL) { \
            ++g_failures; \
            printf("FAIL %s missing=\"%s\" at %s\n", (ID), (NEEDLE), \
                   (ANCHOR)); \
        } \
    } while (0)

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudEvidencePc34 *e =
        dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_evidence_pc34();
    const char *source =
        dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_source_evidence_pc34();

    CHECK("evidence_not_null", e != NULL ? 1 : 0, 1,
          "metadata accessor returns non-NULL");
    CHECK_STR("evidence_hud_status_box", e ? e->hudStatusBoxAnchor : NULL,
              "C175_ZONE_FIRST_CHAMPION_STATUS_BOX",
              "DEFS.H:3793 + DEFS.H:2157 status box zones");
    CHECK_STR("evidence_hud_portrait_box", e ? e->hudPortraitBoxAnchor : NULL,
              "M027_PORTRAIT_X/Y", "DEFS.H:825-826 portrait box zones");
    CHECK_STR("evidence_hud_champion_icon",
              e ? e->hudChampionIconAnchor : NULL,
              "F0296", "CHAMDRAW.C F0296:1185 champion icons");
    CHECK_STR("evidence_hud_action_hand", e ? e->hudActionHandAnchor : NULL,
              "M070_HAND_SLOT_INDEX", "DEFS.H:1874-1878 action hand");
    CHECK_STR("evidence_hud_action_icon", e ? e->hudActionIconAnchor : NULL,
              "C020", "DEFS.H action-icon strip");
    CHECK_STR("evidence_hud_bars", e ? e->hudBarsAnchor : NULL,
              "C027_HP", "DEFS.H bars C027/C028/C029");
    CHECK_STR("evidence_hud_food_water", e ? e->hudFoodWaterAnchor : NULL,
              "C545_FOOD_WATER_WARN", "DEFS.H:545 food/water warning");
    CHECK_STR("evidence_hud_mouth_eye", e ? e->hudMouthEyeAnchor : NULL,
              "F0352/F0353", "PANEL.C F0352/F0353 mouth/eye click");
    CHECK_STR("evidence_close_inventory",
              e ? e->closeInventoryAnchor : NULL,
              "C04_CHAMPION_CLOSE_INVENTORY",
              "PANEL.C F0355:2244-2330 close sentinel");
    CHECK_STR("evidence_candidate_gate", e ? e->candidateGateAnchor : NULL,
              "!G0299_ui_CandidateChampionOrdinal",
              "PANEL.C F0355:2318-2322 candidate gate");
    CHECK_STR("evidence_draw_movement_arrows",
              e ? e->drawMovementArrowsAnchor : NULL,
              "F0395_MENUS_DrawMovementArrows", "PANEL.C F0395");
    CHECK_STR("evidence_draw_floor_ceiling",
              e ? e->drawFloorCeilingAnchor : NULL,
              "F0098_DUNGEONVIEW_DrawFloorAndCeiling", "PANEL.C F0098");
    CHECK_STR("evidence_discard_input",
              e ? e->discardInputAnchor : NULL,
              "F0357_COMMAND_DiscardAllInput", "COMMAND.C F0357");
    CHECK_STR("evidence_revive_open", e ? e->reviveOpenAnchor : NULL,
              "F0280:124-132", "REVIVE.C F0280");
    CHECK_STR("evidence_revive_clear", e ? e->reviveClearAnchor : NULL,
              "F0282:744-806", "REVIVE.C F0282");
    CHECK_STR("evidence_panel_draw", e ? e->panelDrawAnchor : NULL,
              "F0347_INVENTORY_DrawPanel", "PANEL.C F0347");
    CHECK_STR("evidence_panel_resurrect",
              e ? e->panelResurrectAnchor : NULL,
              "F0346_INVENTORY_DrawPanel_ResurrectReincarnate",
              "PANEL.C F0346");
    CHECK_STR("evidence_defs_anchor", e ? e->defsAnchor : NULL,
              "C04_CHAMPION_CLOSE_INVENTORY", "DEFS.H:712-716");
    CHECK_STR("evidence_contract_scope", e ? e->contractScope : NULL,
              "HUD-overlay restoration",
              "contract scope label");

    CHECK_STR("source_chest_close", source, "PANEL.C F0355:2318-2322",
              "PANEL.C F0355:2318-2322");
    CHECK_STR("source_candidate_gate", source, "!G0299_ui_CandidateChampionOrdinal",
              "PANEL.C F0355:2318-2322");
    CHECK_STR("source_no_f0293_on_close", source, "F0293",
              "close path does NOT cascade F0293");
    CHECK_STR("source_no_f0296_on_close", source, "F0296",
              "close path does NOT call F0296 object icons");
    CHECK_STR("source_no_f0335_on_close", source, "F0335",
              "close path does NOT call F0335 clear");
    CHECK_STR("source_no_f0354_on_close", source, "F0354",
              "close path does NOT call F0354 box variants");
    CHECK_STR("source_panel_redraw_on_reopen", source,
              "M568_PANEL_RESURRECT_REINCARNATE", "PANEL.C F0346:1626");
    CHECK_STR("source_discard_input", source, "F0357_COMMAND_DiscardAllInput",
              "COMMAND.C F0357");
    CHECK_STR("source_draw_movement_arrows", source,
              "F0395_MENUS_DrawMovementArrows", "PANEL.C F0395");
    CHECK_STR("source_draw_floor_ceiling", source,
              "F0098_DUNGEONVIEW_DrawFloorAndCeiling", "PANEL.C F0098");
    CHECK_STR("source_defs_c04", source, "C04_CHAMPION_CLOSE_INVENTORY",
              "DEFS.H:712-716");
    CHECK_STR("source_defs_c040", source, "C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE",
              "DEFS.H:2200");
    CHECK_STR("source_defs_m568", source, "M568_PANEL_RESURRECT_REINCARNATE",
              "DEFS.H:3001-3008");
    CHECK_STR("source_defs_g0299", source, "G0299_ui_CandidateChampionOrdinal",
              "DEFS.H:5694");
    CHECK_STR("source_defs_c175", source, "C175_ZONE_FIRST_CHAMPION_STATUS_BOX",
              "DEFS.H:3793");
    CHECK_STR("source_defs_c69", source, "C69_CHAMPION_STATUS_BOX_SPACING",
              "DEFS.H:2157");
    CHECK_STR("source_defs_m070", source, "M070_HAND_SLOT_INDEX",
              "DEFS.H:1874-1878");
    CHECK_STR("source_no_real_asset_claim", source,
              "no real-asset bitmap parity claim",
              "contract-only no-claim marker");
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 state;

    memset(&state, 0, sizeof(state));
    dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_init_pc34(&state);

    CHECK("init.party_champion_count", state.partyChampionCount, 4,
          "DEFS.H party champion count");
    CHECK("init.leader_index", state.leaderIndex, 0,
          "CHAMPION.C F0297 leader index");
    CHECK("init.inventory_champion_ordinal", state.inventoryChampionOrdinal, 1,
          "DEFS.H:5876 G0423_i_InventoryChampionOrdinal");
    CHECK("init.candidate_ordinal", state.candidateOrdinal, 4,
          "DEFS.H:5694 G0299_ui_CandidateChampionOrdinal");
    CHECK("init.candidate_panel_graphic", state.candidatePanelGraphic, 40,
          "DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE");
    CHECK("init.candidate_panel_command", state.candidatePanelCommand, 568,
          "DEFS.H:3001-3008 M568_PANEL_RESURRECT_REINCARNATE");
    CHECK("init.candidate_panel_color", state.candidatePanelColor, 10,
          "DEFS.H:2088 C30..C37/C38 panel color");
    CHECK("init.candidate_owner_slot", state.candidateOwnerSlot, 30,
          "DEFS.H:2088 C30..C37 owner slot");
    CHECK("init.candidate_c038_slot_box", state.candidateC038SlotBox, 38,
          "DEFS.H:2088 C38 slot box");
    CHECK("init.close_inventory_sentinel", state.closeInventorySentinel, 4,
          "DEFS.H:712-716 C04_CHAMPION_CLOSE_INVENTORY");
    CHECK("init.special_inventory_sentinel", state.specialInventorySentinel, 5,
          "DEFS.H C05_CHAMPION_SPECIAL_INVENTORY");
    CHECK("init.champion_none_sentinel", state.championNoneSentinel, -1,
          "DEFS.H CM1_CHAMPION_NONE");
    CHECK("init.deterministic_seed_lo",
          (int)(state.deterministicSeed & 0xFFFFu),
          (int)(0x05E5u & 0xFFFFu), "deterministic seed low word");
    CHECK("init.deterministic_seed_hi",
          (int)((state.deterministicSeed >> 16) & 0xFFFFu),
          (int)((0xC040u) & 0xFFFFu), "deterministic seed high word");

    /* HUD-overlay constants. */
    CHECK("init.status_box_width", state.statusBoxWidth, 67,
          "DEFS.H:2157 status box width");
    CHECK("init.status_box_height", state.statusBoxHeight, 29,
          "DEFS.H status box height");
    CHECK("init.status_box_stride", state.statusBoxStride, 69,
          "DEFS.H:2157 C69_CHAMPION_STATUS_BOX_SPACING");
    CHECK("init.portrait_width", state.portraitWidth, 32,
          "DEFS.H:6391-6392 G2078_C32_PortraitWidth");
    CHECK("init.portrait_height", state.portraitHeight, 29,
          "DEFS.H:6391-6392 G2079_C29_PortraitHeight");
    CHECK("init.first_champion_status_box", state.firstChampionStatusBox, 175,
          "DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX");
    CHECK("init.status_box_spacing", state.statusBoxSpacing, 69,
          "DEFS.H:2157 C69_CHAMPION_STATUS_BOX_SPACING");
    CHECK("init.byte_width", state.byteWidth, 16,
          "DEFS.H:2471 C016_BYTE_WIDTH");

    /* Per-champion HUD overlay fields. */
    CHECK("init.champion0_c30_owner", state.c30Owner[0], 30,
          "DEFS.H:2088 C30 slot");
    CHECK("init.champion1_c30_owner", state.c30Owner[1], 31,
          "DEFS.H:2088 C31 slot");
    CHECK("init.champion2_c30_owner", state.c30Owner[2], 32,
          "DEFS.H:2088 C32 slot");
    CHECK("init.champion3_c30_owner", state.c30Owner[3], 33,
          "DEFS.H:2088 C33 slot");
    CHECK("init.champion0_alive", state.alive[0], 1,
          "CHAMPION.C F0280 alive on publish");
    CHECK("init.champion0_current_hp", state.currentHp[0], 100,
          "DEFS.H C027_HP bar current");
    CHECK("init.champion0_max_hp", state.maximumHp[0], 100,
          "DEFS.H C027_HP bar max");
    CHECK("init.champion0_current_stamina", state.currentStamina[0], 100,
          "DEFS.H C028_STAMINA bar current");
    CHECK("init.champion0_current_mana", state.currentMana[0], 50,
          "DEFS.H C029_MANA bar current");
    CHECK("init.champion0_food", state.foodValue[0], 1500,
          "REVIVE.C F0280:160 food seed");
    CHECK("init.champion0_water", state.waterValue[0], 1500,
          "REVIVE.C F0280:161 water seed");
    CHECK("init.champion0_poisoned", state.poisonedState[0], 0,
          "REVIVE.C F0280 poisoned starts clear");
    CHECK("init.champion0_portrait_ordinal", state.portraitOrdinal[0], 0,
          "DEFS.H C026 atlas ordinal 0");
    CHECK("init.mouth_pressed", state.mouthPressed, 0,
          "PANEL.C G0333_B_PressingMouth clear at init");
    CHECK("init.eye_pressed", state.eyePressed, 0,
          "PANEL.C G0331_B_PressingEye clear at init");

    /* C040 panel state. */
    CHECK("init.c040_panel_open", state.c040PanelOpen, 1,
          "PANEL.C F0346:1626 C040 panel live");
    CHECK("init.c040_panel_graphic", state.c040PanelGraphic, 40,
          "PANEL.C F0346:1626 M519_F0020_MAIN_BlitToViewport graphic");
    CHECK("init.c040_panel_command", state.c040PanelCommand, 568,
          "PANEL.C F0346:1626 M568_PANEL_RESURRECT_REINCARNATE");
    CHECK("init.c040_panel_color", state.c040PanelColor, 10,
          "PANEL.C F0346:1626 C06_COLOR_DARK_GREEN tone");
    CHECK("init.c040_panel_owner_slot", state.c040PanelOwnerSlot, 30,
          "PANEL.C F0346 M516_CHAMPIONS owner slot anchor");
    CHECK("init.c040_panel_c038_slot_box", state.c040PanelC038SlotBox, 38,
          "PANEL.C F0346 C38 slot box anchor");
    CHECK("init.g0299_candidate_ordinal", state.g0299CandidateOrdinal, 4,
          "DEFS.H:5694 G0299_ui_CandidateChampionOrdinal");

    /* Chest state. */
    CHECK("init.g0426_open_chest", state.g0426OpenChest, 0,
          "DEFS.H:5876-5881 G0426 not open by default");

    /* Operation counts. */
    CHECK("init.f0280_candidate_publish_count", state.f0280CandidatePublishCount,
          1, "REVIVE.C F0280:124-132 publish count");
    CHECK("init.f0282_candidate_clear_count", state.f0282CandidateClearCount, 0,
          "REVIVE.C F0282:744-806 not run on initial state");
    CHECK("init.f0334_close_count", state.f0334CloseCount, 0,
          "PANEL.C F0355 F0334 not run on initial state");
    CHECK("init.f0292_champion_draw_state_count",
          state.f0292ChampionDrawStateCount, 0,
          "PANEL.C F0355 F0292 not run on initial state");
    CHECK("init.f0293_draw_all_champion_states_count",
          state.f0293DrawAllChampionStatesCount, 0,
          "CHAMDRAW.C F0293 not run on initial state");
    CHECK("init.f0296_draw_changed_object_icons_count",
          state.f0296DrawChangedObjectIconsCount, 0,
          "CHAMDRAW.C F0296 not run on initial state");
    CHECK("init.f0335_clear_champion_data_count",
          state.f0335ClearChampionDataCount, 0,
          "PANEL.C F0335 not run on initial state");
    CHECK("init.f0354_draw_status_box_portrait_count",
          state.f0354DrawStatusBoxPortraitCount, 0,
          "PANEL.C F0354 not run on initial state");
    CHECK("init.f0347_draw_panel_count", state.f0347DrawPanelCount, 0,
          "PANEL.C F0347 not run on initial state");
    CHECK("init.f0346_resurrect_draw_count", state.f0346ResurrectDrawCount, 0,
          "PANEL.C F0346 not run on initial state");
    CHECK("init.step", (int)state.step, (int)DM1_V1_MC_AMCIERH_STEP_PC34_LIVE,
          "step starts at LIVE");

    /* Per-champion F0292 suppression is derived from the aggregate
     * f0292ChampionDrawStateCount (which must be 0). */
    CHECK("init.f0292_champion_draw_state_count_starts_zero",
          state.f0292ChampionDrawStateCount, 0,
          "PANEL.C F0355 close path suppresses per-champion F0292 (aggregate)");
}

static void test_run_through_all_steps(void)
{
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 state;
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudResultPc34 result;

    memset(&state, 0, sizeof(state));
    memset(&result, 0, sizeof(result));
    dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_init_pc34(&state);

    CHECK("run.accepted", dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_run_pc34(&state, &result), 1,
          "run_pc34 accepts the live->exit->reopen->post transition");

    /* Step machine reached all three downstream steps. */
    CHECK("run.reached_exit", result.reachedExit, 1,
          "step machine reaches EXIT after F0334");
    CHECK("run.reached_reopen", result.reachedReopen, 1,
          "step machine reaches REOPEN after F0347/F0346");
    CHECK("run.reached_post", result.reachedPost, 1,
          "step machine reaches POST after the post-redraw hash");
    CHECK("run.step_post",
          (int)state.step, (int)DM1_V1_MC_AMCIERH_STEP_PC34_POST,
          "state.step ends at POST");

    /* Operation counts after exit. */
    CHECK("run.f0334_close_after_exit", result.f0334CloseCountAfterExit, 1,
          "PANEL.C F0355 F0334_INVENTORY_CloseChest fires exactly once");
    CHECK("run.f0292_after_exit", result.f0292ChampionDrawStateCountAfterExit, 0,
          "PANEL.C F0355:2318-2322 candidate gate suppresses F0292");
    CHECK("run.f0293_after_exit", result.f0293DrawAllChampionStatesCountAfterExit, 0,
          "CHAMDRAW.C F0293 not called on close path");
    CHECK("run.f0296_after_exit", result.f0296DrawChangedObjectIconsCountAfterExit, 0,
          "CHAMDRAW.C F0296 not called on close path");
    CHECK("run.f0335_after_exit", result.f0335ClearChampionDataCountAfterExit, 0,
          "PANEL.C F0335 not called on close path");
    CHECK("run.f0354_after_exit", result.f0354DrawStatusBoxPortraitCountAfterExit, 0,
          "PANEL.C F0354 not called on close path");
    CHECK("run.f0395_after_exit", result.f0395DrawMovementArrowsCountAfterExit, 1,
          "PANEL.C F0395 MENUS_DrawMovementArrows on close path");
    CHECK("run.f0357_after_exit", result.f0357DiscardInputCountAfterExit, 1,
          "COMMAND.C F0357_COMMAND_DiscardAllInput on close path");
    CHECK("run.f0098_after_exit", result.f0098DrawFloorCeilingCountAfterExit, 1,
          "PANEL.C F0098_DUNGEONVIEW_DrawFloorAndCeiling on close path");
    CHECK("run.f0077_after_exit", result.f0077MouseEnableScreenUpdateCountAfterExit, 2,
          "IO.C F0077_MOUSE_EnableScreenUpdate_CPSE fired once on exit + once on reopen");
    CHECK("run.f0326_after_exit", result.f0326RefreshMousePointerMainLoopCountAfterExit, 1,
          "COMMAND.C F0326_B_RefreshMousePointerInMainLoop on close path");
    CHECK("run.f0282_after_exit", result.f0282CandidateClearCountAfterExit, 0,
          "REVIVE.C F0282 NOT called on close path");

    /* Operation counts after reopen. */
    CHECK("run.f0347_after_reopen", result.f0347DrawPanelCountAfterReopen, 1,
          "PANEL.C F0347_INVENTORY_DrawPanel on reopen route");
    CHECK("run.f0346_after_reopen", result.f0346ResurrectDrawCountAfterReopen, 1,
          "PANEL.C F0346 resurrect draw on reopen route");
    CHECK("run.f0292_after_reopen", result.f0292ChampionDrawStateCountAfterReopen, 0,
          "PANEL.C F0347 reopen does NOT call F0292");
    CHECK("run.f0293_after_reopen", result.f0293DrawAllChampionStatesCountAfterReopen, 0,
          "CHAMDRAW.C F0293 not called on reopen");
    CHECK("run.f0296_after_reopen", result.f0296DrawChangedObjectIconsCountAfterReopen, 0,
          "CHAMDRAW.C F0296 not called on reopen");
    CHECK("run.f0354_after_reopen", result.f0354DrawStatusBoxPortraitCountAfterReopen, 0,
          "PANEL.C F0354 not called on reopen");
    CHECK("run.f0282_after_reopen", result.f0282CandidateClearCountAfterReopen, 0,
          "REVIVE.C F0282 NOT called on reopen");
    CHECK("run.f0280_after_reopen", result.f0280CandidatePublishCountAfterReopen, 1,
          "REVIVE.C F0280 NOT called again on reopen (publish-only-once)");

    /* Candidate gate state. */
    CHECK("run.candidate_gate_fired", result.candidateGateFired, 1,
          "PANEL.C F0355:2318-2322 gate fires");
    CHECK("run.candidate_gate_counted", result.candidateGateCounted, 1,
          "candidate gate counted as fired");
    CHECK("run.candidate_gate_source_reachable", result.candidateGateSourceReachable, 1,
          "PANEL.C F0355:2318-2322 source reachable");
    CHECK("run.candidate_gate_panel_suppressed", result.candidateGatePanelSuppressed, 1,
          "candidate gate suppresses F0292 inventory-champion redraw");
    CHECK("run.candidate_gate_champion_redraw_suppressed",
          result.candidateGateChampionRedrawSuppressed, 1,
          "candidate gate suppresses F0292 + F0293 cascade");
    CHECK("run.candidate_gate_hud_overlay_stable",
          result.candidateGateHudOverlayStable, 1,
          "candidate gate preserves HUD overlay");
    CHECK("run.no_candidate_clear_on_exit", result.noCandidateClearOnExit, 1,
          "REVIVE.C F0282 must NOT run on the close path");
    CHECK("run.no_candidate_publish_on_exit", result.noCandidatePublishOnExit, 1,
          "REVIVE.C F0280 must NOT run again on the close path");

    /* HUD-overlay suppression contract. */
    CHECK("run.f0292_per_champion_all_zero", result.f0292PerChampionAllZero, 1,
          "F0292 not called for any of the four champions");
    CHECK("run.per_champion_f0292_zero_champ0",
          result.perChampionF0292CountZero[0], 1,
          "F0292 not called for champion 0");
    CHECK("run.per_champion_f0292_zero_champ1",
          result.perChampionF0292CountZero[1], 1,
          "F0292 not called for champion 1");
    CHECK("run.per_champion_f0292_zero_champ2",
          result.perChampionF0292CountZero[2], 1,
          "F0292 not called for champion 2");
    CHECK("run.per_champion_f0292_zero_champ3",
          result.perChampionF0292CountZero[3], 1,
          "F0292 not called for champion 3");
    CHECK("run.f0293_not_called_on_close", result.f0293NotCalledOnClosePath, 1,
          "F0293 cascade not invoked on close path");
    CHECK("run.f0296_not_called_on_close", result.f0296NotCalledOnClosePath, 1,
          "F0296 object icons not invoked on close path");
    CHECK("run.f0335_not_called_on_close", result.f0335NotCalledOnClosePath, 1,
          "F0335 clear not invoked on close path");
    CHECK("run.f0354_not_called_on_close", result.f0354NotCalledOnClosePath, 1,
          "F0354 box variants not invoked on close path");

    /* HUD overlay stability. */
    CHECK("run.hud_status_box_stable_exit", result.hudStatusBoxStableAcrossExit, 1,
          "C151..C154 status boxes byte-identical across exit");
    CHECK("run.hud_status_box_stable_reopen", result.hudStatusBoxStableAcrossReopen, 1,
          "C151..C154 status boxes byte-identical across reopen");
    CHECK("run.hud_portrait_box_stable_exit", result.hudPortraitBoxStableAcrossExit, 1,
          "C155..C158 portrait boxes byte-identical across exit");
    CHECK("run.hud_portrait_box_stable_reopen", result.hudPortraitBoxStableAcrossReopen, 1,
          "C155..C158 portrait boxes byte-identical across reopen");
    CHECK("run.hud_champion_icon_stable_exit", result.hudChampionIconStableAcrossExit, 1,
          "C113..C116 champion icons byte-identical across exit");
    CHECK("run.hud_champion_icon_stable_reopen", result.hudChampionIconStableAcrossReopen, 1,
          "C113..C116 champion icons byte-identical across reopen");
    CHECK("run.hud_action_hand_stable_exit", result.hudActionHandStableAcrossExit, 1,
          "M070 action hand byte-identical across exit");
    CHECK("run.hud_action_hand_stable_reopen", result.hudActionHandStableAcrossReopen, 1,
          "M070 action hand byte-identical across reopen");
    CHECK("run.hud_action_icon_stable_exit", result.hudActionIconStableAcrossExit, 1,
          "C020 action icon byte-identical across exit");
    CHECK("run.hud_action_icon_stable_reopen", result.hudActionIconStableAcrossReopen, 1,
          "C020 action icon byte-identical across reopen");
    CHECK("run.hud_bars_stable_exit", result.hudBarsStableAcrossExit, 1,
          "C027..C029 bars byte-identical across exit");
    CHECK("run.hud_bars_stable_reopen", result.hudBarsStableAcrossReopen, 1,
          "C027..C029 bars byte-identical across reopen");
    CHECK("run.hud_food_water_stable_exit", result.hudFoodWaterStableAcrossExit, 1,
          "C545 food/water warning byte-identical across exit");
    CHECK("run.hud_food_water_stable_reopen", result.hudFoodWaterStableAcrossReopen, 1,
          "C545 food/water warning byte-identical across reopen");
    CHECK("run.hud_mouth_eye_stable_exit", result.hudMouthEyeStableAcrossExit, 1,
          "C033..C039 mouth/eye warning byte-identical across exit");
    CHECK("run.hud_mouth_eye_stable_reopen", result.hudMouthEyeStableAcrossReopen, 1,
          "C033..C039 mouth/eye warning byte-identical across reopen");
    CHECK("run.hud_poison_stable_exit", result.hudPoisonStableAcrossExit, 1,
          "poison warning byte-identical across exit");
    CHECK("run.hud_poison_stable_reopen", result.hudPoisonStableAcrossReopen, 1,
          "poison warning byte-identical across reopen");
    CHECK("run.hud_portrait_ordinal_stable_exit", result.hudPortraitOrdinalStableAcrossExit, 1,
          "C026 portrait ordinals byte-identical across exit");
    CHECK("run.hud_portrait_ordinal_stable_reopen", result.hudPortraitOrdinalStableAcrossReopen, 1,
          "C026 portrait ordinals byte-identical across reopen");

    /* Live-state preservation: panel pixels. */
    CHECK("run.panel_stayed_c040", result.panelStayedC040, 1,
          "C040 panel pixel + command preserved across exit+reopen");
    CHECK("run.candidate_still_live", result.candidateStillLive, 1,
          "G0299_ui_CandidateChampionOrdinal stays non-zero");
    CHECK("run.candidate_panel_unchanged", result.candidatePanelUnchanged, 1,
          "C040 panel graphic/command/color preserved");
    CHECK("run.candidate_owner_unchanged", result.candidateOwnerUnchanged, 1,
          "G0299 ordinal preserved across exit+reopen");
    CHECK("run.candidate_owner_slot_unchanged", result.candidateOwnerSlotUnchanged, 1,
          "C30 owner slot preserved across exit+reopen");
    CHECK("run.c030_chain_preserved", result.c030ChainPreserved, 1,
          "C030 chain stable across exit+reopen");
    CHECK("run.leader_hand_preserved", result.leaderHandPreserved, 1,
          "M070 leader hand stable across exit+reopen");
    CHECK("run.chest_list_preserved", result.chestListPreserved, 1,
          "G0425 chest list stable across exit+reopen");
    CHECK("run.g0426_open_chest_stable", result.g0426OpenChestStable, 1,
          "G0426 open chest stable across exit+reopen");

    /* Panel stability contract. */
    CHECK("run.panel_graphic_stable_exit", result.panelGraphicStableAcrossExit, 1,
          "C040 panel graphic stable across exit");
    CHECK("run.panel_command_stable_exit", result.panelCommandStableAcrossExit, 1,
          "C040 panel command stable across exit");
    CHECK("run.panel_color_stable_exit", result.panelColorStableAcrossExit, 1,
          "C040 panel color stable across exit");
    CHECK("run.panel_owner_slot_stable_exit", result.panelOwnerSlotStableAcrossExit, 1,
          "C040 panel owner slot stable across exit");
    CHECK("run.panel_c038_slot_box_stable_exit", result.panelC038SlotBoxStableAcrossExit, 1,
          "C040 panel C038 slot box stable across exit");
    CHECK("run.panel_mouth_route_stable_exit", result.panelMouthRouteStableAcrossExit, 1,
          "C040 panel mouth route stable across exit");
    CHECK("run.panel_graphic_restored_reopen", result.panelGraphicRestoredAfterReopen, 1,
          "C040 panel graphic restored after reopen");
    CHECK("run.panel_command_restored_reopen", result.panelCommandRestoredAfterReopen, 1,
          "C040 panel command restored after reopen");
    CHECK("run.panel_color_restored_reopen", result.panelColorRestoredAfterReopen, 1,
          "C040 panel color restored after reopen");

    /* Auto-restore invariant. */
    CHECK("run.g0299_preserved_exit", result.g0299PreservedAcrossExit, 1,
          "G0299 preserved across exit (auto-restore invariant)");
    CHECK("run.g0299_preserved_reopen", result.g0299PreservedAcrossReopen, 1,
          "G0299 preserved across reopen (auto-restore invariant)");
    CHECK("run.g0299_source_reachable", result.g0299SourceReachable, 1,
          "G0299 source reachable from F0346 reopen route");
    CHECK("run.f0282_not_called_on_exit", result.f0282NotCalledOnExit, 1,
          "F0282 must NOT run on exit (else candidate would be cleared)");
    CHECK("run.f0282_not_called_on_reopen", result.f0282NotCalledOnReopen, 1,
          "F0282 must NOT run on reopen (else candidate would be cleared)");
    CHECK("run.f0280_not_called_on_exit", result.f0280NotCalledOnExit, 1,
          "F0280 must NOT run again on exit (publish-only-once)");
    CHECK("run.f0280_not_called_on_reopen", result.f0280NotCalledOnReopen, 1,
          "F0280 must NOT run on reopen (publish-only-once)");

    /* Reopen reroute. */
    CHECK("run.reopen_routes_to_f0346", result.reopenRoutesToF0346, 1,
          "PANEL.C F0347:1654 routes to F0346 on reopen");
    CHECK("run.reopen_f0346_called", result.reopenF0346Called, 1,
          "PANEL.C F0346 resurrect draw called");
    CHECK("run.reopen_f0346_panel_content_set", result.reopenF0346PanelContentSet, 1,
          "G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE");
    CHECK("run.reopen_f0346_command_set", result.reopenF0346CommandSet, 1,
          "C040 panel command M568 set on reopen");
    CHECK("run.reopen_f0346_owner_set", result.reopenF0346OwnerSet, 1,
          "C040 panel owner slot C30 set on reopen");
    CHECK("run.reopen_f0346_color_set", result.reopenF0346ColorSet, 1,
          "C040 panel color 10 set on reopen");
    CHECK("run.reopen_f0346_slot_box_set", result.reopenF0346SlotBoxSet, 1,
          "C040 panel C038 slot box set on reopen");
    CHECK("run.reopen_f0346_c040_blit", result.reopenF0346C040Blit, 1,
          "M519_F0020_MAIN_BlitToViewport C040 graphic blit on reopen");
    CHECK("run.reopen_f0346_c040_panel_rect", result.reopenF0346C040PanelRect, 1,
          "C040 panel rect on reopen");
    CHECK("run.reopen_f0346_c038_slot_box_rect", result.reopenF0346C038SlotBoxRect, 1,
          "C038 slot box rect on reopen");

    /* Forbidden operation counters. */
    CHECK("run.f0333_open_count_total", result.f0333OpenCountTotal, 0,
          "F0333 chest-open must NOT run on close+reopen path");
    CHECK("run.f0282_candidate_clear_total", result.f0282CandidateClearCountTotal, 0,
          "F0282 must NOT run on close+reopen path");
    CHECK("run.f0457_start_draw_enabled_menus_total",
          result.f0457StartDrawEnabledMenusCountTotal, 0,
          "F0457_START_DrawEnabledMenus_CPSF must NOT run");
    CHECK("run.f0360_mirror_queue_confirm_total",
          result.f0360MirrorQueueConfirmCountTotal, 0,
          "F0360 mirror-queue confirm must NOT run");
    CHECK("run.f0368_set_leader_total", result.f0368SetLeaderCountTotal, 0,
          "F0368_COMMAND_SetLeader must NOT run");
    CHECK("run.f0219_wall_impact_sound_total", result.f0219WallImpactSoundCountTotal, 0,
          "F0219 wall-impact sound must NOT run");
    CHECK("run.f0232_door_destroy_total", result.f0232DoorDestroyCountTotal, 0,
          "F0232 door destroy must NOT run");
    CHECK("run.f0394_set_magic_caster_total", result.f0394SetMagicCasterCountTotal, 0,
          "F0394 set magic caster must NOT run");
    CHECK("run.f0401_telemetry_log_total", result.f0401TelemetryLogCountTotal, 0,
          "F0401 telemetry log must NOT run");
    CHECK("run.save_load_teleporter_resurrect_commit_forbidden",
          result.saveLoadTeleporterResurrectCommitForbidden, 1,
          "save/load/teleporter/resurrect-commit all forbidden on this lane");
    CHECK("run.no_save_load", result.noSaveLoad, 1,
          "F0333 chest-open (save_load side) must NOT run");
    CHECK("run.no_teleporter", result.noTeleporter, 1,
          "F0219 wall-impact sound (teleporter side) must NOT run");
    CHECK("run.no_resurrect_commit", result.noResurrectCommit, 1,
          "F0282 resurrect-commit must NOT run");
    CHECK("run.no_resurrect_cancel", result.noResurrectCancel, 1,
          "F0282 resurrect-cancel must NOT run");
    CHECK("run.no_chest_open", result.noChestOpen, 1,
          "F0333 chest-open must NOT run on close+reopen path");
    CHECK("run.no_f0292_per_champion", result.noF0292PerChampion, 1,
          "F0292 must NOT be called for any champion on close path");
    CHECK("run.no_f0293_cascade", result.noF0293Cascade, 1,
          "F0293 cascade must NOT be called on close path");
    CHECK("run.no_f0296_object_icon_cascade", result.noF0296ObjectIconCascade, 1,
          "F0296 object-icon cascade must NOT be called on close path");
    CHECK("run.no_f0335_clear_champion_data", result.noF0335ClearChampionData, 1,
          "F0335 clear-champion-data must NOT be called on close path");
    CHECK("run.no_f0354_box_variants_blit", result.noF0354BoxVariantsBlit, 1,
          "F0354 box-variants blit must NOT be called on close path");

    /* Hash invariants. */
    CHECK_U32("run.hud_overlay_hash_stable_exit",
              result.hudOverlayHashAfterExit,
              result.hudOverlayHashLive,
              "HUD-overlay hash stable across exit");
    CHECK_U32("run.hud_overlay_hash_stable_reopen",
              result.hudOverlayHashAfterReopen,
              result.hudOverlayHashLive,
              "HUD-overlay hash stable across reopen");
    CHECK_U32("run.hud_overlay_hash_stable_post",
              result.hudOverlayHashAfterPost,
              result.hudOverlayHashLive,
              "HUD-overlay hash stable across post");
    CHECK_U32("run.c030_chain_hash_stable",
              result.c030ChainHashAfterReopen,
              result.c030ChainHashLive,
              "C030 chain hash stable");
    CHECK_U32("run.candidate_chain_hash_stable",
              result.candidateChainHashAfterReopen,
              result.candidateChainHashLive,
              "candidate chain hash stable");
    CHECK_U32("run.chest_list_hash_stable",
              result.chestListHashAfterReopen,
              result.chestListHashLive,
              "chest list hash stable");
    CHECK_U32("run.panel_hash_stable_exit",
              result.panelHashAfterExit,
              result.panelHashLive,
              "panel hash stable across exit");
    CHECK_U32("run.panel_hash_stable_reopen",
              result.panelHashAfterReopen,
              result.panelHashLive,
              "panel hash stable across reopen");
    CHECK_U32("run.panel_hash_stable_post",
              result.panelHashAfterPost,
              result.panelHashLive,
              "panel hash stable across post");
}

static void test_disjoint_contract(void)
{
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 state;
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudResultPc34 result;

    memset(&state, 0, sizeof(state));
    memset(&result, 0, sizeof(result));
    dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_init_pc34(&state);
    dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_run_pc34(&state, &result);

    /* Disjoint from the existing panel_redraw_after_inventory_exit gate. */
    CHECK("disjoint.contract_only", result.disjoint.contractOnly, 1,
          "contract-only marker");
    CHECK("disjoint.from_panel_redraw_after_inventory_exit",
          result.disjoint.disjointFromPanelRedrawAfterInventoryExit, 1,
          "different lane from the existing panel_redraw_after_inventory_exit gate");

    /* Disjoint from the existing C040 chest-close gates. */
    CHECK("disjoint.from_c040_redraw_after_chest_close",
          result.disjoint.disjointFromC040RedrawAfterChestClose, 1,
          "different lane from c040_redraw_after_chest_close");
    CHECK("disjoint.from_c040_chrome_inventory_owner_swap",
          result.disjoint.disjointFromC040ChromeInventoryOwnerSwap, 1,
          "different lane from c040_chrome_inventory_owner_swap");
    CHECK("disjoint.from_c040_panel_browse_pickup_rotate_race",
          result.disjoint.disjointFromC040PanelBrowsePickupRotateRace, 1,
          "different lane from c040_panel_browse_pickup_rotate_race");
    CHECK("disjoint.from_c040_close_non_leader_scroll_pickup",
          result.disjoint.disjointFromC040CloseNonLeaderScrollPickup, 1,
          "different lane from c040_close_non_leader_scroll_pickup");

    /* Disjoint from C045 / C545 family. */
    CHECK("disjoint.from_c045_food_water_accept_cross_rotation",
          result.disjoint.disjointFromC045FoodWaterAcceptCrossRotation, 1,
          "different lane from c045_food_water_accept_cross_rotation");
    CHECK("disjoint.from_c045_close_after_non_candidate_transition",
          result.disjoint.disjointFromC045CloseAfterNonCandidateTransition, 1,
          "different lane from c045_close_after_non_candidate_transition");
    CHECK("disjoint.from_c045_food_water_close_no_candidate",
          result.disjoint.disjointFromC045FoodWaterCloseNoCandidate, 1,
          "different lane from c045_food_water_close_no_candidate");
    CHECK("disjoint.from_c545_pickup_while_panel_live",
          result.disjoint.disjointFromC545PickupWhilePanelLive, 1,
          "different lane from c545_pickup_while_panel_live");
    CHECK("disjoint.from_c545_drop_while_panel_live",
          result.disjoint.disjointFromC545DropWhilePanelLive, 1,
          "different lane from c545_drop_while_panel_live");
    CHECK("disjoint.from_c545_accept_during_rotation",
          result.disjoint.disjointFromC545AcceptDuringRotation, 1,
          "different lane from c545_accept_during_rotation");
    CHECK("disjoint.from_c545_food_water_accept_cross_rotation",
          result.disjoint.disjointFromC545FoodWaterAcceptCrossRotation, 1,
          "different lane from c545_food_water_accept_cross_rotation");

    /* Disjoint from resurrect confirm gates. */
    CHECK("disjoint.from_resurrect_chest_close_order",
          result.disjoint.disjointFromResurrectChestCloseOrder, 1,
          "different lane from resurrect_chest_close_order");
    CHECK("disjoint.from_resurrect_reselect_with_inventory_pickup",
          result.disjoint.disjointFromResurrectReselectWithInventoryPickup, 1,
          "different lane from resurrect_reselect_with_inventory_pickup");
    CHECK("disjoint.from_resurrect_confirm_inventory_interrupt",
          result.disjoint.disjointFromResurrectConfirmInventoryInterrupt, 1,
          "different lane from resurrect_confirm_inventory_interrupt");
    CHECK("disjoint.from_close_while_resurrect_pending_with_inventory_pickup",
          result.disjoint.disjointFromCloseWhileResurrectPendingWithInventoryPickup, 1,
          "different lane from close_while_resurrect_pending_with_inventory_pickup");
    CHECK("disjoint.from_resurrect_double_candidate_race",
          result.disjoint.disjointFromResurrectDoubleCandidateRace, 1,
          "different lane from resurrect_double_candidate_race");
    CHECK("disjoint.from_resurrect_cross_candidate_clear",
          result.disjoint.disjointFromResurrectCrossCandidateClear, 1,
          "different lane from resurrect_cross_candidate_clear");
    CHECK("disjoint.from_resurrect_full_c30_chain",
          result.disjoint.disjointFromResurrectFullC30Chain, 1,
          "different lane from resurrect_full_c30_chain");

    /* Disjoint from mirror-candidate keyboard / click / panel gates. */
    CHECK("disjoint.from_mirror_candidate_keyboard_rotation_combo",
          result.disjoint.disjointFromMirrorCandidateKeyboardRotationCombo, 1,
          "different lane from mirror_candidate_keyboard_rotation_combo");
    CHECK("disjoint.from_mirror_candidate_keyboard_browse",
          result.disjoint.disjointFromMirrorCandidateKeyboardBrowse, 1,
          "different lane from mirror_candidate_keyboard_browse");
    CHECK("disjoint.from_mirror_candidate_c159_click_rotation_combo",
          result.disjoint.disjointFromMirrorCandidateC159ClickRotationCombo, 1,
          "different lane from mirror_candidate_c159_click_rotation_combo");
    CHECK("disjoint.from_mirror_candidate_full_chain",
          result.disjoint.disjointFromMirrorCandidateFullChain, 1,
          "different lane from mirror_candidate_full_chain");
    CHECK("disjoint.from_mirror_candidate_inventory_portrait_click",
          result.disjoint.disjointFromMirrorCandidateInventoryPortraitClick, 1,
          "different lane from mirror_candidate_inventory_portrait_click");
    CHECK("disjoint.from_mirror_candidate_party_swap",
          result.disjoint.disjointFromMirrorCandidatePartySwap, 1,
          "different lane from mirror_candidate_party_swap");
    CHECK("disjoint.from_mirror_candidate_open_then_reselect",
          result.disjoint.disjointFromMirrorCandidateOpenThenReselect, 1,
          "different lane from mirror_candidate_open_then_reselect");
    CHECK("disjoint.from_mirror_candidate_reselect_after_deposit_with_party_rotate",
          result.disjoint.disjointFromMirrorCandidateReselectAfterDepositWithPartyRotate, 1,
          "different lane from mirror_candidate_reselect_after_deposit_with_party_rotate");

    /* Disjoint from champion-panel gates. */
    CHECK("disjoint.from_champion_panel_f0354_box_variants",
          result.disjoint.disjointFromChampionPanelF0354BoxVariants, 1,
          "different lane from champion_panel_f0354_box_variants");
    CHECK("disjoint.from_champion_panel_hand_slot_priority_source_lock",
          result.disjoint.disjointFromChampionPanelHandSlotPrioritySourceLock, 1,
          "different lane from champion_panel_hand_slot_priority_source_lock");
    CHECK("disjoint.from_champion_panel_portrait_state_redraw",
          result.disjoint.disjointFromChampionPanelPortraitStateRedraw, 1,
          "different lane from champion_panel_portrait_state_redraw");
    CHECK("disjoint.from_champion_panel_portrait_box_blit_gate",
          result.disjoint.disjointFromChampionPanelPortraitBoxBlitGate, 1,
          "different lane from champion_panel_portrait_box_blit_gate");
    CHECK("disjoint.from_champion_panel_spell_area_overlay",
          result.disjoint.disjointFromChampionPanelSpellAreaOverlay, 1,
          "different lane from champion_panel_spell_area_overlay");
    CHECK("disjoint.from_champion_panel_food_water_status_box",
          result.disjoint.disjointFromChampionPanelFoodWaterStatusBox, 1,
          "different lane from champion_panel_food_water_status_box");
    CHECK("disjoint.from_champion_panel_mouth_eye_release",
          result.disjoint.disjointFromChampionPanelMouthEyeRelease, 1,
          "different lane from champion_panel_mouth_eye_release");
    CHECK("disjoint.from_champion_panel_mouth_eye_poison_warning",
          result.disjoint.disjointFromChampionPanelMouthEyePoisonWarning, 1,
          "different lane from champion_panel_mouth_eye_poison_warning");
    CHECK("disjoint.from_champion_panel_pressing_mouth_eye_statusbox",
          result.disjoint.disjointFromChampionPanelPressingMouthEyeStatusbox, 1,
          "different lane from champion_panel_pressing_mouth_eye_statusbox");
    CHECK("disjoint.from_champion_panel_hud_recompute",
          result.disjoint.disjointFromChampionPanelHudRecompute, 1,
          "different lane from champion_panel_hud_recompute");
    CHECK("disjoint.from_champion_panel_action_hand_slot_priority",
          result.disjoint.disjointFromChampionPanelActionHandSlotPriority, 1,
          "different lane from champion_panel_action_hand_slot_priority");
    CHECK("disjoint.from_champion_panel_clock_tick_stat_repaint",
          result.disjoint.disjointFromChampionPanelClockTickStatRepaint, 1,
          "different lane from champion_panel_clock_tick_stat_repaint");
    CHECK("disjoint.from_champion_panel_hud_food_water_recompute",
          result.disjoint.disjointFromChampionPanelHudFoodWaterRecompute, 1,
          "different lane from champion_panel_hud_food_water_recompute");
    CHECK("disjoint.from_champion_panel_name_box_clip",
          result.disjoint.disjointFromChampionPanelNameBoxClip, 1,
          "different lane from champion_panel_name_box_clip");
    CHECK("disjoint.from_champion_panel_dead_member_hand_refresh",
          result.disjoint.disjointFromChampionPanelDeadMemberHandRefresh, 1,
          "different lane from champion_panel_dead_member_hand_refresh");
    CHECK("disjoint.from_champion_panel_spell_area_clear_on_inventory",
          result.disjoint.disjointFromChampionPanelSpellAreaClearOnInventory, 1,
          "different lane from champion_panel_spell_area_clear_on_inventory");

    /* Disjoint from chest gates. */
    CHECK("disjoint.from_chest_close_while_party_rotate_pickup_pending",
          result.disjoint.disjointFromChestCloseWhilePartyRotatePickupPending, 1,
          "different lane from chest_close_while_party_rotate_pickup_pending");
    CHECK("disjoint.from_chest_close_while_candidate_live_non_leader",
          result.disjoint.disjointFromChestCloseWhileCandidateLiveNonLeader, 1,
          "different lane from chest_close_while_candidate_live_non_leader");
    CHECK("disjoint.from_chest_scroll_wheel_close_race",
          result.disjoint.disjointFromChestScrollWheelCloseRace, 1,
          "different lane from chest_scroll_wheel_close_race");
    CHECK("disjoint.from_chest_scroll_wheel_resurrect_confirmation",
          result.disjoint.disjointFromChestScrollWheelResurrectConfirmation, 1,
          "different lane from chest_scroll_wheel_resurrect_confirmation");
    CHECK("disjoint.from_chest_resurrect_rotation_scroll_wheel",
          result.disjoint.disjointFromChestResurrectRotationScrollWheel, 1,
          "different lane from chest_resurrect_rotation_scroll_wheel");
    CHECK("disjoint.from_chest_open_during_pending",
          result.disjoint.disjointFromChestOpenDuringPending, 1,
          "different lane from chest_open_during_pending");
    CHECK("disjoint.from_chest_pickup_during_resurrect_pending_non_leader",
          result.disjoint.disjointFromChestPickupDuringResurrectPendingNonLeader, 1,
          "different lane from chest_pickup_during_resurrect_pending_non_leader");
    CHECK("disjoint.from_chest_deposit_during_leader_rotation",
          result.disjoint.disjointFromChestDepositDuringLeaderRotation, 1,
          "different lane from chest_deposit_during_leader_rotation");
    CHECK("disjoint.from_chest_partial_drop_to_floor_while_chest_open",
          result.disjoint.disjointFromChestPartialDropToFloorWhileChestOpen, 1,
          "different lane from chest_partial_drop_to_floor_while_chest_open");
    CHECK("disjoint.from_chest_inventory_c545_drop_to_leader_hand_already_occupied",
          result.disjoint.disjointFromChestInventoryC545DropToLeaderHandAlreadyOccupied, 1,
          "different lane from chest_inventory_c545_drop_to_leader_hand_already_occupied");
    CHECK("disjoint.from_chest_teleporter_survival_open_g0426",
          result.disjoint.disjointFromChestTeleporterSurvivalOpenG0426, 1,
          "different lane from chest_teleporter_survival_open_g0426");
}

static void test_rejects_without_candidate(void)
{
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 state;
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudResultPc34 result;

    memset(&state, 0, sizeof(state));
    memset(&result, 0, sizeof(result));
    dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_init_pc34(&state);

    /* Drop the candidate. */
    state.g0299CandidateOrdinal = 0;
    state.c040PanelOpen = 0;

    CHECK("rejects.run_pc34_returns_zero",
          dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_run_pc34(&state, &result), 0,
          "without a live C040 candidate the close path is not the auto-mirror lane");
}

static void test_rejects_without_c040_panel_open(void)
{
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 state;
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudResultPc34 result;

    memset(&state, 0, sizeof(state));
    memset(&result, 0, sizeof(result));
    dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_init_pc34(&state);

    /* Close the C040 panel. */
    state.c040PanelOpen = 0;

    CHECK("rejects.no_c040_panel.run_pc34_returns_zero",
          dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_run_pc34(&state, &result), 0,
          "without a C040 panel open the lane is the food/water lane, not the auto-mirror lane");
}

static void test_rejects_with_invalid_inventory_champion(void)
{
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 state;
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudResultPc34 result;

    memset(&state, 0, sizeof(state));
    memset(&result, 0, sizeof(result));
    dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_init_pc34(&state);

    /* Drop the inventory champion ordinal to CM1_CHAMPION_NONE. */
    state.inventoryChampionOrdinal = DM1_V1_MC_AMCIERH_CHAMPION_NONE_PC34;

    CHECK("rejects.no_inventory_champion.run_pc34_returns_zero",
          dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_run_pc34(&state, &result), 0,
          "without an inventory champion the close path cannot fire F0334/F0292 in the right scope");
}

static void test_rejects_null_arguments(void)
{
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudStatePc34 state;
    Dm1V1MirrorCandidateAutoMirrorCandidateInventoryExitRestoreHudResultPc34 result;

    memset(&state, 0, sizeof(state));
    memset(&result, 0, sizeof(result));
    dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_init_pc34(&state);

    CHECK("rejects.null_state",
          dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_run_pc34(NULL, &result), 0,
          "null state must reject");
    CHECK("rejects.null_result",
          dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_run_pc34(&state, NULL), 0,
          "null result must reject");
}

int main(void)
{
    test_source_lock_metadata();
    test_initial_state();
    test_run_through_all_steps();
    test_disjoint_contract();
    test_rejects_without_candidate();
    test_rejects_without_c040_panel_open();
    test_rejects_with_invalid_inventory_champion();
    test_rejects_null_arguments();

    printf("dm1_v1_mirror_candidate_auto_mirror_candidate_inventory_exit_restore_hud_pc34_compat: "
           "assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
