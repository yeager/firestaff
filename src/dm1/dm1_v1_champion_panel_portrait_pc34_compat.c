#include "dm1/dm1_v1_champion_panel_portrait_pc34_compat.h"

#include <string.h>

static const DM1_V1_ChampionPanelPortraitEvidencePc34Compat s_evidence = {
    true,
    "CHAMDRAW.C F0292_CHAMPION_DrawState:810-812 inventory champion calls "
    "F0354 and keeps only MASK0x0100_STATISTICS dirty",
    "PANEL.C F0354_INVENTORY_DrawStatusBoxPortrait:2208-2213 old-media "
    "portrait box; 2226-2232 PC34 zone/portrait blit; 2237-2240 "
    "invisibility hatch",
    "DEFS.H:2076 CM1_COLOR_NO_TRANSPARENCY; 2471 C016_BYTE_WIDTH; "
    "3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX",
    "COORD.C:1713 G2071_C320_ScreenPixelWidth; 1748-1749 "
    "G2078_C32/G2079_C29 portrait dimensions",
    "contract-only synthetic champion-panel portrait route; no bitmap sampling",
    "without claiming real-asset portrait parity"
};

static const char s_source_evidence[] =
    "contract_only=1; CHAMDRAW.C F0292:810-812 calls "
    "F0354_INVENTORY_DrawStatusBoxPortrait only for the live inventory "
    "champion and then leaves only MASK0x0100_STATISTICS in the redraw mask; "
    "PANEL.C F0354:2208-2213 defines the 32x29 portrait box at "
    "championIndex*C69+7, top 0, bottom 28; PANEL.C F0354:2226-2232 "
    "uses C175+championIndex and blits the champion portrait with "
    "CM1_COLOR_NO_TRANSPARENCY; PANEL.C F0354:2237-2240 hatches the same "
    "status-box zone with C12 when party invisibility is active; DEFS.H "
    "and COORD.C anchor no-transparency, byte width, screen width, and "
    "portrait dimensions; without claiming real-asset portrait parity.";

const DM1_V1_ChampionPanelPortraitEvidencePc34Compat *
DM1_V1_ChampionPanelPortrait_EvidencePc34Compat(void)
{
    return &s_evidence;
}

const char *
DM1_V1_ChampionPanelPortrait_SourceEvidencePc34Compat(void)
{
    return s_source_evidence;
}

void DM1_V1_ChampionPanelPortrait_DefaultInputPc34Compat(
    DM1_V1_ChampionPanelPortraitInputPc34Compat *input)
{
    if (!input) {
        return;
    }

    memset(input, 0, sizeof(*input));
    input->contract_only = 1;
    input->champion_index = 0;
    input->inventory_champion_ordinal = 1;
    input->current_health = 1;
    input->party_invisibility_count = 0;
}

static int valid_champion_index(int champion_index)
{
    return champion_index >= 0 &&
           champion_index < DM1_V1_CPPOR_CHAMPION_COUNT_PC34;
}

static int valid_inventory_ordinal(int inventory_champion_ordinal)
{
    return inventory_champion_ordinal >= 0 &&
           inventory_champion_ordinal <= DM1_V1_CPPOR_CHAMPION_COUNT_PC34;
}

int DM1_V1_ChampionPanelPortrait_BuildPc34Compat(
    const DM1_V1_ChampionPanelPortraitInputPc34Compat *input,
    DM1_V1_ChampionPanelPortraitResultPc34Compat *out_result)
{
    DM1_V1_ChampionPanelPortraitInputPc34Compat local_input;
    int left;

    if (!out_result) {
        return 0;
    }

    memset(out_result, 0, sizeof(*out_result));
    out_result->contract_only = true;
    out_result->evidence = &s_evidence;
    out_result->status_box_zone = -1;
    out_result->target_left = -1;
    out_result->target_top = -1;
    out_result->target_right = -1;
    out_result->target_bottom = -1;
    out_result->transparent_color = DM1_V1_CPPOR_NO_TRANSPARENCY_PC34;
    out_result->hatch_color = -1;

    if (!input) {
        DM1_V1_ChampionPanelPortrait_DefaultInputPc34Compat(&local_input);
        input = &local_input;
    }

    if (!valid_champion_index(input->champion_index)) {
        /*
         * ReDMCSB CHAMDRAW.C F0292:755 selects M516_CHAMPIONS[index];
         * the panel contract is bounded to the four status boxes.
         */
        out_result->rejected_champion_index = true;
        return 0;
    }
    if (!valid_inventory_ordinal(input->inventory_champion_ordinal)) {
        /*
         * ReDMCSB CHAMDRAW.C F0292:759 compares the champion ordinal with
         * G0423_i_InventoryChampionOrdinal; valid ordinals are none or 1..4.
         */
        out_result->rejected_inventory_ordinal = true;
        return 0;
    }
    if (input->party_invisibility_count < 0) {
        /*
         * ReDMCSB PANEL.C F0354:2237 tests the party invisibility event
         * count as a nonnegative active/inactive counter.
         */
        out_result->rejected_invisibility_count = true;
        return 0;
    }

    out_result->valid = true;
    out_result->champion_index = input->champion_index;
    out_result->champion_ordinal = input->champion_index + 1;
    out_result->inventory_champion_ordinal = input->inventory_champion_ordinal;
    out_result->is_inventory_champion =
        out_result->champion_ordinal == input->inventory_champion_ordinal;
    out_result->is_alive = input->current_health > 0;
    out_result->should_draw_portrait =
        out_result->is_alive && out_result->is_inventory_champion;

    if (!out_result->should_draw_portrait) {
        /*
         * ReDMCSB CHAMDRAW.C F0292:810-814 reaches F0354 only for the live
         * inventory champion; all other live champions take the redraw-mask
         * branch and dead champions skip to the dead-status-box path.
         */
        return 1;
    }

    /*
     * ReDMCSB PANEL.C F0354:2208-2213 sets the status-box portrait target
     * to top=0, bottom=28, left=championIndex*C69+7, right=left+31.
     * The PC34 branch at 2226-2232 uses the corresponding C175+champion
     * zone with G2078/G2071 dimensions and CM1 no transparency.
     */
    left = input->champion_index * DM1_V1_CPPOR_STATUS_BOX_SPACING_PC34 +
           DM1_V1_CPPOR_STATUS_BOX_PORTRAIT_LEFT_MARGIN_PC34;
    out_result->status_box_zone =
        DM1_V1_CPPOR_ZONE_FIRST_STATUS_BOX_PC34 + input->champion_index;
    out_result->target_left = left;
    out_result->target_top = 0;
    out_result->target_right = left + DM1_V1_CPPOR_PORTRAIT_WIDTH_PC34 - 1;
    out_result->target_bottom = DM1_V1_CPPOR_PORTRAIT_HEIGHT_PC34 - 1;
    out_result->target_width = DM1_V1_CPPOR_PORTRAIT_WIDTH_PC34;
    out_result->target_height = DM1_V1_CPPOR_PORTRAIT_HEIGHT_PC34;
    out_result->portrait_source_x = 0;
    out_result->portrait_source_y = 0;
    out_result->portrait_width = DM1_V1_CPPOR_PORTRAIT_WIDTH_PC34;
    out_result->portrait_height = DM1_V1_CPPOR_PORTRAIT_HEIGHT_PC34;
    out_result->portrait_byte_width = DM1_V1_CPPOR_PORTRAIT_BYTE_WIDTH_PC34;
    out_result->screen_pixel_width = DM1_V1_CPPOR_SCREEN_PIXEL_WIDTH_PC34;

    out_result->operations[0] =
        DM1_V1_CPPOR_OP_GET_STATUS_BOX_ZONE_PC34;
    out_result->operations[1] =
        DM1_V1_CPPOR_OP_BLIT_CHAMPION_PORTRAIT_PC34;
    out_result->operation_count = 2;

    /*
     * ReDMCSB PANEL.C F0354:2237-2240 applies a C12 hatch to the same
     * C175+champion status-box zone while party invisibility is active.
     */
    if (input->party_invisibility_count > 0) {
        out_result->hatches_for_invisibility = true;
        out_result->hatch_color = DM1_V1_CPPOR_HATCH_COLOR_PC34;
        out_result->operations[2] =
            DM1_V1_CPPOR_OP_HATCH_INVISIBILITY_PC34;
        out_result->operation_count = 3;
    }

    return 1;
}
