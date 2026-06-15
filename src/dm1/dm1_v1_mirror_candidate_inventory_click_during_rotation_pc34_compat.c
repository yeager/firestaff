#include "dm1_v1_mirror_candidate_inventory_click_during_rotation_pc34_compat.h"

/*
 * Contract-only DM1 V1 mirror-candidate inventory-click-during-rotation gate.
 *
 * ReDMCSB CHAMPION.C F0284:93-131 updates party direction for C156/C157 turn
 * commands and redraws changed object icons after the direction change.
 * ReDMCSB COMMAND.C F0359:1985-1990 keeps live M568/C040 mirror-panel clicks
 * in the panel command dispatch path before resurrect/reincarnate handling.
 * The local ReDMCSB checkout maps the requested mouse queue anchor through
 * INPUT.C:641-664 for left/right button down events entering F0359 and
 * IO.C F0078/F0077:1102-1122 for mouse-update suppression while blitting.
 * ReDMCSB CHAMDRAW.C F0293:1117-1143 redraws all champion states, and
 * PANEL.C F0354:2208-2240 is the portrait box/blit route that must not be
 * reached with a stale in-progress rotation portrait.
 */

static const Dm1V1MirrorCandidateInventoryClickDuringRotationEvidencePc34Compat
    s_evidence = {
        1,
        "CHAMPION.C F0284:93-131 C156/C157 SetPartyDirection update",
        "COMMAND.C F0359:1985-1990 M568/C040 panel dispatch",
        "INPUT.C:641-664 button events call F0359; IO.C F0078/F0077:"
        "1102-1122 guards mouse screen updates",
        "CHAMDRAW.C F0293:1117-1143 champion-state redraw loop",
        "PANEL.C F0354:2208-2240 inventory/status portrait box blit",
        "contract_only=1 synthetic dispatch gate; no framebuffer, SDL, "
        "real-asset, or V2 presentation parity is claimed"
    };

static const Dm1V1MirrorCandidateInventoryClickDuringRotationCasePc34Compat
    dm1_v1_mirror_candidate_rotation_click_table[] = {
        {
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C156_TURN_LEFT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_LEFT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_SUPPRESS_DURING_ROTATION_PC34_COMPAT,
            "left-click portrait during in-progress C156 rotation",
            "CHAMPION.C F0284:93-131; PANEL.C F0354:2208-2240"
        },
        {
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C157_TURN_RIGHT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_RIGHT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_QUEUE_FOR_AFTER_ROTATION_PC34_COMPAT,
            "right-click portrait during in-progress C157 rotation",
            "INPUT.C:663-664; PANEL.C F0354:2208-2240"
        },
        {
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C156_TURN_LEFT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_LEFT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_ACTION_HAND_SLOT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT,
            "left-click action-hand slot during in-progress C156 rotation",
            "COMMAND.C F0359:1985-1990"
        },
        {
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C157_TURN_RIGHT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_RIGHT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_ACTION_HAND_SLOT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT,
            "right-click action-hand slot during in-progress C157 rotation",
            "INPUT.C:663-664; COMMAND.C F0359:1985-1990"
        },
        {
            DM1_V1_MIRROR_CANDIDATE_ROTATION_PAUSED_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_LEFT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT,
            "click during paused rotation",
            "CHAMDRAW.C F0293:1117-1143"
        },
        {
            DM1_V1_MIRROR_CANDIDATE_ROTATION_COMPLETE_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_RIGHT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT,
            "click after rotation completes",
            "CHAMPION.C F0284:129-130"
        },
        {
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C156_TURN_LEFT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_LEFT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_CLOSED_INVENTORY_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT,
            "click on closed inventory during rotation",
            "COMMAND.C F0359:1985-1990"
        },
        {
            DM1_V1_MIRROR_CANDIDATE_ROTATION_C157_TURN_RIGHT_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_MIDDLE_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_OPEN_INVENTORY_PC34_COMPAT,
            DM1_V1_MIRROR_CANDIDATE_QUEUE_FOR_AFTER_ROTATION_PC34_COMPAT,
            "click on open inventory during rotation",
            "PANEL.C F0354:2208-2240; CHAMDRAW.C F0293:1117-1143"
        }
    };

static int is_rotation_in_progress(int rotationInProgress)
{
    return rotationInProgress ==
               DM1_V1_MIRROR_CANDIDATE_ROTATION_C156_TURN_LEFT_PC34_COMPAT ||
           rotationInProgress ==
               DM1_V1_MIRROR_CANDIDATE_ROTATION_C157_TURN_RIGHT_PC34_COMPAT;
}

static int is_valid_click_type(int clickType)
{
    return clickType >= DM1_V1_MIRROR_CANDIDATE_CLICK_LEFT_PC34_COMPAT &&
           clickType <= DM1_V1_MIRROR_CANDIDATE_CLICK_MIDDLE_PC34_COMPAT;
}

Dm1V1MirrorCandidateInventoryClickDecisionPc34Compat
dm1_v1_mirror_candidate_inventory_click_during_rotation_decide(
    int rotationInProgress,
    int clickType,
    int clickTarget)
{
    if (!is_valid_click_type(clickType)) {
        return DM1_V1_MIRROR_CANDIDATE_REJECT_INVALID_CLICK_TYPE_PC34_COMPAT;
    }
    if (!is_rotation_in_progress(rotationInProgress)) {
        return DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT;
    }
    if (clickTarget ==
        DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_ACTION_HAND_SLOT_PC34_COMPAT) {
        return DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT;
    }
    if (clickTarget ==
        DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_CLOSED_INVENTORY_PC34_COMPAT) {
        return DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT;
    }
    if (clickTarget ==
        DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT &&
        clickType == DM1_V1_MIRROR_CANDIDATE_CLICK_LEFT_PC34_COMPAT) {
        return DM1_V1_MIRROR_CANDIDATE_SUPPRESS_DURING_ROTATION_PC34_COMPAT;
    }
    if (clickTarget ==
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_PORTRAIT_PC34_COMPAT ||
        clickTarget ==
            DM1_V1_MIRROR_CANDIDATE_CLICK_TARGET_OPEN_INVENTORY_PC34_COMPAT) {
        return DM1_V1_MIRROR_CANDIDATE_QUEUE_FOR_AFTER_ROTATION_PC34_COMPAT;
    }
    return DM1_V1_MIRROR_CANDIDATE_DISPATCH_NORMALLY_PC34_COMPAT;
}

const Dm1V1MirrorCandidateInventoryClickDuringRotationCasePc34Compat *
dm1_v1_mirror_candidate_inventory_click_during_rotation_table(
    size_t *outCount)
{
    if (outCount) {
        *outCount = sizeof(dm1_v1_mirror_candidate_rotation_click_table) /
                    sizeof(dm1_v1_mirror_candidate_rotation_click_table[0]);
    }
    return dm1_v1_mirror_candidate_rotation_click_table;
}

int dm1_v1_mirror_candidate_inventory_click_during_rotation_case_at(
    size_t index,
    const Dm1V1MirrorCandidateInventoryClickDuringRotationCasePc34Compat
        **outCase)
{
    size_t count = 0u;

    if (!outCase) {
        return 0;
    }
    dm1_v1_mirror_candidate_inventory_click_during_rotation_table(&count);
    if (index >= count) {
        *outCase = 0;
        return 0;
    }
    *outCase = &dm1_v1_mirror_candidate_rotation_click_table[index];
    return 1;
}

const Dm1V1MirrorCandidateInventoryClickDuringRotationEvidencePc34Compat *
dm1_v1_mirror_candidate_inventory_click_during_rotation_evidence(void)
{
    return &s_evidence;
}
