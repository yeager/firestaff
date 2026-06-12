/*
 * DM1 V1 mirror-candidate C040 panel redraw after inventory exit gate
 * test.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206, PC 3.4 path, MEDIA009+):
 *   - PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2330 owns the
 *     close-inventory path.
 *   - PANEL.C F0355:2318-2322 calls F0334_INVENTORY_CloseChest and
 *     applies the !G0299_ui_CandidateChampionOrdinal gate that
 *     suppresses the F0292 inventory-champion redraw when the C040
 *     candidate is live.
 *   - PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 owns the panel
 *     re-derivation. The G0299 non-zero check at line 1654 routes to
 *     F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637.
 *   - PANEL.C F0346:1626 sets G0424_i_PanelContent =
 *     M568_PANEL_RESURRECT_REINCARNATE.
 *   - PANEL.C F0395_MENUS_DrawMovementArrows is the post-exit arrow
 *     draw.
 *   - PANEL.C F0098_DUNGEONVIEW_DrawFloorAndCeiling redraws the
 *     floor/ceiling only.
 *   - COMMAND.C F0357_COMMAND_DiscardAllInput clears the input queue.
 *   - REVIVE.C F0280:124-132 publishes the candidate; F0282:744-806
 *     consumes/clears it. Neither runs on the close path.
 *   - DEFS.H:2088 C30..C37/C38, G0425/G0426, C040, M568, G0299.
 *   - DEFS.H:712-716 C04_CHAMPION_CLOSE_INVENTORY.
 *   - DEFS.H:5876 G0423_i_InventoryChampionOrdinal.
 *
 * Contract-only, no-asset fixture. This gate is the *close+reopen* lane
 * and does NOT pin the F0333 chest-open path, the C040 panel-resurrect
 * confirm path, the C045 food/water accept cross-rotation, the C540
 * scroll-wheel close race, the C028 resurrect-pending non-leader
 * pickup, the lower-arrow owner-ignore guard, the double-open/close
 * guard, the C159 click rotation combo, the C545 food/water accept/
 * drop, save/load, teleporter, party-rotate, leader-rotation, portrait
 * redraw, or F0354 box-variants gates.
 *
 * No real-asset or original-DOS pixel parity claim.
 */

#include "firestaff/dm1/v1/mirror_candidate/panel_redraw_after_inventory_exit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

enum {
    kExpectedDeterministicHash = 0
};

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
    const Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitEvidencePc34 *e =
        dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_evidence_pc34();
    const char *source =
        dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_source_evidence_pc34();

    CHECK("evidence_not_null", e != NULL ? 1 : 0, 1,
          "metadata accessor returns non-NULL");
    CHECK_STR("evidence_chest_close", e ? e->chestCloseAnchor : NULL,
              "F0334_INVENTORY_CloseChest", "CHEST.C F0334");
    CHECK_STR("evidence_candidate_gate", e ? e->candidateGateAnchor : NULL,
              "!G0299_ui_CandidateChampionOrdinal",
              "PANEL.C F0355:2318-2322");
    CHECK_STR("evidence_champion_draw_state",
              e ? e->championDrawStateAnchor : NULL,
              "PANEL.C F0292", "PANEL.C F0292 / CHAMDRAW.C F0292");
    CHECK_STR("evidence_panel_draw", e ? e->panelDrawAnchor : NULL,
              "F0347_INVENTORY_DrawPanel", "PANEL.C F0347");
    CHECK_STR("evidence_panel_resurrect",
              e ? e->panelResurrectAnchor : NULL,
              "F0346_INVENTORY_DrawPanel_ResurrectReincarnate",
              "PANEL.C F0346");
    CHECK_STR("evidence_panel_redraw_on_reopen",
              e ? e->panelRedrawOnReopenAnchor : NULL,
              "M568_PANEL_RESURRECT_REINCARNATE",
              "PANEL.C F0346:1626");
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
    CHECK_STR("evidence_defs_anchor", e ? e->defsAnchor : NULL,
              "C04_CHAMPION_CLOSE_INVENTORY", "DEFS.H:712-716");
    CHECK_STR("evidence_contract_scope", e ? e->contractScope : NULL,
              "close+reopen", "contract scope");

    CHECK_STR("source_chest_close", source, "PANEL.C F0355:2318-2322",
              "PANEL.C F0355:2318-2322");
    CHECK_STR("source_candidate_gate", source, "!G0299_ui_CandidateChampionOrdinal",
              "PANEL.C F0355:2318-2322");
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
    CHECK_STR("source_no_real_asset_claim", source,
              "no real-asset bitmap parity claim",
              "contract-only no-claim marker");
}

static void test_initial_state(void)
{
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 state;

    memset(&state, 0, sizeof(state));
    dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_init_pc34(&state);

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
    CHECK("init.deterministic_seed_lo", (int)(state.deterministicSeed & 0xFFFFu),
          (int)(0x05E5u & 0xFFFFu), "deterministic seed low word");
    CHECK("init.deterministic_seed_hi",
          (int)((state.deterministicSeed >> 16) & 0xFFFFu),
          (int)((0xC040u) & 0xFFFFu), "deterministic seed high word");

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

    CHECK("init.g0426_open_chest", state.g0426OpenChest, 0,
          "DEFS.H:5876-5881 G0426 not open by default");
    CHECK("init.f0280_candidate_publish_count", state.f0280CandidatePublishCount,
          1, "REVIVE.C F0280:124-132 publish count");
    CHECK("init.f0282_candidate_clear_count", state.f0282CandidateClearCount, 0,
          "REVIVE.C F0282:744-806 not run on initial state");
    CHECK("init.f0334_close_count", state.f0334CloseCount, 0,
          "PANEL.C F0355 F0334 not run on initial state");
    CHECK("init.f0292_champion_draw_state_count",
          state.f0292ChampionDrawStateCount, 0,
          "PANEL.C F0355 F0292 not run on initial state");
    CHECK("init.f0347_draw_panel_count", state.f0347DrawPanelCount, 0,
          "PANEL.C F0347 not run on initial state");
    CHECK("init.f0346_resurrect_draw_count", state.f0346ResurrectDrawCount, 0,
          "PANEL.C F0346 not run on initial state");
    CHECK("init.step", (int)state.step, (int)DM1_V1_MC_PRAIE_STEP_PC34_LIVE,
          "step is LIVE on init");
}

static void test_close_inventory_with_c040_live(void)
{
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 state;
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitResultPc34 result;
    int i;

    memset(&state, 0, sizeof(state));
    memset(&result, 0, sizeof(result));
    dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_init_pc34(&state);

    CHECK("run.accepted",
          dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_run_pc34(
              &state, &result),
          1, "close+reopen accepted");
    CHECK("run.reached_exit", result.reachedExit, 1,
          "PANEL.C F0355 close path reached");
    CHECK("run.reached_reopen", result.reachedReopen, 1,
          "PANEL.C F0347 reopen path reached");
    CHECK("run.reached_post", result.reachedPost, 1,
          "post-reopen stabilization reached");

    CHECK("run.f0334_close_count_after_exit",
          result.f0334CloseCountAfterExit, 1,
          "PANEL.C F0355:2318 F0334 called exactly once");
    CHECK("run.f0292_champion_draw_state_count_after_exit",
          result.f0292ChampionDrawStateCountAfterExit, 0,
          "PANEL.C F0355:2318-2322 F0292 suppressed by !G0299 gate");
    CHECK("run.f0395_draw_movement_arrows_after_exit",
          result.f0395DrawMovementArrowsCountAfterExit, 1,
          "PANEL.C F0395 post-exit arrow draw");
    CHECK("run.f0357_discard_input_after_exit",
          result.f0357DiscardInputCountAfterExit, 1,
          "COMMAND.C F0357 post-exit input discard");
    CHECK("run.f0098_draw_floor_ceiling_after_exit",
          result.f0098DrawFloorCeilingCountAfterExit, 1,
          "PANEL.C F0098 post-exit floor/ceiling redraw");
    CHECK("run.f0326_refresh_mouse_pointer_after_exit",
          result.f0326RefreshMousePointerMainLoopCountAfterExit, 1,
          "COMMAND.C F0326 post-exit mouse pointer refresh");
    CHECK("run.f0457_start_draw_enabled_menus_after_exit",
          result.f0457StartDrawEnabledMenusCountAfterExit, 0,
          "MENU.C F0457 not run on close path (only on cancel)");
    CHECK("run.f0280_candidate_publish_count_after_exit",
          result.f0280CandidatePublishCountAfterExit, 1,
          "REVIVE.C F0280 initial publish only");
    CHECK("run.f0282_candidate_clear_count_after_exit",
          result.f0282CandidateClearCountAfterExit, 0,
          "REVIVE.C F0282 not run on close path");

    CHECK("run.f0347_draw_panel_count_after_reopen",
          result.f0347DrawPanelCountAfterReopen, 1,
          "PANEL.C F0347 reopen draw count");
    CHECK("run.f0346_resurrect_draw_count_after_reopen",
          result.f0346ResurrectDrawCountAfterReopen, 1,
          "PANEL.C F0346 reopen resurrect draw count");
    CHECK("run.f0292_champion_draw_state_count_after_reopen",
          result.f0292ChampionDrawStateCountAfterReopen, 0,
          "PANEL.C F0292 not re-fired on reopen (panel re-derives only)");

    CHECK("run.candidate_gate_fired", result.candidateGateFired, 1,
          "PANEL.C F0355:2318-2322 !G0299 gate fired");
    CHECK("run.candidate_gate_counted", result.candidateGateCounted, 1,
          "PANEL.C F0355:2318-2322 !G0299 gate counted");
    CHECK("run.candidate_gate_source_reachable",
          result.candidateGateSourceReachable, 1,
          "PANEL.C F0355:2318-2322 source reachable");
    CHECK("run.candidate_gate_panel_suppressed",
          result.candidateGatePanelSuppressed, 1,
          "PANEL.C F0355:2318-2322 C040 panel suppressed");
    CHECK("run.candidate_gate_champion_redraw_suppressed",
          result.candidateGateChampionRedrawSuppressed, 1,
          "PANEL.C F0355:2318-2322 F0292 suppressed");
    CHECK("run.no_candidate_clear_on_exit", result.noCandidateClearOnExit, 1,
          "REVIVE.C F0282 not run on close path");

    CHECK("run.panel_graphic_stable_across_exit",
          result.panelGraphicStableAcrossExit, 1,
          "PANEL.C F0346:1626 C040 graphic stable");
    CHECK("run.panel_command_stable_across_exit",
          result.panelCommandStableAcrossExit, 1,
          "PANEL.C F0346:1626 M568 command stable");
    CHECK("run.panel_color_stable_across_exit",
          result.panelColorStableAcrossExit, 1,
          "PANEL.C F0346:1626 C06 color stable");
    CHECK("run.panel_owner_slot_stable_across_exit",
          result.panelOwnerSlotStableAcrossExit, 1,
          "PANEL.C F0346 C30 owner slot stable");
    CHECK("run.panel_c038_slot_box_stable_across_exit",
          result.panelC038SlotBoxStableAcrossExit, 1,
          "PANEL.C F0346 C38 slot box stable");
    CHECK("run.panel_mouth_route_stable_across_exit",
          result.panelMouthRouteStableAcrossExit, 1,
          "PANEL.C F0346 C545 mouth route stable");
    CHECK("run.panel_graphic_restored_after_reopen",
          result.panelGraphicRestoredAfterReopen, 1,
          "PANEL.C F0346:1626 C040 graphic restored");
    CHECK("run.panel_command_restored_after_reopen",
          result.panelCommandRestoredAfterReopen, 1,
          "PANEL.C F0346:1626 M568 command restored");
    CHECK("run.panel_color_restored_after_reopen",
          result.panelColorRestoredAfterReopen, 1,
          "PANEL.C F0346:1626 C06 color restored");

    CHECK("run.reopen_routes_to_f0346", result.reopenRoutesToF0346, 1,
          "PANEL.C F0347:1654 routes to F0346 when G0299 non-zero");
    CHECK("run.reopen_f0346_called", result.reopenF0346Called, 1,
          "PANEL.C F0346:1619-1637 called on reopen");
    CHECK("run.reopen_f0346_panel_content_set",
          result.reopenF0346PanelContentSet, 1,
          "PANEL.C F0346:1626 G0424 = M568_PANEL_RESURRECT_REINCARNATE");
    CHECK("run.reopen_f0346_command_set", result.reopenF0346CommandSet, 1,
          "PANEL.C F0346:1626 M568 command set");
    CHECK("run.reopen_f0346_owner_set", result.reopenF0346OwnerSet, 1,
          "PANEL.C F0346 C30 owner slot set");
    CHECK("run.reopen_f0346_color_set", result.reopenF0346ColorSet, 1,
          "PANEL.C F0346 C06 color set");
    CHECK("run.reopen_f0346_slot_box_set", result.reopenF0346SlotBoxSet, 1,
          "PANEL.C F0346 C38 slot box set");
    CHECK("run.reopen_f0346_c040_blit", result.reopenF0346C040Blit, 1,
          "PANEL.C F0346:1626 C040 graphic blit");
    CHECK("run.reopen_f0346_c040_panel_rect",
          result.reopenF0346C040PanelRect, 1,
          "PANEL.C F0346 C30..C37 panel rect in range");
    CHECK("run.reopen_f0346_c038_slot_box_rect",
          result.reopenF0346C038SlotBoxRect, 1,
          "PANEL.C F0346 C38 slot box rect");

    CHECK("run.panel_stayed_c040", result.panelStayedC040, 1,
          "PANEL.C F0346:1626 C040 panel stable");
    CHECK("run.candidate_still_live", result.candidateStillLive, 1,
          "DEFS.H:5694 G0299 still non-zero");
    CHECK("run.candidate_panel_unchanged", result.candidatePanelUnchanged, 1,
          "PANEL.C F0346 candidate panel unchanged");
    CHECK("run.candidate_owner_unchanged", result.candidateOwnerUnchanged, 1,
          "PANEL.C F0346 candidate owner unchanged");
    CHECK("run.candidate_owner_slot_unchanged",
          result.candidateOwnerSlotUnchanged, 1,
          "PANEL.C F0346 candidate owner slot unchanged");
    CHECK("run.c030_chain_preserved", result.c030ChainPreserved, 1,
          "CHAMPION.C F0300/F0301/F0302 C030 chain preserved");
    CHECK("run.leader_hand_preserved", result.leaderHandPreserved, 1,
          "CHAMPION.C F0297/F0298 leader hand preserved");
    CHECK("run.chest_list_preserved", result.chestListPreserved, 1,
          "DEFS.H:5876-5881 G0425 chest list preserved");
    CHECK("run.g0426_open_chest_stable", result.g0426OpenChestStable, 1,
          "DEFS.H:5876-5881 G0426 closed by F0355:2318");

    CHECK("run.f0333_open_count_total", result.f0333OpenCountTotal, 0,
          "CHEST.C F0333 not run on close+reopen path");
    CHECK("run.f0282_candidate_clear_count_total",
          result.f0282CandidateClearCountTotal, 0,
          "REVIVE.C F0282 not run on close+reopen path");
    CHECK("run.f0360_mirror_queue_confirm_count_total",
          result.f0360MirrorQueueConfirmCountTotal, 0,
          "COMMAND.C F0360 mirror confirm not run");
    CHECK("run.f0368_set_leader_count_total", result.f0368SetLeaderCountTotal,
          0, "COMMAND.C F0368 set leader not run");
    CHECK("run.f0219_wall_impact_sound_count_total",
          result.f0219WallImpactSoundCountTotal, 0,
          "PROJEXPL.C F0219 wall impact not run");
    CHECK("run.f0232_door_destroy_count_total", result.f0232DoorDestroyCountTotal,
          0, "PROJEXPL.C F0232 door destroy not run");
    CHECK("run.f0394_set_magic_caster_count_total",
          result.f0394SetMagicCasterCountTotal, 0,
          "CASTER.C F0394 magic caster not run");
    CHECK("run.f0401_telemetry_log_count_total",
          result.f0401TelemetryLogCountTotal, 0,
          "F0401 telemetry log not run");
    CHECK("run.save_load_teleporter_resurrect_commit_forbidden",
          result.saveLoadTeleporterResurrectCommitForbidden, 1,
          "no save/load/teleporter/resurrect-commit on close+reopen");
    CHECK("run.no_save_load", result.noSaveLoad, 1,
          "no save/load on close+reopen path");
    CHECK("run.no_teleporter", result.noTeleporter, 1,
          "no teleporter on close+reopen path");
    CHECK("run.no_resurrect_commit", result.noResurrectCommit, 1,
          "no resurrect-commit on close+reopen path");
    CHECK("run.no_resurrect_cancel", result.noResurrectCancel, 1,
          "no resurrect-cancel on close+reopen path");
    CHECK("run.no_chest_open", result.noChestOpen, 1,
          "no chest-open on close+reopen path");

    CHECK("run.disjoint.contract_only", result.disjoint.contractOnly, 1,
          "contract-only gate");
    CHECK("run.disjoint.c040_redraw_after_chest_close",
          result.disjoint.disjointFromC040RedrawAfterChestClose, 1,
          "disjoint from c040_redraw_after_chest_close");
    CHECK("run.disjoint.c040_chrome_inventory_owner_swap",
          result.disjoint.disjointFromC040ChromeInventoryOwnerSwap, 1,
          "disjoint from c040_chrome_inventory_owner_swap");
    CHECK("run.disjoint.c040_panel_browse_pickup_rotate_race",
          result.disjoint.disjointFromC040PanelBrowsePickupRotateRace, 1,
          "disjoint from c040_panel_browse_pickup_rotate_race");
    CHECK("run.disjoint.c040_close_non_leader_scroll_pickup",
          result.disjoint.disjointFromC040CloseNonLeaderScrollPickup, 1,
          "disjoint from c040_close_non_leader_scroll_pickup");
    CHECK("run.disjoint.c045_food_water_accept_cross_rotation",
          result.disjoint.disjointFromC045FoodWaterAcceptCrossRotation, 1,
          "disjoint from c045_food_water_accept_cross_rotation");
    CHECK("run.disjoint.c545_pickup_while_panel_live",
          result.disjoint.disjointFromC545PickupWhilePanelLive, 1,
          "disjoint from c545_pickup_while_panel_live");
    CHECK("run.disjoint.c545_drop_while_panel_live",
          result.disjoint.disjointFromC545DropWhilePanelLive, 1,
          "disjoint from c545_drop_while_panel_live");
    CHECK("run.disjoint.c545_accept_during_rotation",
          result.disjoint.disjointFromC545AcceptDuringRotation, 1,
          "disjoint from c545_accept_during_rotation");
    CHECK("run.disjoint.inventory_toggle",
          result.disjoint.disjointFromInventoryToggle, 1,
          "disjoint from inventory_toggle");
    CHECK("run.disjoint.resurrect_chest_close_order",
          result.disjoint.disjointFromResurrectChestCloseOrder, 1,
          "disjoint from resurrect_chest_close_order");
    CHECK("run.disjoint.resurrect_reselect_with_inventory_pickup",
          result.disjoint.disjointFromResurrectReselectWithInventoryPickup, 1,
          "disjoint from resurrect_reselect_with_inventory_pickup");
    CHECK("run.disjoint.resurrect_confirm_inventory_interrupt",
          result.disjoint.disjointFromResurrectConfirmInventoryInterrupt, 1,
          "disjoint from resurrect_confirm_inventory_interrupt");
    CHECK("run.disjoint.close_while_resurrect_pending_with_inventory_pickup",
          result.disjoint
              .disjointFromCloseWhileResurrectPendingWithInventoryPickup,
          1, "disjoint from close_while_resurrect_pending_with_inventory_pickup");
    CHECK("run.disjoint.resurrect_double_candidate_race",
          result.disjoint.disjointFromResurrectDoubleCandidateRace, 1,
          "disjoint from resurrect_double_candidate_race");
    CHECK("run.disjoint.resurrect_full_c30_chain",
          result.disjoint.disjointFromResurrectFullC30Chain, 1,
          "disjoint from resurrect_full_c30_chain");
    CHECK("run.disjoint.champion_panel_f0354_box_variants",
          result.disjoint.disjointFromChampionPanelF0354BoxVariants, 1,
          "disjoint from champion_panel_f0354_box_variants");
    CHECK("run.disjoint.champion_panel_hand_slot_priority_source_lock",
          result.disjoint
              .disjointFromChampionPanelHandSlotPrioritySourceLock,
          1, "disjoint from champion_panel_hand_slot_priority_source_lock");
    CHECK("run.disjoint.champion_panel_portrait_state_redraw",
          result.disjoint.disjointFromChampionPanelPortraitStateRedraw, 1,
          "disjoint from champion_panel_portrait_state_redraw");
    CHECK("run.disjoint.champion_panel_portrait_box_blit_gate",
          result.disjoint.disjointFromChampionPanelPortraitBoxBlitGate, 1,
          "disjoint from champion_panel_portrait_box_blit_gate");
    CHECK("run.disjoint.champion_panel_spell_area_overlay",
          result.disjoint.disjointFromChampionPanelSpellAreaOverlay, 1,
          "disjoint from champion_panel_spell_area_overlay");
    CHECK("run.disjoint.chest_close_while_party_rotate_pickup_pending",
          result.disjoint.disjointFromChestCloseWhilePartyRotatePickupPending,
          1, "disjoint from chest_close_while_party_rotate_pickup_pending");
    CHECK("run.disjoint.chest_close_while_candidate_live_non_leader",
          result.disjoint.disjointFromChestCloseWhileCandidateLiveNonLeader, 1,
          "disjoint from chest_close_while_candidate_live_non_leader");
    CHECK("run.disjoint.chest_scroll_wheel_close_race",
          result.disjoint.disjointFromChestScrollWheelCloseRace, 1,
          "disjoint from chest_scroll_wheel_close_race");
    CHECK("run.disjoint.chest_scroll_wheel_resurrect_confirmation",
          result.disjoint.disjointFromChestScrollWheelResurrectConfirmation, 1,
          "disjoint from chest_scroll_wheel_resurrect_confirmation");
    CHECK("run.disjoint.chest_resurrect_rotation_scroll_wheel",
          result.disjoint.disjointFromChestResurrectRotationScrollWheel, 1,
          "disjoint from chest_resurrect_rotation_scroll_wheel");
    CHECK("run.disjoint.chest_open_during_pending",
          result.disjoint.disjointFromChestOpenDuringPending, 1,
          "disjoint from chest_open_during_pending");
    CHECK("run.disjoint.chest_pickup_during_resurrect_pending_non_leader",
          result.disjoint
              .disjointFromChestPickupDuringResurrectPendingNonLeader,
          1, "disjoint from chest_pickup_during_resurrect_pending_non_leader");
    CHECK("run.disjoint.mirror_candidate_inventory_toggle",
          result.disjoint.disjointFromMirrorCandidateInventoryToggle, 1,
          "disjoint from mirror_candidate_inventory_toggle");
    CHECK("run.disjoint.mirror_candidate_double_open_close_guard",
          result.disjoint.disjointFromMirrorCandidateDoubleOpenCloseGuard, 1,
          "disjoint from mirror_candidate_double_open_close_guard");
    CHECK("run.disjoint.mirror_candidate_chest_close_pending_panel",
          result.disjoint.disjointFromMirrorCandidateChestClosePendingPanel, 1,
          "disjoint from mirror_candidate_chest_close_pending_panel");
    CHECK("run.disjoint.mirror_candidate_chest_close_leader_hand_pickup",
          result.disjoint
              .disjointFromMirrorCandidateChestCloseLeaderHandPickup,
          1, "disjoint from mirror_candidate_chest_close_leader_hand_pickup");
    CHECK("run.disjoint.mirror_candidate_chest_open_during_pending",
          result.disjoint.disjointFromMirrorCandidateChestOpenDuringPending, 1,
          "disjoint from mirror_candidate_chest_open_during_pending");
    CHECK("run.disjoint.inventory_champion_switch_hand_carry",
          result.disjoint.disjointFromInventoryChampionSwitchHandCarry, 1,
          "disjoint from inventory_champion_switch_hand_carry");
    CHECK("run.disjoint.mirror_candidate_lower_arrow_guard",
          result.disjoint.disjointFromMirrorCandidateLowerArrowGuard, 1,
          "disjoint from mirror_candidate_lower_arrow_guard");
    CHECK("run.disjoint.mirror_candidate_close_button",
          result.disjoint.disjointFromMirrorCandidateCloseButton, 1,
          "disjoint from mirror_candidate_close_button");
    CHECK("run.disjoint.mirror_candidate_icon_refresh",
          result.disjoint.disjointFromMirrorCandidateIconRefresh, 1,
          "disjoint from mirror_candidate_icon_refresh");
    CHECK("run.disjoint.mirror_candidate_resurrect_champion_switch_reopen",
          result.disjoint
              .disjointFromMirrorCandidateResurrectChampionSwitchReopenRuntime,
          1, "disjoint from mirror_candidate_resurrect_champion_switch_reopen");
    CHECK("run.disjoint.mirror_candidate_reshuffle_panel_live",
          result.disjoint.disjointFromMirrorCandidateReshufflePanelLive, 1,
          "disjoint from mirror_candidate_reshuffle_panel_live");
    CHECK("run.disjoint.mirror_candidate_occupied_hand_panel",
          result.disjoint.disjointFromMirrorCandidateOccupiedHandPanel, 1,
          "disjoint from mirror_candidate_occupied_hand_panel");
    CHECK("run.disjoint.mirror_candidate_click_cancel",
          result.disjoint.disjointFromMirrorCandidateClickCancel, 1,
          "disjoint from mirror_candidate_click_cancel");
    CHECK("run.disjoint.mirror_candidate_click_cancel_with_rotation",
          result.disjoint.disjointFromMirrorCandidateClickCancelWithRotation, 1,
          "disjoint from mirror_candidate_click_cancel_with_rotation");
    CHECK("run.disjoint.mirror_candidate_rotation_during_resurrect_confirmation",
          result.disjoint
              .disjointFromMirrorCandidateRotationDuringResurrectConfirmation,
          1, "disjoint from mirror_candidate_rotation_during_resurrect_confirmation");

    CHECK("run.c030_chain_hash_live_eq_after_exit",
          result.c030ChainHashLive == result.c030ChainHashAfterExit, 1,
          "C030 chain hash stable across exit");
    CHECK("run.c030_chain_hash_live_eq_after_reopen",
          result.c030ChainHashLive == result.c030ChainHashAfterReopen, 1,
          "C030 chain hash stable across reopen");
    CHECK("run.candidate_chain_hash_live_eq_after_exit",
          result.candidateChainHashLive == result.candidateChainHashAfterExit,
          1, "candidate chain hash stable across exit");
    CHECK("run.candidate_chain_hash_live_eq_after_reopen",
          result.candidateChainHashLive ==
              result.candidateChainHashAfterReopen,
          1, "candidate chain hash stable across reopen");
    CHECK("run.chest_list_hash_live_eq_after_exit",
          result.chestListHashLive == result.chestListHashAfterExit, 1,
          "chest list hash stable across exit");
    CHECK("run.chest_list_hash_live_eq_after_reopen",
          result.chestListHashLive == result.chestListHashAfterReopen, 1,
          "chest list hash stable across reopen");
    CHECK("run.panel_hash_live_eq_after_exit",
          result.panelHashLive == result.panelHashAfterExit, 1,
          "panel hash stable across exit");
    CHECK("run.panel_hash_live_eq_after_reopen",
          result.panelHashLive == result.panelHashAfterReopen, 1,
          "panel hash stable across reopen");
    CHECK("run.panel_hash_after_reopen_eq_after_post",
          result.panelHashAfterReopen == result.panelHashAfterPost, 1,
          "panel hash stable after post-reopen stabilization");
    (void)i;
}

static void test_close_with_no_candidate_rejected(void)
{
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitStatePc34 state;
    Dm1V1MirrorCandidatePanelRedrawAfterInventoryExitResultPc34 result;

    memset(&state, 0, sizeof(state));
    memset(&result, 0, sizeof(result));
    dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_init_pc34(&state);

    /* Drop the candidate. The gate's source requires G0299 to be
     * non-zero; with G0299 == 0 the run is rejected because the lane
     * is the *close+reopen with live C040* lane. */
    state.g0299CandidateOrdinal = 0;
    state.c040PanelOpen = 0;
    state.c040PanelGraphic = 0;
    state.c040PanelCommand = 0;
    state.c040PanelColor = 0;
    state.c040PanelOwnerSlot = 0;
    state.c040PanelC038SlotBox = 0;
    state.mouthRouteZone = 0;
    state.mouthRouteCommand = 0;

    CHECK("no_candidate.rejected",
          dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_run_pc34(
              &state, &result),
          0, "no-candidate path is rejected by the gate");
}

int main(void)
{
    test_source_lock_metadata();
    test_initial_state();
    test_close_inventory_with_c040_live();
    test_close_with_no_candidate_rejected();

    if (g_failures) {
        printf("FAIL test_dm1_v1_mirror_candidate_panel_redraw_after_inventory"
               "_exit_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS test_dm1_v1_mirror_candidate_panel_redraw_after_inventory_"
           "exit_pc34_compat failures=0 assertions=%d\n",
           g_assertions);
    return 0;
}
