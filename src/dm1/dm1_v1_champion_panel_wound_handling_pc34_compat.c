#include "dm1_v1_champion_panel_wound_handling_pc34_compat.h"

#include <string.h>

int DM1_ChampionPanel_BuildWoundHandSlotBoxModel(
    int championIndex, int handIndex, int woundLevel, int isActingChampion,
    DM1_ChampionPanel_WoundHandSlotBoxModel *outModel)
{
    int isActionHand;
    int isWoundAffected;

    if (!outModel ||
        championIndex < 0 ||
        championIndex >=
            DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_CHAMPION_COUNT_PC34 ||
        handIndex < 0 ||
        handIndex >= DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_HAND_COUNT_PC34 ||
        woundLevel < 0) {
        return 0;
    }

    isActionHand =
        handIndex == DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_ACTION_HAND_PC34;
    isWoundAffected = woundLevel > 0;

    memset(outModel, 0, sizeof(*outModel));
    outModel->championIndex = championIndex;
    outModel->handIndex = handIndex;
    outModel->bodySlotIndex = handIndex;
    outModel->slotBoxIndex =
        (championIndex *
         DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_HAND_COUNT_PC34) +
        handIndex;
    outModel->zoneId =
        DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_FIRST_HAND_ZONE_PC34 +
        outModel->slotBoxIndex;
    outModel->woundLevel = woundLevel;
    outModel->isWoundAffected = isWoundAffected ? 1 : 0;
    outModel->isActingChampion = isActingChampion ? 1 : 0;
    outModel->isActionHand = isActionHand ? 1 : 0;
    outModel->x =
        (championIndex *
         DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_STATUS_BOX_SPACING_PC34) +
        (isActionHand ? 24 : 4);
    outModel->y = 10;
    outModel->width =
        DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_SLOT_BOX_SIZE_PC34;
    outModel->height =
        DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_SLOT_BOX_SIZE_PC34;

    /*
     * ReDMCSB CHAMDRAW.C F0291:632-646 limits this selector to body slots
     * C00..C05; ready/action hands are C00/C01. F0291:638-641 returns
     * C034_GRAPHIC_SLOT_BOX_WOUNDED when M007_GET(Wounds, 1 << slotIndex)
     * is true, otherwise C033. F0291:634-635 and 648-651 give the acting
     * action hand C035 priority over that wound branch.
     */
    if (isActionHand && isActingChampion) {
        outModel->branch =
            DM1_CHAMPION_PANEL_WOUND_HAND_SLOT_BOX_BRANCH_ACTING_PC34;
        outModel->graphicId =
            DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_ACTING_PC34;
        outModel->woundBranchSuppressedByActing =
            isWoundAffected ? 1 : 0;
    } else if (isWoundAffected) {
        outModel->branch =
            DM1_CHAMPION_PANEL_WOUND_HAND_SLOT_BOX_BRANCH_WOUNDED_PC34;
        outModel->graphicId =
            DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_WOUNDED_PC34;
    } else {
        outModel->branch =
            DM1_CHAMPION_PANEL_WOUND_HAND_SLOT_BOX_BRANCH_NORMAL_PC34;
        outModel->graphicId =
            DM1_V1_CHAMPION_PANEL_WOUND_HANDLING_GFX_SLOT_NORMAL_PC34;
    }

    /*
     * ReDMCSB CHAMDRAW.C F0296:1226-1231 walks status hand slot boxes
     * 0..(partyCount<<1)-1 and maps them back to ready/action hands with
     * DEFS.H M070_HAND_SLOT_INDEX. F0292's wound cadence at CHAMDRAW.C
     * 937-955 calls F0291 for action then ready hand, so this source-lock
     * gate keeps the same C033/C034/C035 decision visible as a contract.
     */
    return 1;
}
