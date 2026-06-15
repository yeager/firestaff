#include "dm1_v1_champion_panel_pressing_mouth_eye_statusbox_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_CPMESB_ZONE_CHAMPION_0_STATUS_BOX_PC34 = 151,
    DM1_V1_CPMESB_ZONE_CHAMPION_3_STATUS_BOX_PC34 = 154,
    DM1_V1_CPMESB_ZONE_POISONED_PC34 = 502,
    DM1_V1_CPMESB_PANEL_ROUTE_FOOD_WATER_POISON_PC34 = 345,
    DM1_V1_CPMESB_PANEL_ROUTE_SKILLS_STATISTICS_PC34 = 351,
    DM1_V1_CPMESB_PANEL_ROUTE_OBJECT_DESCRIPTION_PC34 = 342
};

/*
 * ReDMCSB anchors for this narrow byte-plan:
 * - CHAMDRAW.C F0291_CHAMPION_DrawSlot:498-677 slot render path.
 * - CHAMDRAW.C F0292_CHAMPION_DrawState:771-789 C151..C154 live fill and
 *   1060-1087 G0333/G0331 panel/action-hand redraw route.
 * - CHAMPION.C F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:677-711
 *   status-hand box object swap and F0292 redraw handoff.
 * - CHAMPION.C F0284_CHAMPION_SetPartyDirection:117-130 leader-rotation
 *   cascade before changed-icon redraw.
 * - TEXT.C F0041:1254-1264 requested text primitive anchor; PANEL.C
 *   F0345:1601-1606 is the actual C032 poisoned-banner overlay blit.
 * - DEFS.H:2193-2195 C033/C034/C035 slot boxes and 3783-3786 C151..C154.
 */
static const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPc34Contract
    s_contract = {
        1,
        DM1_V1_CPMESB_CHAMPION_COUNT_PC34,
        DM1_V1_CPMESB_ZONE_CHAMPION_0_STATUS_BOX_PC34,
        DM1_V1_CPMESB_ZONE_CHAMPION_3_STATUS_BOX_PC34,
        DM1_V1_CPMESB_STATUS_BOX_WIDTH_PC34,
        DM1_V1_CPMESB_STATUS_BOX_HEIGHT_PC34,
        DM1_STATUS_BOX_SPACING,
        DM1_COLOR_DARKEST_GRAY,
        DM1_SLOT_READY_HAND,
        DM1_SLOT_ACTION_HAND,
        DM1_SLOT_BOX_SIZE,
        DM1_GFX_SLOT_NORMAL,
        DM1_GFX_SLOT_WOUNDED,
        DM1_GFX_SLOT_ACTING,
        DM1_GFX_POISONED_LABEL,
        DM1_V1_CPMESB_ZONE_POISONED_PC34,
        DM1_V1_CPMESB_PRESS_MOUTH_PC34,
        DM1_V1_CPMESB_PRESS_EYE_PC34,
        "CHAMDRAW.C F0291_CHAMPION_DrawSlot:498-677",
        "CHAMDRAW.C F0292_CHAMPION_DrawState:771-789,1060-1087",
        "CHAMPION.C F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:677-711",
        "CHAMPION.C F0284_CHAMPION_SetPartyDirection:117-130",
        "TEXT.C F0041:1254-1264 plus PANEL.C F0345:1601-1606 poison label",
        "DEFS.H C033-C035:2193-2195; C151-C154:3783-3786"
    };

static const char s_source_evidence[] =
    "contract_only=1; CHAMDRAW.C F0291:498-677 selects status-hand slot "
    "boxes and C033/C034/C035 graphics; CHAMDRAW.C F0292:771-789 fills live "
    "C151..C154 status boxes with C12 and F0292:1060-1087 preserves the "
    "G0333 mouth/G0331 eye panel route before action-hand redraw. "
    "CHAMPION.C F0302:677-711 routes status-hand slot boxes through the "
    "leader-hand/object swap and calls F0292; CHAMPION.C F0284:117-130 "
    "rotates party champion cells/directions before changed icons redraw, "
    "without changing C151..C154 geometry. TEXT.C F0041:1254-1264 is the "
    "trailing-space print primitive requested by the source lock; the actual "
    "poisoned banner overlay is PANEL.C F0345:1601-1606, which blits "
    "C032_GRAPHIC_POISONED_LABEL to C502_ZONE_POISONED. DEFS.H:2193-2195 "
    "defines C033/C034/C035 and DEFS.H:3783-3786 defines C151..C154.";

const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPc34Contract *
dm1_v1_champion_panel_pressing_mouth_eye_statusbox_contract_pc34(void)
{
    return &s_contract;
}

const char *
dm1_v1_champion_panel_pressing_mouth_eye_statusbox_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static int valid_champion_index(int champion_index)
{
    return champion_index >= 0 &&
           champion_index < DM1_V1_CPMESB_CHAMPION_COUNT_PC34;
}

static int valid_press(
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPressPc34 press)
{
    return press == DM1_V1_CPMESB_PRESS_MOUTH_PC34 ||
           press == DM1_V1_CPMESB_PRESS_EYE_PC34;
}

static void paint_rect(DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34 *frame,
                       int x,
                       int y,
                       int width,
                       int height,
                       int value)
{
    int row;
    int col;

    for (row = y; row < y + height; ++row) {
        for (col = x; col < x + width; ++col) {
            frame->bytes[row * DM1_V1_CPMESB_STATUS_BOX_WIDTH_PC34 + col] =
                (uint8_t)value;
        }
    }
}

static void build_frame(
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34 *frame,
    int tick_index,
    int champion_index,
    int active_champion_index,
    int acting_champion_index)
{
    DM1_ChampionPanel_StatusHandSlotBoxModel ready;
    DM1_ChampionPanel_StatusHandSlotBoxModel action;
    int is_active;

    memset(frame, 0, sizeof(*frame));
    frame->tick_index = tick_index;
    frame->champion_index = champion_index;
    frame->zone_id =
        DM1_V1_CPMESB_ZONE_CHAMPION_0_STATUS_BOX_PC34 + champion_index;
    frame->screen_x = champion_index * DM1_STATUS_BOX_SPACING;
    frame->screen_y = 0;
    frame->width = DM1_V1_CPMESB_STATUS_BOX_WIDTH_PC34;
    frame->height = DM1_V1_CPMESB_STATUS_BOX_HEIGHT_PC34;
    frame->status_box_fill_color = DM1_COLOR_DARKEST_GRAY;
    frame->ready_hand_relative_x = 4;
    frame->action_hand_relative_x = 24;
    frame->hand_relative_y = 10;
    frame->ready_hand_graphic = -1;
    frame->action_hand_graphic = -1;

    is_active = champion_index == active_champion_index;
    if (!is_active) {
        return;
    }

    /*
     * ReDMCSB CHAMDRAW.C F0292:771-789: C151..C154 live status-box
     * redraw fills exactly x=(champion*69)..+66 and y=0..28 with
     * C12_COLOR_DARKEST_GRAY before any borders or hand slots.
     */
    frame->redraws_status_box = 1;
    frame->status_box_fill_pixel_count =
        DM1_V1_CPMESB_STATUS_BOX_BYTE_COUNT_PC34;
    paint_rect(frame,
               0,
               0,
               DM1_V1_CPMESB_STATUS_BOX_WIDTH_PC34,
               DM1_V1_CPMESB_STATUS_BOX_HEIGHT_PC34,
               DM1_COLOR_DARKEST_GRAY);

    /*
     * ReDMCSB CHAMDRAW.C F0291:648-651: only the action hand of the acting
     * champion gets C035. Ready hand remains C033; wounds are intentionally
     * outside this non-wound slice.
     */
    (void)DM1_ChampionPanel_BuildStatusHandSlotBoxModel(
        champion_index,
        DM1_SLOT_READY_HAND,
        champion_index == acting_champion_index,
        &ready);
    (void)DM1_ChampionPanel_BuildStatusHandSlotBoxModel(
        champion_index,
        DM1_SLOT_ACTION_HAND,
        champion_index == acting_champion_index,
        &action);
    frame->ready_hand_relative_x = ready.x - frame->screen_x;
    frame->action_hand_relative_x = action.x - frame->screen_x;
    frame->hand_relative_y = ready.y;
    frame->ready_hand_graphic = ready.graphicId;
    frame->action_hand_graphic = action.graphicId;
    frame->slot_box_pixel_count = DM1_SLOT_BOX_SIZE * DM1_SLOT_BOX_SIZE * 2;
    paint_rect(frame,
               frame->ready_hand_relative_x,
               frame->hand_relative_y,
               DM1_SLOT_BOX_SIZE,
               DM1_SLOT_BOX_SIZE,
               frame->ready_hand_graphic);
    paint_rect(frame,
               frame->action_hand_relative_x,
               frame->hand_relative_y,
               DM1_SLOT_BOX_SIZE,
               DM1_SLOT_BOX_SIZE,
               frame->action_hand_graphic);
}

static int changed_bytes(
    const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34 *pre,
    const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxFramePc34 *post)
{
    int index;
    int changed;

    changed = 0;
    for (index = 0; index < DM1_V1_CPMESB_STATUS_BOX_BYTE_COUNT_PC34;
         ++index) {
        if (pre->bytes[index] != post->bytes[index]) {
            ++changed;
        }
    }
    return changed;
}

int dm1_v1_champion_panel_pressing_mouth_eye_statusbox_plan_pc34(
    const DM1_V1_ChampionPanelPressingMouthEyeStatusBoxInputPc34 *input,
    DM1_V1_ChampionPanelPressingMouthEyeStatusBoxPlanPc34 *out_plan)
{
    int champion_index;
    int changed;

    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!input ||
        !valid_champion_index(input->active_champion_index) ||
        !valid_champion_index(input->inventory_champion_index) ||
        !valid_champion_index(input->acting_champion_index) ||
        (input->poison_banner_active &&
         !valid_champion_index(input->poison_banner_champion_index))) {
        out_plan->rejected_invalid_champion = 1;
        return 0;
    }
    if (!valid_press(input->press)) {
        out_plan->rejected_invalid_press = 1;
        return 0;
    }
    if (input->poison_banner_active &&
        input->poison_banner_champion_index == input->active_champion_index) {
        out_plan->rejected_same_poison_champion = 1;
        return 0;
    }

    out_plan->valid = 1;
    out_plan->active_champion_index = input->active_champion_index;
    out_plan->inventory_champion_index = input->inventory_champion_index;
    out_plan->acting_champion_index = input->acting_champion_index;
    out_plan->pressing_eye = input->press == DM1_V1_CPMESB_PRESS_EYE_PC34;
    out_plan->pressing_mouth =
        input->press == DM1_V1_CPMESB_PRESS_MOUTH_PC34;
    out_plan->g0331_presses_eye = out_plan->pressing_eye;
    out_plan->g0333_presses_mouth = out_plan->pressing_mouth;
    out_plan->panel_redraw_route = out_plan->pressing_mouth
        ? DM1_V1_CPMESB_PANEL_ROUTE_FOOD_WATER_POISON_PC34
        : (input->leader_empty_handed
               ? DM1_V1_CPMESB_PANEL_ROUTE_SKILLS_STATISTICS_PC34
               : DM1_V1_CPMESB_PANEL_ROUTE_OBJECT_DESCRIPTION_PC34);
    out_plan->viewport_redraw_requested = 1;
    out_plan->status_box_redraw_requested = 1;
    out_plan->poison_banner_active = input->poison_banner_active ? 1 : 0;
    out_plan->poison_banner_champion_index =
        input->poison_banner_active ? input->poison_banner_champion_index : -1;
    out_plan->poison_overlay_zone = DM1_V1_CPMESB_ZONE_POISONED_PC34;
    out_plan->poison_overlay_graphic = DM1_GFX_POISONED_LABEL;
    out_plan->poison_overlay_on_different_champion =
        input->poison_banner_active ? 1 : 0;
    /*
     * ReDMCSB PANEL.C F0345:1601-1606 blits C032 to C502 in the inventory
     * panel; TEXT.C F0041:1254-1264 is the requested text primitive anchor.
     * Neither target intersects C151..C154 top status-box memory.
     */
    out_plan->poison_overlay_touches_status_box_bytes = 0;

    for (champion_index = 0;
         champion_index < DM1_V1_CPMESB_CHAMPION_COUNT_PC34;
         ++champion_index) {
        build_frame(&out_plan->pre_tick[champion_index],
                    0,
                    champion_index,
                    input->active_champion_index,
                    input->acting_champion_index);
        build_frame(&out_plan->post_tick[champion_index],
                    1,
                    champion_index,
                    input->active_champion_index,
                    input->acting_champion_index);
    }

    /*
     * ReDMCSB CHAMDRAW.C F0292:1060-1078 keeps the G0333/G0331 panel route
     * active across the redraw; this one-tick slice must not mutate C151..C154
     * status-box bytes when the press is still held.
     */
    changed = 0;
    for (champion_index = 0;
         champion_index < DM1_V1_CPMESB_CHAMPION_COUNT_PC34;
         ++champion_index) {
        changed += changed_bytes(&out_plan->pre_tick[champion_index],
                                 &out_plan->post_tick[champion_index]);
    }
    out_plan->post_tick_changed_byte_count = changed;
    return 1;
}
