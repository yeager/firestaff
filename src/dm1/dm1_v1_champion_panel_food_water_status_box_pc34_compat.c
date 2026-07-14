#include "dm1_v1_champion_panel_food_water_status_box_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_CPFW_CHAMPION_COUNT_PC34 = 4,
    DM1_V1_CPFW_STATUS_BOX_FIRST_ZONE_PC34 = 151,
    DM1_V1_CPFW_STATUS_BOX_LAST_ZONE_PC34 = 154,
    DM1_V1_CPFW_STATUS_BOX_STRIDE_PC34 = 69,
    DM1_V1_CPFW_COLOR_BLACK_PC34 = 0,
    DM1_V1_CPFW_COLOR_LIGHT_BROWN_PC34 = 5,
    DM1_V1_CPFW_COLOR_RED_PC34 = 8,
    DM1_V1_CPFW_COLOR_FLESH_PC34 = 10,
    DM1_V1_CPFW_COLOR_YELLOW_PC34 = 11,
    DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34 = 12,
    DM1_V1_CPFW_COLOR_BLUE_PC34 = 14,
    DM1_V1_CPFW_GFX_PANEL_EMPTY_PC34 = 20,
    DM1_V1_CPFW_GFX_FOOD_LABEL_PC34 = 30,
    DM1_V1_CPFW_GFX_WATER_LABEL_PC34 = 31,
    DM1_V1_CPFW_ZONE_PANEL_PC34 = 101,
    DM1_V1_CPFW_ZONE_FOOD_BAR_PC34 = 103,
    DM1_V1_CPFW_ZONE_WATER_BAR_PC34 = 104,
    DM1_V1_CPFW_ZONE_FOOD_LABEL_PC34 = 500,
    DM1_V1_CPFW_ZONE_WATER_LABEL_PC34 = 501,
    DM1_V1_CPFW_THING_NONE_PC34 = 0xFFFFu,
    DM1_V1_CPFW_THING_END_OF_LIST_PC34 = 0xFFFEu
};

/*
 * ReDMCSB source-lock anchors for this contract-only slice:
 * - CHEST.C F0334:113-132 closes G0426, clears non-empty G0425 slots, and
 *   relinks the visible chest cell list before the next panel/status draw.
 * - PANEL.C F0354:2299-2322 brackets an inventory close by clearing G0423,
 *   calling F0334, dirtying MASK0x1000_STATUS_BOX, then calling F0292.
 * - CHAMDRAW.C F0292:771-789 owns the C151..C154 live 67x29 status-box fill;
 *   F0292:804/807 uses C10_COLOR_FLESH transparency for status-box borders.
 * - CHAMPION.C F0284:93-130 owns the G0305 party champion count traversal
 *   and the M516 champion-state addressing used with G0423 ordinals.
 * - PANEL.C F0345:1579-1615 reads M516_CHAMPIONS[G0423-1], blits C030/C031
 *   food/water labels, and draws C103/C104 food/water bars via F0344.
 * - PANEL.C F0344:1493-1561 maps food/water counters to red/yellow/base
 *   colors and proportional fill.
 * - MENU.C F0409/F0410/F0411:1666-1721 are spell/flask helpers in this
 *   ReDMCSB snapshot, so the food/water draw is explicitly pinned to PANEL.C
 *   while this contract records the requested MENU symbols as disambiguated.
 * - DEFS.H:2076-2092,2157,2190-2191,3776-3778,3783-3786,3869-3870 define
 *   C10/C12, C69, C030/C031, C101/C103/C104, C151..C154, and C500/C501.
 */

static const char s_source_evidence[] =
    "contract_only=1; CHEST.C F0334:113-132 closes G0426 and relinks "
    "non-empty G0425 chest cells before panel/status drawing. PANEL.C "
    "F0354:2299-2322 clears G0423 on close, calls F0334, marks "
    "MASK0x1000_STATUS_BOX, then calls F0292. CHAMDRAW.C F0292:771-789 "
    "fills the live C151..C154 status box as 67x29 at stride C69; "
    "CHAMDRAW.C F0292:804/807 uses DEFS.H:2088 C10_COLOR_FLESH transparency "
    "for status-box border overlays. CHAMPION.C F0284:93-130 anchors the "
    "G0305 party-count/M516 champion-state traversal used with G0423. "
    "PANEL.C F0345:1579-1615 reads M516_CHAMPIONS[G0423-1], blits "
    "C030/C031 to C500/C501, and draws C103/C104 food/water bars through "
    "PANEL.C F0344:1493-1561. MENU.C F0409/F0410/F0411:1666-1721 are "
    "spell/flask helpers in this ReDMCSB snapshot, not food/water panel "
    "draw functions; this gate disambiguates them and pins the actual "
    "food/water draw to PANEL.C. DEFS.H:2076-2092,2157,2190-2191,"
    "3776-3778,3783-3786,3869-3870 provide the constants.";

static const dm1_v1_champion_panel_food_water_status_box_contract_pc34_t
    s_contract = {
        1,
        DM1_V1_CPFW_CHAMPION_COUNT_PC34,
        DM1_V1_CPFW_STATUS_BOX_FIRST_ZONE_PC34,
        DM1_V1_CPFW_STATUS_BOX_LAST_ZONE_PC34,
        DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34,
        DM1_V1_CPFW_STATUS_BOX_HEIGHT_PC34,
        DM1_V1_CPFW_STATUS_BOX_STRIDE_PC34,
        DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34,
        DM1_V1_CPFW_COLOR_FLESH_PC34,
        DM1_V1_CPFW_GFX_PANEL_EMPTY_PC34,
        DM1_V1_CPFW_GFX_FOOD_LABEL_PC34,
        DM1_V1_CPFW_GFX_WATER_LABEL_PC34,
        DM1_V1_CPFW_ZONE_FOOD_LABEL_PC34,
        DM1_V1_CPFW_ZONE_WATER_LABEL_PC34,
        DM1_V1_CPFW_ZONE_FOOD_BAR_PC34,
        DM1_V1_CPFW_ZONE_WATER_BAR_PC34,
        DM1_V1_CPFW_COLOR_LIGHT_BROWN_PC34,
        DM1_V1_CPFW_COLOR_BLUE_PC34,
        DM1_V1_CPFW_COLOR_YELLOW_PC34,
        DM1_V1_CPFW_COLOR_RED_PC34,
        "CHEST.C F0334:113-132",
        "MENU.C F0409/F0410/F0411:1666-1721 disambiguated",
        "PANEL.C F0344:1493-1561; F0345:1563-1616",
        "CHAMDRAW.C F0292:771-789,804,807",
        "CHAMPION.C F0284:93-130",
        "PANEL.C F0354:2299-2322",
        "DEFS.H C10/C12/C69/C030/C031/C101/C103/C104/C151-C154/C500-C501"
    };

const dm1_v1_champion_panel_food_water_status_box_contract_pc34_t *
dm1_v1_champion_panel_food_water_status_box_contract_pc34(void)
{
    return &s_contract;
}

const char *
dm1_v1_champion_panel_food_water_status_box_source_evidence_pc34(void)
{
    return s_source_evidence;
}

dm1_v1_champion_panel_food_water_status_box_input_pc34_t
dm1_v1_champion_panel_food_water_status_box_default_input_pc34(void)
{
    dm1_v1_champion_panel_food_water_status_box_input_pc34_t input;

    memset(&input, 0, sizeof(input));
    input.inventory_champion_ordinal = 2;
    input.party_champion_count = DM1_V1_CPFW_CHAMPION_COUNT_PC34;
    input.current_health = 99;
    input.food = 256;
    input.water = -700;
    input.poison_event_count = 0;
    input.open_chest_thing = 0x1234u;
    input.chest_slots[0] = 0x0101u;
    input.chest_slots[1] = (uint16_t)DM1_V1_CPFW_THING_NONE_PC34;
    input.chest_slots[2] = 0x0102u;
    input.chest_slots[3] = (uint16_t)DM1_V1_CPFW_THING_NONE_PC34;
    input.chest_slots[4] = 0x0103u;
    input.chest_slots[5] = (uint16_t)DM1_V1_CPFW_THING_NONE_PC34;
    input.chest_slots[6] = (uint16_t)DM1_V1_CPFW_THING_NONE_PC34;
    input.chest_slots[7] = (uint16_t)DM1_V1_CPFW_THING_NONE_PC34;
    return input;
}

static int valid_input(
    const dm1_v1_champion_panel_food_water_status_box_input_pc34_t *input)
{
    return input->party_champion_count > 0 &&
           input->party_champion_count <= DM1_V1_CPFW_CHAMPION_COUNT_PC34 &&
           input->inventory_champion_ordinal > 0 &&
           input->inventory_champion_ordinal <= input->party_champion_count;
}

static int color_for_amount(int amount, int base_color)
{
    /*
     * ReDMCSB PANEL.C F0344:1519-1525: red below -512, yellow below zero,
     * otherwise the food/water base color passed by F0345:1614-1615.
     */
    if (amount < -512) {
        return DM1_V1_CPFW_COLOR_RED_PC34;
    }
    if (amount < 0) {
        return DM1_V1_CPFW_COLOR_YELLOW_PC34;
    }
    return base_color;
}

static int proportional_units_for_amount(int amount)
{
    long normalized;

    /*
     * ReDMCSB PANEL.C F0344:1537-1544: the PC34 route adds 1024 then
     * passes a 0..10000 proportional value to F0637_GetProportionalZone.
     */
    normalized = amount + 1024L;
    if (normalized < 0) {
        normalized = 0;
    }
    if (normalized > 3072L) {
        normalized = 3072L;
    }
    return (int)((normalized * 10000L) / 3072L);
}

static void fill_status_box(
    dm1_v1_champion_panel_food_water_status_box_frame_pc34_t *frame)
{
    int i;

    for (i = 0; i < DM1_V1_CPFW_STATUS_BOX_BYTES_PC34; ++i) {
        frame->bytes[i] = (uint8_t)DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
    }
    frame->fill_pixel_count = DM1_V1_CPFW_STATUS_BOX_BYTES_PC34;
}

static int pixel_index(int x, int y)
{
    return y * DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34 + x;
}

static void apply_transparent_border(
    dm1_v1_champion_panel_food_water_status_box_frame_pc34_t *frame)
{
    int x;
    int y;
    int preserved;

    /*
     * ReDMCSB CHAMDRAW.C F0292:804/807: status-box border graphics use
     * C10_COLOR_FLESH as transparent, so C10 source pixels preserve the
     * C12 fill already written by F0292:786/789.
     */
    preserved = 0;
    for (x = 0; x < DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34; ++x) {
        frame->bytes[pixel_index(x, 0)] =
            (uint8_t)DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
        frame->bytes[pixel_index(x, DM1_V1_CPFW_STATUS_BOX_HEIGHT_PC34 - 1)] =
            (uint8_t)DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
        preserved += 2;
    }
    for (y = 1; y < DM1_V1_CPFW_STATUS_BOX_HEIGHT_PC34 - 1; ++y) {
        frame->bytes[pixel_index(0, y)] =
            (uint8_t)DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
        frame->bytes[pixel_index(DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34 - 1, y)] =
            (uint8_t)DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
        preserved += 2;
    }
    frame->transparent_border_pixel_count = preserved;
}

static void append_operation(
    dm1_v1_champion_panel_food_water_status_box_result_pc34_t *result,
    dm1_v1_champion_panel_food_water_status_box_operation_kind_pc34_t kind,
    int graphic_id,
    int zone_id,
    int transparent_color,
    int fill_color,
    int amount,
    int proportional_units,
    const char *source_evidence)
{
    dm1_v1_champion_panel_food_water_status_box_operation_pc34_t *operation;

    if (result->operation_count >= DM1_V1_CPFW_MAX_OPERATIONS_PC34) {
        return;
    }
    operation = &result->operations[result->operation_count];
    memset(operation, 0, sizeof(*operation));
    operation->kind = kind;
    operation->sequence = result->operation_count;
    operation->graphic_id = graphic_id;
    operation->zone_id = zone_id;
    operation->transparent_color = transparent_color;
    operation->fill_color = fill_color;
    operation->amount = amount;
    operation->proportional_units = proportional_units;
    operation->sourceEvidence = source_evidence;
    result->operation_count++;
}

dm1_v1_champion_panel_food_water_status_box_result_pc34_t
dm1_v1_champion_panel_food_water_status_box_probe_pc34(
    const dm1_v1_champion_panel_food_water_status_box_input_pc34_t *input)
{
    dm1_v1_champion_panel_food_water_status_box_result_pc34_t result;
    dm1_v1_champion_panel_food_water_status_box_input_pc34_t local_input;
    int i;

    memset(&result, 0, sizeof(result));
    result.contract_only = 1;
    result.sourceEvidence = s_source_evidence;
    result.loads_graphics_dat = 0;
    result.loads_dungeon_dat = 0;
    result.relink_first_thing = (uint16_t)DM1_V1_CPFW_THING_END_OF_LIST_PC34;
    result.relink_last_thing = (uint16_t)DM1_V1_CPFW_THING_END_OF_LIST_PC34;

    if (!input) {
        local_input = dm1_v1_champion_panel_food_water_status_box_default_input_pc34();
        input = &local_input;
    }
    if (!valid_input(input)) {
        result.rejected_invalid_champion = 1;
        return result;
    }
    if (input->current_health <= 0) {
        result.rejected_dead_champion = 1;
        return result;
    }

    result.valid = 1;
    result.inventory_champion_index = input->inventory_champion_ordinal - 1;
    result.party_champion_count = input->party_champion_count;
    result.g0423_inventory_champion_ordinal = input->inventory_champion_ordinal;
    result.g0305_party_champion_count = input->party_champion_count;
    result.food_counter = input->food;
    result.water_counter = input->water;
    result.poison_event_count = input->poison_event_count;
    result.food_word_color = DM1_V1_CPFW_COLOR_LIGHT_BROWN_PC34;
    result.water_word_color = DM1_V1_CPFW_COLOR_BLUE_PC34;
    result.food_bar_color = color_for_amount(
        input->food, DM1_V1_CPFW_COLOR_LIGHT_BROWN_PC34);
    result.water_bar_color = color_for_amount(
        input->water, DM1_V1_CPFW_COLOR_BLUE_PC34);
    result.food_bar_units = proportional_units_for_amount(input->food);
    result.water_bar_units = proportional_units_for_amount(input->water);
    result.menu_f0409_f0410_f0411_disambiguated = 1;

    result.frame.champion_index = result.inventory_champion_index;
    result.frame.zone_id =
        DM1_V1_CPFW_STATUS_BOX_FIRST_ZONE_PC34 + result.inventory_champion_index;
    result.frame.screen_x =
        result.inventory_champion_index * DM1_V1_CPFW_STATUS_BOX_STRIDE_PC34;
    result.frame.screen_y = 0;
    result.frame.width = DM1_V1_CPFW_STATUS_BOX_WIDTH_PC34;
    result.frame.height = DM1_V1_CPFW_STATUS_BOX_HEIGHT_PC34;
    result.frame.fill_color = DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34;
    result.frame.border_transparent_color = DM1_V1_CPFW_COLOR_FLESH_PC34;

    result.chest_was_open =
        input->open_chest_thing != DM1_V1_CPFW_THING_NONE_PC34;
    if (result.chest_was_open) {
        result.open_chest_cleared = 1;
        for (i = 0; i < DM1_V1_CPFW_CHEST_SLOT_COUNT_PC34; ++i) {
            if (input->chest_slots[i] != DM1_V1_CPFW_THING_NONE_PC34) {
                if (!result.non_empty_chest_slots) {
                    result.relink_first_thing = input->chest_slots[i];
                }
                result.relink_last_thing = input->chest_slots[i];
                result.non_empty_chest_slots++;
            }
        }
    }

    append_operation(&result,
                     DM1_V1_CPFW_OP_CHEST_CLOSE_PC34,
                     -1,
                     -1,
                     -1,
                     -1,
                     result.non_empty_chest_slots,
                     result.open_chest_cleared,
                     "CHEST.C F0334:113-132");
    append_operation(&result,
                     DM1_V1_CPFW_OP_PANEL_CLOSE_BRACKET_PC34,
                     -1,
                     -1,
                     -1,
                     -1,
                     input->inventory_champion_ordinal,
                     input->party_champion_count,
                     "PANEL.C F0354:2299-2322");

    fill_status_box(&result.frame);
    append_operation(&result,
                     DM1_V1_CPFW_OP_STATUS_BOX_FILL_PC34,
                     -1,
                     result.frame.zone_id,
                     -1,
                     DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34,
                     DM1_V1_CPFW_STATUS_BOX_BYTES_PC34,
                     DM1_V1_CPFW_STATUS_BOX_STRIDE_PC34,
                     "CHAMDRAW.C F0292:771-789");
    apply_transparent_border(&result.frame);
    append_operation(&result,
                     DM1_V1_CPFW_OP_STATUS_BOX_BORDER_PC34,
                     -1,
                     result.frame.zone_id,
                     DM1_V1_CPFW_COLOR_FLESH_PC34,
                     DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34,
                     result.frame.transparent_border_pixel_count,
                     DM1_V1_CPFW_STATUS_BOX_STRIDE_PC34,
                     "CHAMDRAW.C F0292:804,807 and DEFS.H:2088");

    append_operation(&result,
                     DM1_V1_CPFW_OP_PANEL_EMPTY_PC34,
                     DM1_V1_CPFW_GFX_PANEL_EMPTY_PC34,
                     DM1_V1_CPFW_ZONE_PANEL_PC34,
                     DM1_V1_CPFW_COLOR_RED_PC34,
                     -1,
                     0,
                     0,
                     "PANEL.C F0345:1597");
    append_operation(&result,
                     DM1_V1_CPFW_OP_FOOD_LABEL_PC34,
                     DM1_V1_CPFW_GFX_FOOD_LABEL_PC34,
                     DM1_V1_CPFW_ZONE_FOOD_LABEL_PC34,
                     DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34,
                     result.food_word_color,
                     input->food,
                     0,
                     "PANEL.C F0345:1598 and DEFS.H:2190,3869");
    append_operation(&result,
                     DM1_V1_CPFW_OP_WATER_LABEL_PC34,
                     DM1_V1_CPFW_GFX_WATER_LABEL_PC34,
                     DM1_V1_CPFW_ZONE_WATER_LABEL_PC34,
                     DM1_V1_CPFW_COLOR_DARKEST_GRAY_PC34,
                     result.water_word_color,
                     input->water,
                     0,
                     "PANEL.C F0345:1599 and DEFS.H:2191,3870");
    append_operation(&result,
                     DM1_V1_CPFW_OP_FOOD_BAR_PC34,
                     -1,
                     DM1_V1_CPFW_ZONE_FOOD_BAR_PC34,
                     -1,
                     result.food_bar_color,
                     input->food,
                     result.food_bar_units,
                     "PANEL.C F0344:1493-1561 and F0345:1614");
    append_operation(&result,
                     DM1_V1_CPFW_OP_WATER_BAR_PC34,
                     -1,
                     DM1_V1_CPFW_ZONE_WATER_BAR_PC34,
                     -1,
                     result.water_bar_color,
                     input->water,
                     result.water_bar_units,
                     "PANEL.C F0344:1493-1561 and F0345:1615");

    result.close_before_status_box =
        result.operations[0].kind == DM1_V1_CPFW_OP_CHEST_CLOSE_PC34 &&
        result.operations[2].kind == DM1_V1_CPFW_OP_STATUS_BOX_FILL_PC34;
    result.status_box_before_panel =
        result.operations[3].kind == DM1_V1_CPFW_OP_STATUS_BOX_BORDER_PC34 &&
        result.operations[4].kind == DM1_V1_CPFW_OP_PANEL_EMPTY_PC34;

    return result;
}
