#include "dm1_v1_champion_panel_mouth_eye_release_pc34_compat.h"

#include <string.h>

static const DM1_V1_ChampionPanelMouthEyeReleaseEvidencePc34Compat s_evidence = {
    true,
    "PANEL.C F0349_INVENTORY_ProcessCommand70_ClickOnMouth:1788-1818",
    "PANEL.C F0350_INVENTORY_DrawStopPressingMouth:1953-1963",
    "PANEL.C F0352_INVENTORY_ProcessCommand71_ClickOnEye:2111-2160",
    "PANEL.C F0353_INVENTORY_DrawStopPressingEye:2162-2193",
    "CHAMPION.C F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:677-711",
    "CHEST.C F0333:30-38 and F0334:79-132",
    "COMMAND.C F0359/F0380:1982-1990,2129-2140,2315-2319",
    "DEFS.H:1953-1956 C202/C203/C205 icon ordinals",
    "DEFS.H:3914-3915 C545/C546 mouth/eye zones",
    "DEFS.H:2564,6956 and PANEL.C:1817,1960,2159,2181 viewport redraw mode",
    "contract-only PC34 mouth/eye press-release route; no bitmap sampling",
    "without claiming real-asset parity"
};

static const char s_source_evidence[] =
    "contract_only=1; PANEL.C F0349:1788-1818 empty-hand mouth press sets "
    "G0597/G0333, hides the pointer, sets G0587=1, draws F0345, then calls "
    "F0097(C0_VIEWPORT_NOT_DUNGEON_VIEW); F0350:1959-1962 redraws F0347, "
    "F0097(0), sets G0587=1, and shows the pointer. PANEL.C F0352:2123-2159 "
    "sets G0597/G0331, discards input, hides pointer, delays 8, draws "
    "C203 in C546, dispatches to F0351 or F0342, then calls F0097(0). "
    "PANEL.C F0353:2174-2192 draws C202 in C546, redraws F0347 and F0097(0), "
    "clears four skill-upgraded flags when the hand is empty or redraws the "
    "leader hand object name, then shows the pointer. CHAMPION.C F0302:677-711 "
    "owns live slot/leader-hand swaps separately from mouth/eye panel routes; "
    "CHEST.C F0333:30-38 and F0334:79-132 own open-chest preservation/close; "
    "COMMAND.C F0359/F0380:1982-1990,2129-2140,2315-2319 routes queued "
    "commands without treating a pending hand item as G4055. DEFS.H anchors "
    "C202/C203/C205, C545/C546, and C0_VIEWPORT_NOT_DUNGEON_VIEW; without "
    "claiming real-asset parity.";

const DM1_V1_ChampionPanelMouthEyeReleaseEvidencePc34Compat *
DM1_V1_ChampionPanelMouthEyeRelease_EvidencePc34Compat(void)
{
    return &s_evidence;
}

const char *
DM1_V1_ChampionPanelMouthEyeRelease_SourceEvidencePc34Compat(void)
{
    return s_source_evidence;
}

void DM1_V1_ChampionPanelMouthEyeRelease_DefaultInputPc34Compat(
    DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat *input)
{
    if (!input) {
        return;
    }
    memset(input, 0, sizeof(*input));
    input->action = DM1_V1_CPMER_ACTION_EYE_RELEASE_PC34;
    input->leader_empty_handed = true;
    input->left_button_down = true;
    input->inventory_champion_ordinal = DM1_V1_CPMER_INVENTORY_ORDINAL_FIRST_PC34;
    input->leader_hand_thing_before = -1;
    input->pending_hand_thing_before = -1;
}

static void append_op(DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat *result,
                      DM1_V1_ChampionPanelMouthEyeReleaseOpPc34Compat op)
{
    if (result->operation_count < DM1_V1_CPMER_OPERATION_CAPACITY_PC34) {
        result->operations[result->operation_count++] = op;
    }
}

static bool valid_action(DM1_V1_ChampionPanelMouthEyeReleaseActionPc34Compat action)
{
    return action >= DM1_V1_CPMER_ACTION_MOUTH_PRESS_PC34 &&
           action <= DM1_V1_CPMER_ACTION_EYE_RELEASE_PC34;
}

static bool valid_inventory_ordinal(int ordinal)
{
    return ordinal >= DM1_V1_CPMER_INVENTORY_ORDINAL_NONE_PC34 &&
           ordinal <= DM1_V1_CPMER_INVENTORY_ORDINAL_LAST_PC34;
}

static void draw_viewport(
    DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat *result)
{
    result->viewport_mode = DM1_V1_CPMER_VIEWPORT_NOT_DUNGEON_VIEW_PC34;
    result->viewport_draw_count++;
    append_op(result, DM1_V1_CPMER_OP_DRAW_VIEWPORT_PC34);
}

static void build_mouth_press(
    const DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat *input,
    DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat *result)
{
    /*
     * ReDMCSB PANEL.C F0349:1788-1818: this contract covers the
     * empty-leader-hand path that opens the food/water/poison mouth panel.
     */
    if (!input->leader_empty_handed ||
        input->panel_already_food_water_poisoned) {
        result->early_return = true;
        return;
    }

    result->ignore_mouse_movements = true;
    result->pressing_mouth = true;
    append_op(result, DM1_V1_CPMER_OP_SET_IGNORE_MOUSE_PC34);
    append_op(result, DM1_V1_CPMER_OP_SET_PRESSING_MOUTH_PC34);

    if (!input->left_button_down) {
        result->ignore_mouse_movements = false;
        result->pressing_mouth = false;
        result->early_return = true;
        append_op(result, DM1_V1_CPMER_OP_RESET_IGNORE_MOUSE_PC34);
        append_op(result, DM1_V1_CPMER_OP_RESET_PRESSING_MOUTH_PC34);
        return;
    }

    result->pointer_hidden = true;
    result->hide_mouse_pointer_request_count = 1;
    result->panel_route = DM1_V1_CPMER_PANEL_ROUTE_FOOD_WATER_POISON_PC34;
    append_op(result, DM1_V1_CPMER_OP_HIDE_POINTER_PC34);
    append_op(result, DM1_V1_CPMER_OP_SET_HIDE_REQUEST_PC34);
    append_op(result, DM1_V1_CPMER_OP_DRAW_FOOD_WATER_POISON_PANEL_PC34);
    draw_viewport(result);
}

static void build_mouth_release(
    DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat *result)
{
    /*
     * ReDMCSB PANEL.C F0350:1959-1962: releasing mouth restores the
     * inventory panel, redraws the viewport, resets the hide count, then
     * shows the mouse pointer.
     */
    result->panel_route = DM1_V1_CPMER_PANEL_ROUTE_INVENTORY_PANEL_PC34;
    append_op(result, DM1_V1_CPMER_OP_DRAW_INVENTORY_PANEL_PC34);
    draw_viewport(result);
    result->hide_mouse_pointer_request_count = 1;
    result->pointer_shown = true;
    append_op(result, DM1_V1_CPMER_OP_SET_HIDE_REQUEST_PC34);
    append_op(result, DM1_V1_CPMER_OP_SHOW_POINTER_PC34);
}

static void build_eye_press(
    const DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat *input,
    DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat *result)
{
    /*
     * ReDMCSB PANEL.C F0352:2123-2159: eye press keeps the cursor hidden,
     * swaps the C546 eye icon to C203, and then dispatches the panel by
     * leader-hand state.
     */
    result->ignore_mouse_movements = true;
    result->pressing_eye = true;
    append_op(result, DM1_V1_CPMER_OP_SET_IGNORE_MOUSE_PC34);
    append_op(result, DM1_V1_CPMER_OP_SET_PRESSING_EYE_PC34);

    if (!input->left_button_down) {
        result->ignore_mouse_movements = false;
        result->pressing_eye = false;
        result->early_return = true;
        append_op(result, DM1_V1_CPMER_OP_RESET_IGNORE_MOUSE_PC34);
        append_op(result, DM1_V1_CPMER_OP_RESET_PRESSING_EYE_PC34);
        return;
    }

    append_op(result, DM1_V1_CPMER_OP_DISCARD_INPUT_PC34);
    result->pointer_hidden = true;
    append_op(result, DM1_V1_CPMER_OP_HIDE_POINTER_PC34);
    result->delay_ticks = DM1_V1_CPMER_DELAY_TICKS_PC34;
    append_op(result, DM1_V1_CPMER_OP_DELAY_PC34);
    result->icon_index = DM1_V1_CPMER_ICON_EYE_LOOKING_PC34;
    result->icon_zone = DM1_V1_CPMER_ZONE_EYE_PC34;
    append_op(result, DM1_V1_CPMER_OP_DRAW_EYE_LOOKING_PC34);

    if (input->leader_empty_handed) {
        result->panel_route = DM1_V1_CPMER_PANEL_ROUTE_SKILLS_STATISTICS_PC34;
        append_op(result, DM1_V1_CPMER_OP_DRAW_SKILLS_STATISTICS_PC34);
    } else {
        result->object_name_cleared = true;
        result->object_panel_inspect = true;
        result->panel_route = DM1_V1_CPMER_PANEL_ROUTE_OBJECT_DESCRIPTION_PC34;
        append_op(result, DM1_V1_CPMER_OP_CLEAR_LEADER_HAND_NAME_PC34);
        append_op(result, DM1_V1_CPMER_OP_DRAW_OBJECT_PANEL_PC34);
    }
    draw_viewport(result);
}

static void build_eye_release(
    const DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat *input,
    DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat *result)
{
    /*
     * ReDMCSB PANEL.C F0353:2174-2192: eye release restores C202 in C546,
     * redraws the regular inventory panel and viewport, then either clears
     * four recently-upgraded skill flags or redraws the leader-hand name.
     */
    result->icon_index = DM1_V1_CPMER_ICON_EYE_NOT_LOOKING_PC34;
    result->icon_zone = DM1_V1_CPMER_ZONE_EYE_PC34;
    append_op(result, DM1_V1_CPMER_OP_DRAW_EYE_NOT_LOOKING_PC34);
    result->panel_route = DM1_V1_CPMER_PANEL_ROUTE_INVENTORY_PANEL_PC34;
    append_op(result, DM1_V1_CPMER_OP_DRAW_INVENTORY_PANEL_PC34);
    draw_viewport(result);

    if (input->leader_hand_has_object) {
        result->object_name_drawn = true;
        append_op(result, DM1_V1_CPMER_OP_DRAW_LEADER_HAND_NAME_PC34);
    } else {
        result->skill_recently_upgraded_clear_count =
            DM1_V1_CPMER_SKILL_GROUP_COUNT_PC34;
        append_op(result, DM1_V1_CPMER_OP_CLEAR_SKILL_RECENTLY_UPGRADED_PC34);
    }
    result->pointer_shown = true;
    append_op(result, DM1_V1_CPMER_OP_SHOW_POINTER_PC34);
}

int DM1_V1_ChampionPanelMouthEyeRelease_BuildPc34Compat(
    const DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat *input,
    DM1_V1_ChampionPanelMouthEyeReleaseResultPc34Compat *out_result)
{
    DM1_V1_ChampionPanelMouthEyeReleaseInputPc34Compat local_input;

    if (!out_result) {
        return 0;
    }

    memset(out_result, 0, sizeof(*out_result));
    out_result->contract_only = true;
    out_result->icon_index = -1;
    out_result->icon_zone = -1;
    out_result->viewport_mode = -1;
    out_result->evidence = &s_evidence;

    if (!input) {
        DM1_V1_ChampionPanelMouthEyeRelease_DefaultInputPc34Compat(&local_input);
        input = &local_input;
    }

    if (!valid_action(input->action)) {
        out_result->rejected_action = true;
        return 0;
    }
    if (!valid_inventory_ordinal(input->inventory_champion_ordinal)) {
        out_result->rejected_inventory_ordinal = true;
        return 0;
    }

    out_result->leader_hand_thing_before = input->leader_hand_thing_before;
    out_result->leader_hand_thing_after = input->leader_hand_thing_before;
    out_result->pending_hand_queue_count_before =
        input->pending_hand_queue_count;
    out_result->pending_hand_queue_count_after =
        input->pending_hand_queue_count;
    out_result->pending_hand_thing_before = input->pending_hand_thing_before;
    out_result->pending_hand_thing_after = input->pending_hand_thing_before;

    out_result->valid = true;
    switch (input->action) {
    case DM1_V1_CPMER_ACTION_MOUTH_PRESS_PC34:
        build_mouth_press(input, out_result);
        break;
    case DM1_V1_CPMER_ACTION_MOUTH_RELEASE_PC34:
        build_mouth_release(out_result);
        break;
    case DM1_V1_CPMER_ACTION_EYE_PRESS_PC34:
        build_eye_press(input, out_result);
        break;
    case DM1_V1_CPMER_ACTION_EYE_RELEASE_PC34:
        build_eye_release(input, out_result);
        break;
    default:
        out_result->rejected_action = true;
        out_result->valid = false;
        return 0;
    }

    /*
     * ReDMCSB PANEL.C F0349/F0352 route only the live G4055 leader hand.
     * COMMAND.C F0380 pending-click replay and CHAMPION.C F0302 slot swaps
     * remain separate, so pending hand items must not be consumed or used as
     * stale object-panel content by mouth/eye redraws.
     */
    out_result->leader_hand_consumed =
        out_result->leader_hand_thing_before != out_result->leader_hand_thing_after;
    out_result->pending_hand_consumed =
        out_result->pending_hand_queue_count_before !=
            out_result->pending_hand_queue_count_after ||
        out_result->pending_hand_thing_before != out_result->pending_hand_thing_after;
    out_result->pending_queue_preserved = !out_result->pending_hand_consumed;
    out_result->stale_panel_after =
        input->pending_hand_queue_count > 0 &&
        input->leader_empty_handed &&
        out_result->panel_route ==
            DM1_V1_CPMER_PANEL_ROUTE_OBJECT_DESCRIPTION_PC34;

    return 1;
}
