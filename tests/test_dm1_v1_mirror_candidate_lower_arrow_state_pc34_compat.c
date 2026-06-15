/*
 * DM1 V1 mirror-candidate lower movement arrow state gate test.
 *
 * Contract-only, no-asset fixture. See
 * include/dm1_v1_mirror_candidate_lower_arrow_state_pc34_compat.h
 * for the full source-lock anchor table and disjoint contract.
 *
 * This test pins the *narrow* lane of the C004/C005/C006 lower
 * movement arrow click (the lower row y=147-167 of
 * G0463_aai_Graphic561_Box_MovementArrows) being processed by
 * F0380_COMMAND_ProcessQueue_CPSC and dispatched to
 * F0366_COMMAND_ProcessTypes3To6_MoveParty without an explicit
 * !G0299 guard. The C040 panel state and candidate chain MUST stay
 * byte-stable across the click.
 */

#include "dm1_v1_mirror_candidate_lower_arrow_state_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gTests;
static int gPasses;

#define CHECK(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

#define CHECK_STR(haystack, needle, msg, anchor) do { \
    ++gTests; \
    if ((haystack) && strstr((haystack), (needle)) != NULL) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s (missing=\"%s\") [%s]\n", (msg), (needle), (anchor)); \
    } \
} while (0)

static void test_source_lock_metadata(void)
{
    const Dm1V1MirrorCandidateLowerArrowStateEvidencePc34 *e =
        dm1_v1_mirror_candidate_lower_arrow_state_evidence_pc34();
    const char *src =
        dm1_v1_mirror_candidate_lower_arrow_state_source_evidence_pc34();

    CHECK(e != NULL, "evidence accessor returns non-NULL",
          "static evidence table");
    CHECK_STR(e ? e->movementMouseTableAnchor : NULL,
              "G0448_SecondaryMouseInput_Movement",
              "evidence cites G0448 secondary movement mouse input",
              e ? e->movementMouseTableAnchor : "null anchor");
    CHECK_STR(e ? e->movementMouseTableAnchor : NULL,
              "C004_COMMAND_MOVE_RIGHT",
              "evidence cites C004 lower-row right movement",
              e ? e->movementMouseTableAnchor : "null anchor");
    CHECK_STR(e ? e->movementArrowBoxesAnchor : NULL,
              "G0463_aai_Graphic561_Box_MovementArrows",
              "evidence cites G0463 movement arrow boxes",
              e ? e->movementArrowBoxesAnchor : "null anchor");
    CHECK_STR(e ? e->movementArrowBoxesAnchor : NULL,
              "y=147-167",
              "evidence cites the lower-row y=147-167",
              e ? e->movementArrowBoxesAnchor : "null anchor");
    CHECK_STR(e ? e->commandQueueMoveRangeAnchor : NULL,
              "F0380:2151-2156",
              "evidence cites the F0380 2151-2156 move range",
              e ? e->commandQueueMoveRangeAnchor : "null anchor");
    CHECK_STR(e ? e->commandQueueMoveRangeAnchor : NULL,
              "F0366_COMMAND_ProcessTypes3To6_MoveParty",
              "evidence cites the F0366 move-party dispatch",
              e ? e->commandQueueMoveRangeAnchor : "null anchor");
    CHECK_STR(e ? e->commandQueueMoveForwardAnchor : NULL,
              "C003..C006",
              "evidence cites the C003..C006 range",
              e ? e->commandQueueMoveForwardAnchor : "null anchor");
    CHECK_STR(e ? e->commandQueueStatusInventoryGuardAnchor : NULL,
              "!G0299",
              "evidence cites the !G0299 status/inventory guard",
              e ? e->commandQueueStatusInventoryGuardAnchor : "null anchor");
    CHECK_STR(e ? e->commandQueueSpellActionGuardAnchor : NULL,
              "C100 spell",
              "evidence cites the C100 spell-area guard",
              e ? e->commandQueueSpellActionGuardAnchor : "null anchor");
    CHECK_STR(e ? e->commandQueueSaveGuardAnchor : NULL,
              "C140",
              "evidence cites the C140 save guard",
              e ? e->commandQueueSaveGuardAnchor : "null anchor");
    CHECK_STR(e ? e->panelC040RedrawAnchor : NULL,
              "F0346_INVENTORY_DrawPanel_ResurrectReincarnate",
              "evidence cites the F0346 resurrect redraw",
              e ? e->panelC040RedrawAnchor : "null anchor");
    CHECK_STR(e ? e->panelResurrectAnchor : NULL,
              "M568_PANEL_RESURRECT_REINCARNATE",
              "evidence cites M568 panel content sentinel",
              e ? e->panelResurrectAnchor : "null anchor");
    CHECK_STR(e ? e->panelDrawRouteAnchor : NULL,
              "F0347_INVENTORY_DrawPanel:1654",
              "evidence cites F0347:1654 G0299 non-zero route",
              e ? e->panelDrawRouteAnchor : "null anchor");
    CHECK_STR(e ? e->reviveCandidatePublishAnchor : NULL,
              "F0280:124-132",
              "evidence cites the F0280 candidate publish gate",
              e ? e->reviveCandidatePublishAnchor : "null anchor");
    CHECK_STR(e ? e->reviveCandidateClearAnchor : NULL,
              "F0282:744-758",
              "evidence cites the F0282 candidate clear anchor",
              e ? e->reviveCandidateClearAnchor : "null anchor");
    CHECK_STR(e ? e->movePartyNoPanelStateAnchor : NULL,
              "F0366_COMMAND_ProcessTypes3To6_MoveParty",
              "evidence cites F0366 as the move-party path",
              e ? e->movePartyNoPanelStateAnchor : "null anchor");
    CHECK_STR(e ? e->movePartyNoPanelStateAnchor : NULL,
              "G0299",
              "evidence cites that F0366 does not touch G0299",
              e ? e->movePartyNoPanelStateAnchor : "null anchor");
    CHECK_STR(e ? e->defsPanelContentAnchor : NULL,
              "M568_PANEL_RESURRECT_REINCARNATE",
              "evidence cites DEFS.H M568 panel content",
              e ? e->defsPanelContentAnchor : "null anchor");
    CHECK_STR(e ? e->defsPanelGraphicAnchor : NULL,
              "C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE",
              "evidence cites DEFS.H C040 panel graphic",
              e ? e->defsPanelGraphicAnchor : "null anchor");
    CHECK_STR(e ? e->defsCandidateOrdinalAnchor : NULL,
              "G0299_ui_CandidateChampionOrdinal",
              "evidence cites DEFS.H G0299 candidate ordinal",
              e ? e->defsCandidateOrdinalAnchor : "null anchor");
    CHECK_STR(e ? e->defsChestListAnchor : NULL,
              "G0425",
              "evidence cites DEFS.H G0425 chest list",
              e ? e->defsChestListAnchor : "null anchor");
    CHECK_STR(e ? e->movementArrowRowYAnchor : NULL,
              "viewport wall",
              "evidence notes the C004/C005/C006 movement arrows are "
              "screen-relative, not viewport-relative",
              e ? e->movementArrowRowYAnchor : "null anchor");
    CHECK_STR(e ? e->contractScope : NULL, "contract_only=1",
              "fixture is explicitly contract-only",
              e ? e->contractScope : "null scope");
    CHECK_STR(e ? e->contractScope : NULL, "deterministic",
              "fixture is deterministic",
              e ? e->contractScope : "null scope");

    CHECK_STR(src, "G0448_SecondaryMouseInput_Movement",
              "source cites G0448 movement mouse input",
              "ReDMCSB COMMAND.C:107-112");
    CHECK_STR(src, "G0463_aai_Graphic561_Box_MovementArrows",
              "source cites G0463 movement arrow boxes",
              "ReDMCSB COMMAND.C:323-328");
    CHECK_STR(src, "F0380:2151-2156",
              "source cites F0380:2151-2156 C003..C006 move range",
              "ReDMCSB COMMAND.C F0380:2151-2156");
    CHECK_STR(src, "F0366_COMMAND_ProcessTypes3To6_MoveParty",
              "source cites F0366 move-party dispatch",
              "ReDMCSB CLIKMENU.C F0366");
    CHECK_STR(src, "F0346_INVENTORY_DrawPanel_ResurrectReincarnate",
              "source cites F0346 resurrect redraw",
              "ReDMCSB PANEL.C F0346");
    CHECK_STR(src, "F0280:124-132",
              "source cites F0280 candidate publish gate",
              "ReDMCSB REVIVE.C F0280");
    CHECK_STR(src, "F0282:744-758",
              "source cites F0282 candidate clear",
              "ReDMCSB REVIVE.C F0282");
    CHECK_STR(src, "DEFS.H:2088",
              "source cites DEFS.H:2088 C30..C37/C38",
              "ReDMCSB DEFS.H:2088");
    CHECK_STR(src, "no real-asset bitmap parity claim",
              "source includes the no-real-asset claim",
              "contract-only no-claim marker");
}

static void publish_live_candidate(
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 *state)
{
    int published;

    dm1_v1_mirror_candidate_lower_arrow_state_init_pc34(state);
    published =
        dm1_v1_mirror_candidate_lower_arrow_state_publish_candidate_pc34(
            state);

    CHECK(published == 1,
          "portrait sensor publishes one mirror candidate",
          "REVIVE.C F0280:124-132");
    CHECK(state->candidateOrdinal == 3,
          "G0299 candidate ordinal becomes the appended champion",
          "REVIVE.C F0280:272-276");
    CHECK(state->c040PanelOpen == 1,
          "C040 panel is live after publish",
          "PANEL.C F0346:1619-1637");
    CHECK(state->c040PanelGraphic == 40,
          "C040 panel graphic is C040 (40)",
          "DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE");
    CHECK(state->c040PanelCommand == 568,
          "C040 panel command is M568 (568)",
          "DEFS.H:3001-3008 M568_PANEL_RESURRECT_REINCARNATE");
    CHECK(state->panelContent == 568,
          "G0424 panel content is M568",
          "PANEL.C F0346:1626");
    CHECK(state->leaderHandEmpty == 1,
          "publish fixture satisfies the leader-empty guard",
          "REVIVE.C F0280:124-132");
}

static void test_lower_arrow_c004_keeps_c040_state(void)
{
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 state;
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 result;
    int ok;

    publish_live_candidate(&state);
    ok = dm1_v1_mirror_candidate_lower_arrow_state_dispatch_pc34(
        &state, 4 /* C004_COMMAND_MOVE_RIGHT */, &result);

    CHECK(ok == 1, "C004 lower arrow click is accepted by F0380",
          "COMMAND.C F0380:2151-2156");
    CHECK(result.lowerArrowC004Accepted == 1,
          "C004 lower arrow is the accepted lower-row right command",
          "COMMAND.C:107-112 G0448_SecondaryMouseInput_Movement");
    CHECK(result.reachedF0366MoveParty == 1,
          "C004 reaches F0366_COMMAND_ProcessTypes3To6_MoveParty",
          "CLIKMENU.C F0366_COMMAND_ProcessTypes3To6_MoveParty");
    CHECK(result.reachedF0380QueueDrain == 1,
          "C004 reaches the F0380_COMMAND_ProcessQueue_CPSC drain",
          "COMMAND.C F0380:2151-2156");
    CHECK(result.f0380DrainCountRecorded == 1,
          "F0380 drain count is recorded",
          "COMMAND.C F0380:2151-2156");
    CHECK(result.f0366MovePartyCountRecorded == 1,
          "F0366 move-party entry is recorded",
          "CLIKMENU.C F0366:180-280");
    CHECK(result.panelStayedC040 == 1,
          "C040 panel stays open across C004 lower arrow",
          "PANEL.C F0346:1619-1637");
    CHECK(result.panelContentStayedM568 == 1,
          "G0424 panel content stays M568 across C004 lower arrow",
          "PANEL.C F0346:1626");
    CHECK(result.panelGraphicStayed40 == 1,
          "C040 panel graphic stays 40 across C004 lower arrow",
          "DEFS.H:2200 C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE");
    CHECK(result.panelCommandStayed568 == 1,
          "C040 panel command stays 568 across C004 lower arrow",
          "DEFS.H:3001-3008 M568_PANEL_RESURRECT_REINCARNATE");
    CHECK(result.panelOwnerSlotStayedCandidateOwner == 1,
          "C040 panel owner slot stays at the candidate owner",
          "PANEL.C F0346 M516_CHAMPIONS owner slot anchor");
    CHECK(result.panelC038SlotBoxStayed38 == 1,
          "C040 panel C038 slot box anchor stays 38",
          "DEFS.H:2088 C38");
    CHECK(result.candidateOrdinalPreserved == 1,
          "G0299 candidate ordinal is preserved across C004",
          "REVIVE.C F0280:124-132");
    CHECK(result.panelOpenPreserved == 1,
          "c040PanelOpen is preserved across C004",
          "PANEL.C F0346:1619-1637");
    CHECK(result.ownerChainPreserved == 1,
          "C30..C37 owner chain is byte-stable across C004",
          "CHAMPION.C F0300/F0301/F0302");
    CHECK(result.leaderHandPreserved == 1,
          "Leader hand is byte-stable across C004",
          "CHAMPION.C F0297/F0298");
    CHECK(result.chestListPreserved == 1,
          "G0425 chest list is byte-stable across C004",
          "DEFS.H:5876-5881 G0425");
    CHECK(result.noF0282ClearOnLowerArrow == 1,
          "F0282 candidate clear does NOT run on C004 lower arrow",
          "REVIVE.C F0282:744-806");
    CHECK(result.noF0333OpenOnLowerArrow == 1,
          "F0333 chest open does NOT run on C004 lower arrow",
          "CHEST.C F0333:30-75");
    CHECK(result.noF0334CloseOnLowerArrow == 1,
          "F0334 chest close does NOT run on C004 lower arrow",
          "CHEST.C F0334:117-132");
    CHECK(result.noF0342ObjectOnLowerArrow == 1,
          "F0342 inventory object draw does NOT run on C004",
          "PANEL.C F0342:1119-1135");
    CHECK(result.noF0345FoodWaterOnLowerArrow == 1,
          "F0345 food/water poisoned draw does NOT run on C004",
          "PANEL.C F0345:1563-1617");
    CHECK(result.noF0355ToggleOnLowerArrow == 1,
          "F0355 inventory toggle does NOT run on C004",
          "PANEL.C F0355:2244-2330");
    CHECK(result.noF0395MovementArrowsOnLowerArrow == 1,
          "F0395 movement arrows draw does NOT run on C004",
          "PANEL.C F0395_MENUS_DrawMovementArrows");
    CHECK(result.noF0457StartDrawEnabledOnLowerArrow == 1,
          "F0457 draw enabled menus does NOT run on C004",
          "F0457_START_DrawEnabledMenus_CPSF");
    CHECK(result.noF0219WallImpactOnLowerArrow == 1,
          "F0219 wall impact projectile sound does NOT run on C004",
          "PROJEXPL.C F0219");
    CHECK(result.noF0232DoorDestroyOnLowerArrow == 1,
          "F0232 door destroy does NOT run on C004",
          "PROJEXPL.C F0232_GROUP_IsDoorDestroyedByAttack");
    CHECK(result.noF0283ViAltarRebirthOnLowerArrow == 1,
          "F0283 Vi Altar rebirth does NOT run on C004",
          "REVIVE.C F0283_CHAMPION_ViAltarRebirth");
    CHECK(result.noF0281ChampionRenameOnLowerArrow == 1,
          "F0281 champion rename does NOT run on C004",
          "REVIVE.C F0281_CHAMPION_Rename");
    CHECK(result.noSaveLoadOnLowerArrow == 1,
          "Save/load does NOT run on C004",
          "COMMAND.C F0380:2366-2370");
    CHECK(result.noTeleporterOnLowerArrow == 1,
          "Teleporter does NOT run on C004",
          "TELEPORT.C");
    CHECK(result.noPartyRotateOnLowerArrow == 1,
          "Party rotate does NOT run on C004",
          "CHAMPION.C F0284");
    CHECK(result.noLeaderRotationOnLowerArrow == 1,
          "Leader rotation pending does NOT run on C004",
          "CHAMPION.C F0302");
    CHECK(result.noResurrectCommitOnLowerArrow == 1,
          "Resurrect commit does NOT run on C004",
          "REVIVE.C F0282:785-806");
    CHECK(result.noResurrectCancelOnLowerArrow == 1,
          "Resurrect cancel does NOT run on C004",
          "REVIVE.C F0282:744-758");
    CHECK(result.c040PanelHashStable == 1,
          "C040 panel fingerprint is stable across C004",
          "PANEL.C F0346:1619-1637");
    CHECK(result.candidateChainHashStable == 1,
          "Candidate chain fingerprint is stable across C004",
          "REVIVE.C F0280:124-132");
    CHECK(result.c030OwnerHashStable == 1,
          "C30 owner chain fingerprint is stable across C004",
          "CHAMPION.C F0300/F0301/F0302");
    CHECK(result.leaderHandHashStable == 1,
          "Leader hand fingerprint is stable across C004",
          "CHAMPION.C F0297/F0298");
    CHECK(result.chestListHashStable == 1,
          "Chest list fingerprint is stable across C004",
          "DEFS.H:5876-5881 G0425");
    CHECK(result.noPanelContentMutation == 1,
          "G0424 panel content is not mutated by C004",
          "PANEL.C F0346:1626");
    CHECK(result.disjoint.disjointFromC159ClickRotationCombo == 1,
          "disjoint from c159_click_rotation_combo",
          "COMMAND.C:484-488");
    CHECK(result.disjoint.disjointFromC160CloseWhileRotationPending == 1,
          "disjoint from c160_close_while_rotation_pending",
          "COMMAND.C F0359:1985-1990");
    CHECK(result.disjoint.disjointFromPanelRedrawAfterInventoryExit == 1,
          "disjoint from panel_redraw_after_inventory_exit",
          "PANEL.C F0355:2244-2330");
    CHECK(result.disjoint.disjointFromChestScrollWheelCloseRace == 1,
          "disjoint from chest_scroll_wheel_close_race",
          "CHEST.C F0333/F0334");
    CHECK(result.disjoint.disjointFromMirrorCandidateSaveLoad == 1,
          "disjoint from mirror_candidate_save_load",
          "LOADSAVE.C F0433");
    CHECK(result.disjoint.disjointFromMirrorCandidateTeleporterSurvival == 1,
          "disjoint from mirror_candidate_teleporter_survival",
          "TELEPORT.C");
    CHECK(result.disjoint.disjointFromSaveLoadContract == 1,
          "disjoint from save/load contract",
          "LOADSAVE.C");
    CHECK(result.disjoint.disjointFromTeleporterContract == 1,
          "disjoint from teleporter contract",
          "TELEPORT.C");
    CHECK(result.disjoint.disjointFromLeaderRotationContract == 1,
          "disjoint from leader-rotation contract",
          "CHAMPION.C F0302");
    CHECK(result.disjoint.disjointFromPartyRotateContract == 1,
          "disjoint from party-rotate contract",
          "CHAMPION.C F0284");
    CHECK(state.f0334VisibleRewriteCount == 0,
          "F0334 visible rewrite count stays zero on C004",
          "CHEST.C F0334:117-132");
    CHECK(state.f0333OpenCount == 0,
          "F0333 chest open count stays zero on C004",
          "CHEST.C F0333:30-75");
    CHECK(state.f0282CandidateClearCount == 0,
          "F0282 candidate clear count stays zero on C004",
          "REVIVE.C F0282:744-806");
    CHECK(state.f0345FoodWaterPoisonedCount == 0,
          "F0345 food/water poisoned count stays zero on C004",
          "PANEL.C F0345:1563-1617");
    CHECK(state.f0342DrawPanelObjectCount == 0,
          "F0342 object draw count stays zero on C004",
          "PANEL.C F0342:1119-1135");
    CHECK(state.f0355ToggleSuppressedByCandidateCount == 0,
          "F0355 inventory toggle count stays zero on C004",
          "PANEL.C F0355:2244-2330");
    CHECK(state.f0395DrawMovementArrowsCount == 0,
          "F0395 movement arrows count stays zero on C004",
          "PANEL.C F0395_MENUS_DrawMovementArrows");
    CHECK(state.f0457StartDrawEnabledMenusCount == 0,
          "F0457 start draw enabled count stays zero on C004",
          "F0457_START_DrawEnabledMenus_CPSF");
    CHECK(state.f0219WallImpactSoundCount == 0,
          "F0219 wall impact sound count stays zero on C004",
          "PROJEXPL.C F0219");
    CHECK(state.f0232DoorDestroyCount == 0,
          "F0232 door destroy count stays zero on C004",
          "PROJEXPL.C F0232_GROUP_IsDoorDestroyedByAttack");
    CHECK(state.saveLoadCount == 0,
          "save/load count stays zero on C004",
          "LOADSAVE.C");
    CHECK(state.teleporterCount == 0,
          "teleporter count stays zero on C004",
          "TELEPORT.C");
    CHECK(state.partyRotateCount == 0,
          "party rotate count stays zero on C004",
          "CHAMPION.C F0284");
    CHECK(state.championRotationPendingCount == 0,
          "leader rotation pending count stays zero on C004",
          "CHAMPION.C F0302");
    CHECK(state.f0283ViAltarRebirthCount == 0,
          "F0283 Vi Altar rebirth count stays zero on C004",
          "REVIVE.C F0283");
    CHECK(state.f0281ChampionRenameCount == 0,
          "F0281 champion rename count stays zero on C004",
          "REVIVE.C F0281");
}

static void test_lower_arrow_c005_keeps_c040_state(void)
{
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 state;
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 result;
    int ok;

    publish_live_candidate(&state);
    ok = dm1_v1_mirror_candidate_lower_arrow_state_dispatch_pc34(
        &state, 5 /* C005_COMMAND_MOVE_BACKWARD */, &result);

    CHECK(ok == 1, "C005 lower arrow click is accepted by F0380",
          "COMMAND.C F0380:2151-2156");
    CHECK(result.lowerArrowC005Accepted == 1,
          "C005 lower arrow is the accepted lower-row backward command",
          "COMMAND.C:107-112 G0448_SecondaryMouseInput_Movement");
    CHECK(result.reachedF0366MoveParty == 1,
          "C005 reaches F0366_COMMAND_ProcessTypes3To6_MoveParty",
          "CLIKMENU.C F0366_COMMAND_ProcessTypes3To6_MoveParty");
    CHECK(result.panelStayedC040 == 1,
          "C040 panel stays open across C005 lower arrow",
          "PANEL.C F0346:1619-1637");
    CHECK(result.panelContentStayedM568 == 1,
          "G0424 panel content stays M568 across C005",
          "PANEL.C F0346:1626");
    CHECK(result.candidateOrdinalPreserved == 1,
          "G0299 candidate ordinal is preserved across C005",
          "REVIVE.C F0280:124-132");
    CHECK(result.noF0282ClearOnLowerArrow == 1,
          "F0282 candidate clear does NOT run on C005",
          "REVIVE.C F0282:744-806");
    CHECK(state.f0380DrainCount == 1,
          "F0380 drain count is exactly 1 after C005",
          "COMMAND.C F0380:2151-2156");
    CHECK(state.f0366MovePartyEnterCount == 1,
          "F0366 move-party entry count is exactly 1 after C005",
          "CLIKMENU.C F0366:180-280");
}

static void test_lower_arrow_c006_keeps_c040_state(void)
{
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 state;
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 result;
    int ok;

    publish_live_candidate(&state);
    ok = dm1_v1_mirror_candidate_lower_arrow_state_dispatch_pc34(
        &state, 6 /* C006_COMMAND_MOVE_LEFT */, &result);

    CHECK(ok == 1, "C006 lower arrow click is accepted by F0380",
          "COMMAND.C F0380:2151-2156");
    CHECK(result.lowerArrowC006Accepted == 1,
          "C006 lower arrow is the accepted lower-row left command",
          "COMMAND.C:107-112 G0448_SecondaryMouseInput_Movement");
    CHECK(result.reachedF0366MoveParty == 1,
          "C006 reaches F0366_COMMAND_ProcessTypes3To6_MoveParty",
          "CLIKMENU.C F0366_COMMAND_ProcessTypes3To6_MoveParty");
    CHECK(result.panelStayedC040 == 1,
          "C040 panel stays open across C006 lower arrow",
          "PANEL.C F0346:1619-1637");
    CHECK(result.candidateOrdinalPreserved == 1,
          "G0299 candidate ordinal is preserved across C006",
          "REVIVE.C F0280:124-132");
    CHECK(state.f0366MovePartyEnterCount == 1,
          "F0366 move-party entry count is exactly 1 after C006",
          "CLIKMENU.C F0366:180-280");
}

static void test_upper_arrow_c003_still_dispatches_without_c040_change(void)
{
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 state;
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 result;
    int ok;

    publish_live_candidate(&state);
    ok = dm1_v1_mirror_candidate_lower_arrow_state_dispatch_pc34(
        &state, 3 /* C003_COMMAND_MOVE_FORWARD */, &result);

    CHECK(ok == 1, "C003 forward arrow click is accepted by F0380",
          "COMMAND.C F0380:2151-2156");
    CHECK(result.upperArrowC003Accepted == 1,
          "C003 forward is the upper-row accepted command",
          "COMMAND.C:107-112 G0448_SecondaryMouseInput_Movement");
    CHECK(result.reachedF0366MoveParty == 1,
          "C003 also reaches F0366_COMMAND_ProcessTypes3To6_MoveParty",
          "CLIKMENU.C F0366_COMMAND_ProcessTypes3To6_MoveParty");
    CHECK(result.panelStayedC040 == 1,
          "C040 panel stays open across C003 forward",
          "PANEL.C F0346:1619-1637");
    CHECK(result.candidateOrdinalPreserved == 1,
          "G0299 candidate ordinal is preserved across C003",
          "REVIVE.C F0280:124-132");
    CHECK(result.lowerArrowC004Accepted == 0 &&
          result.lowerArrowC005Accepted == 0 &&
          result.lowerArrowC006Accepted == 0,
          "C003 is NOT marked as a lower-row acceptance",
          "COMMAND.C:323-328 G0463");
}

static void test_inventory_toggle_blocked_by_g0299(void)
{
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 state;
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 result;
    int ok;

    publish_live_candidate(&state);
    ok = dm1_v1_mirror_candidate_lower_arrow_state_dispatch_pc34(
        &state, 7 /* C007_COMMAND_TOGGLE_INVENTORY_CHAMPION_0 */, &result);

    CHECK(ok == 1,
          "C007 inventory toggle drain is recorded even though gated by !G0299",
          "COMMAND.C F0380:2159-2181");
    CHECK(result.inventoryToggleC007Accepted == 1,
          "C007 is recorded as the inventory toggle acceptance",
          "COMMAND.C F0380:2159-2181");
    CHECK(result.panelStayedC040 == 1,
          "C040 panel stays open after blocked C007 toggle",
          "PANEL.C F0346:1619-1637");
    CHECK(result.candidateOrdinalPreserved == 1,
          "G0299 candidate ordinal is preserved across blocked C007",
          "REVIVE.C F0280:124-132");
    CHECK(result.noF0355ToggleOnLowerArrow == 0,
          "F0355 toggle-suppression count is recorded on C007",
          "PANEL.C F0355:2244-2330");
}

static void test_close_inventory_c011_path(void)
{
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 state;
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 result;
    int ok;

    publish_live_candidate(&state);
    ok = dm1_v1_mirror_candidate_lower_arrow_state_dispatch_pc34(
        &state, 11 /* C011_COMMAND_CLOSE_INVENTORY */, &result);

    CHECK(ok == 1, "C011 close-inventory drain is recorded",
          "COMMAND.C F0380:2159-2181");
    CHECK(result.closeInventoryC011Accepted == 1,
          "C011 is recorded as the close-inventory acceptance",
          "COMMAND.C F0380:2159-2181");
    CHECK(result.noF0355ToggleOnLowerArrow == 0,
          "F0355 toggle-suppression count is recorded on C011",
          "PANEL.C F0355:2244-2330");
}

static void test_panel_command_c160_path(void)
{
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 state;
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 result;
    int ok;

    publish_live_candidate(&state);
    ok = dm1_v1_mirror_candidate_lower_arrow_state_dispatch_pc34(
        &state, 160 /* C160_COMMAND_CLICK_IN_PANEL_RESURRECT */, &result);

    CHECK(ok == 1, "C160 panel command drain is recorded",
          "COMMAND.C F0378_COMMAND_ProcessType81_ClickInPanel");
    CHECK(result.panelCommandC160Accepted == 1,
          "C160 is recorded as the panel resurrect command",
          "PANEL.C F0346:1619-1637");
    CHECK(state.f0378PanelRouteCount == 1,
          "F0378 panel route count is exactly 1 after C160",
          "COMMAND.C F0378_COMMAND_ProcessType81_ClickInPanel");
    CHECK(result.candidateOrdinalPreserved == 1,
          "G0299 candidate ordinal is preserved in this lane (C160 replay "
          "is in a different lane)",
          "REVIVE.C F0280:124-132");
    CHECK(state.f0282CandidateClearCount == 0,
          "F0282 candidate clear does NOT run in this lane on C160",
          "REVIVE.C F0282:744-806");
}

static void test_baseline_without_c040_returns_zero(void)
{
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 state;
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 result;
    int ok;

    dm1_v1_mirror_candidate_lower_arrow_state_init_pc34(&state);
    ok = dm1_v1_mirror_candidate_lower_arrow_state_dispatch_pc34(
        &state, 4 /* C004_COMMAND_MOVE_RIGHT */, &result);

    CHECK(ok == 0,
          "without a live C040 panel the lower-arrow state gate is not in scope",
          "PANEL.C F0346:1619-1637");
    CHECK(result.panelStayedC040 == 0,
          "C040 panel state is not declared stable when the gate is not in scope",
          "PANEL.C F0346:1619-1637");
    CHECK(state.c040PanelOpen == 0,
          "no-C040 baseline keeps c040PanelOpen = 0",
          "PANEL.C F0346:1619-1637");
}

static void test_three_lower_arrows_in_sequence(void)
{
    Dm1V1MirrorCandidateLowerArrowStateStatePc34 state;
    Dm1V1MirrorCandidateLowerArrowStateResultPc34 result;
    int ok;

    publish_live_candidate(&state);
    /* Three lower-arrow clicks in sequence: C004, C005, C006. */
    for (int i = 0; i < 3; ++i) {
        int command = 4 + i;
        ok = dm1_v1_mirror_candidate_lower_arrow_state_dispatch_pc34(
            &state, command, &result);
        CHECK(ok == 1, "lower-arrow sequence click is accepted by F0380",
              "COMMAND.C F0380:2151-2156");
        CHECK(result.panelStayedC040 == 1,
              "C040 panel stays open across lower-arrow sequence",
              "PANEL.C F0346:1619-1637");
        CHECK(result.candidateOrdinalPreserved == 1,
              "G0299 candidate ordinal is preserved across sequence",
              "REVIVE.C F0280:124-132");
    }
    CHECK(state.f0380DrainCount == 3,
          "F0380 drain count is exactly 3 after three lower-arrow clicks",
          "COMMAND.C F0380:2151-2156");
    CHECK(state.f0366MovePartyEnterCount == 3,
          "F0366 move-party entry count is exactly 3 after sequence",
          "CLIKMENU.C F0366:180-280");
    CHECK(state.c040PanelOpen == 1,
          "c040PanelOpen is still 1 after three lower-arrow clicks",
          "PANEL.C F0346:1619-1637");
    CHECK(state.f0282CandidateClearCount == 0,
          "F0282 candidate clear count is still 0 after sequence",
          "REVIVE.C F0282:744-806");
    CHECK(state.candidateOrdinal == 3,
          "candidate ordinal is still 3 after sequence",
          "REVIVE.C F0280:272-276");
}

int main(void)
{
    test_source_lock_metadata();
    test_lower_arrow_c004_keeps_c040_state();
    test_lower_arrow_c005_keeps_c040_state();
    test_lower_arrow_c006_keeps_c040_state();
    test_upper_arrow_c003_still_dispatches_without_c040_change();
    test_inventory_toggle_blocked_by_g0299();
    test_close_inventory_c011_path();
    test_panel_command_c160_path();
    test_baseline_without_c040_returns_zero();
    test_three_lower_arrows_in_sequence();

    printf("PASS dm1_v1_mirror_candidate_lower_arrow_state_pc34_compat "
           "%d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}
