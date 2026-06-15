#include "dm1/dm1_v1_viewport_d2c_stairs_pit_dispatch_pc34_compat.h"

/*
 * Source-locked contract_only=1 gate. ReDMCSB DUNVIEW.C:
 * F0121_DUNGEONVIEW_DrawSquareD2C lines 7256-7368 dispatches D2C
 * stairs/pit through F0104 and then the shared F0108/F0112/F0115 tail;
 * F0105 lines 3185-3247 is covered here as the sibling flipped
 * floor/pit/stairs bitmap helper contract. No real assets are loaded.
 */

static const DM1_V1_D2CDispatchEvidencePc34 s_evidence = {
    "DUNVIEW.C:F0121_DUNGEONVIEW_DrawSquareD2C:7256-7368; stairs rail slice 7257-7288, pit/tail 7343-7368",
    "DUNVIEW.C:F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap:3113-3156; C10_COLOR_FLESH transparent blit at 3145-3148",
    "DUNVIEW.C:F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally:3185-3247; C10_COLOR_FLESH transparent flipped blit at 3218-3239",
    "DUNVIEW.C:F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF:4547-4581; MEDIA720 view-square guard 5668-5671",
    "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8503-8517 D2L2/D2R2/D2L/D2R follow-up writes before D2C center at 8520-8521",
    "DUNGEON.C:F0163:1769-1838 and F0164:1840-1905 preserve square thing-list metadata; F0172:2466-2523,2628-2650,2693-2697,2721 binds pit/stair ordinals",
    "DEFS.H:2088 C10; 2443-2452 C03/C10 stair slots; 2582-2583,2596-2604 M603; 2662/2676 cell order; 4144-4162 and 4202-4207 zones",
    "Contract-only synthetic PC 3.4 compatibility gate; no real-asset bitmap or pixel parity claim."
};

static const DM1_V1_D2CFollowUpWritePc34 s_followups[] = {
    {2, -2, 10, "F0678_DrawD2L2"},
    {2, 2, 20, "F0679_DrawD2R2"},
    {2, -1, 30, "F0119_DUNGEONVIEW_DrawSquareD2L"},
    {2, 1, 40, "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF"}
};

static void init_dispatch_output(DM1_V1_D2CDispatchOutputPc34 *output,
                                 const DM1_V1_D2CDispatchInputPc34 *input)
{
    output->route_taken = DM1_V1_D2C_DISPATCH_PC34_ROUTE_UNSUPPORTED;
    output->native_bitmap_index = -1;
    output->zone_index = -1;
    output->ceiling_pit_graphic = DM1_V1_D2C_DISPATCH_PC34_CEILING_PIT_D2C_GRAPHIC;
    output->ceiling_pit_zone = DM1_V1_D2C_DISPATCH_PC34_ZONE_CEILING_PIT_D2C;
    output->view_square_index = DM1_V1_D2C_DISPATCH_PC34_VIEW_SQUARE_D2C;
    output->view_floor_index = DM1_V1_D2C_DISPATCH_PC34_VIEW_FLOOR_D2C;
    output->field_zone_index = DM1_V1_D2C_DISPATCH_PC34_ZONE_FIELD_D2C;
    output->floor_ornament_ordinal = input->floor_ornament_ordinal;
    output->first_thing = input->first_thing;
    output->cell_order_called = -1;
    output->pit_bitmap_order = 10;
    output->floor_ornament_order = 20;
    output->ceiling_pit_order = 30;
    output->thing_pass_order = 40;
    output->field_order = 50;
    output->used_f0104 = false;
    output->used_f0105 = false;
    output->used_f0108_floor_ornament = false;
    output->used_f0112_ceiling_pit = false;
    output->used_f0113_field = false;
    output->used_f0115 = false;
    output->bug0_64_floor_ornament_after_open_pit = false;
    output->wall_returned_before_tail = false;
    output->contract_only = true;
    output->real_asset_claim = false;
}

static void mark_tail(DM1_V1_D2CDispatchOutputPc34 *output)
{
    output->cell_order_called = DM1_V1_D2C_DISPATCH_PC34_CELL_ORDER_OPEN;
    output->used_f0108_floor_ornament = true;
    output->used_f0112_ceiling_pit = true;
    output->used_f0115 = true;
}

bool dm1_v1_viewport_d2c_stairs_pit_dispatch_probe_pc34(
    const DM1_V1_D2CDispatchInputPc34 *input,
    DM1_V1_D2CDispatchOutputPc34 *output)
{
    if (!input || !output) {
        return false;
    }

    init_dispatch_output(output, input);

    switch (input->element) {
    case DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT:
        output->route_taken = input->stairs_up
            ? DM1_V1_D2C_DISPATCH_PC34_ROUTE_STAIRS_UP_FRONT
            : DM1_V1_D2C_DISPATCH_PC34_ROUTE_STAIRS_DOWN_FRONT;
        output->native_bitmap_index = input->stairs_up
            ? DM1_V1_D2C_DISPATCH_PC34_STAIRS_UP_SLOT_D2C
            : DM1_V1_D2C_DISPATCH_PC34_STAIRS_DOWN_SLOT_D2C;
        output->zone_index = input->stairs_up
            ? DM1_V1_D2C_DISPATCH_PC34_ZONE_STAIRS_UP_D2C
            : DM1_V1_D2C_DISPATCH_PC34_ZONE_STAIRS_DOWN_D2C;
        output->used_f0104 = true;
        mark_tail(output);
        return true;

    case DM1_V1_D2C_DISPATCH_PC34_ELEMENT_PIT:
        output->route_taken = DM1_V1_D2C_DISPATCH_PC34_ROUTE_OPEN_PIT;
        output->native_bitmap_index = input->pit_or_teleporter_visible
            ? DM1_V1_D2C_DISPATCH_PC34_INVISIBLE_FLOOR_PIT_D2C_GRAPHIC
            : DM1_V1_D2C_DISPATCH_PC34_FLOOR_PIT_D2C_GRAPHIC;
        output->zone_index = DM1_V1_D2C_DISPATCH_PC34_ZONE_FLOOR_PIT_D2C;
        output->used_f0104 = true;
        output->bug0_64_floor_ornament_after_open_pit = true;
        mark_tail(output);
        return true;

    case DM1_V1_D2C_DISPATCH_PC34_ELEMENT_CORRIDOR:
        output->route_taken = DM1_V1_D2C_DISPATCH_PC34_ROUTE_CORRIDOR_TAIL;
        mark_tail(output);
        return true;

    case DM1_V1_D2C_DISPATCH_PC34_ELEMENT_TELEPORTER:
        output->route_taken = DM1_V1_D2C_DISPATCH_PC34_ROUTE_TELEPORTER_FIELD;
        mark_tail(output);
        output->used_f0113_field = true;
        output->zone_index = DM1_V1_D2C_DISPATCH_PC34_ZONE_FIELD_D2C;
        return true;

    case DM1_V1_D2C_DISPATCH_PC34_ELEMENT_WALL:
        output->route_taken = DM1_V1_D2C_DISPATCH_PC34_ROUTE_WALL_RETURN;
        output->wall_returned_before_tail = true;
        return true;

    default:
        return true;
    }
}

bool dm1_v1_viewport_d2c_stairs_pit_dispatch_metadata_pc34(
    const DM1_V1_D2CMetadataInputPc34 *input,
    DM1_V1_D2CMetadataOutputPc34 *output)
{
    int square_type;

    if (!input || !output) {
        return false;
    }

    square_type = input->raw_square >> 5;
    output->element = square_type;
    output->stairs_up = false;
    output->pit_or_teleporter_visible = false;
    output->footprints_allowed = false;
    output->floor_ornament_ordinal = input->floor_ornament_ordinal;
    output->first_thing = input->first_thing_after_metadata;

    switch (square_type) {
    case DM1_V1_D2C_DISPATCH_PC34_ELEMENT_PIT:
        if (input->raw_square & DM1_V1_D2C_DISPATCH_PC34_MASK_PIT_OPEN) {
            output->element = DM1_V1_D2C_DISPATCH_PC34_ELEMENT_PIT;
            output->pit_or_teleporter_visible =
                (input->raw_square & DM1_V1_D2C_DISPATCH_PC34_MASK_PIT_INVISIBLE) != 0;
        } else {
            output->element = DM1_V1_D2C_DISPATCH_PC34_ELEMENT_CORRIDOR;
            output->footprints_allowed = true;
        }
        return true;

    case DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS:
        output->element =
            (((input->raw_square & DM1_V1_D2C_DISPATCH_PC34_MASK_STAIRS_NS) >> 3) ==
             (input->direction & 1))
                ? DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS_SIDE
                : DM1_V1_D2C_DISPATCH_PC34_ELEMENT_STAIRS_FRONT;
        output->stairs_up =
            (input->raw_square & DM1_V1_D2C_DISPATCH_PC34_MASK_STAIRS_UP) != 0;
        output->footprints_allowed = false;
        return true;

    case DM1_V1_D2C_DISPATCH_PC34_ELEMENT_CORRIDOR:
        output->footprints_allowed = true;
        return true;

    default:
        return true;
    }
}

bool dm1_v1_viewport_d2c_stairs_pit_dispatch_center_geometry_pc34(
    int map_x,
    int map_y,
    DM1_V1_D2CCenterGeometryPc34 *output)
{
    if (!output) {
        return false;
    }

    output->base_map_x = map_x;
    output->base_map_y = map_y;
    output->relative_depth = 2;
    output->relative_lateral = 0;
    output->resolved_map_x = map_x + 2;
    output->resolved_map_y = map_y;
    output->view_square_index = DM1_V1_D2C_DISPATCH_PC34_VIEW_SQUARE_D2C;
    output->field_zone_index = DM1_V1_D2C_DISPATCH_PC34_ZONE_FIELD_D2C;
    return true;
}

const DM1_V1_D2CFollowUpWritePc34 *
dm1_v1_viewport_d2c_stairs_pit_dispatch_followups_pc34(size_t *count)
{
    if (count) {
        *count = sizeof(s_followups) / sizeof(s_followups[0]);
    }
    return s_followups;
}

bool dm1_v1_viewport_d2c_stairs_pit_dispatch_f0105_blit_pc34(
    const DM1_V1_D2CF0105BlitInputPc34 *input,
    DM1_V1_D2CF0105BlitOutputPc34 *output)
{
    size_t x;
    size_t y;

    if (!input || !output || !input->source || !input->destination ||
        input->width == 0 || input->height == 0 ||
        input->source_len < input->width * input->height ||
        input->destination_stride < input->width ||
        input->destination_len < input->destination_stride * input->height) {
        return false;
    }

    output->ok = true;
    output->used_f0105 = true;
    output->copied_with_horizontal_flip = true;
    output->native_bitmap_index = input->native_bitmap_index;
    output->zone_index = input->zone_index;
    output->transparent_color = DM1_V1_D2C_DISPATCH_PC34_COLOR_TRANSPARENT;
    output->writes = 0;
    output->transparent_skips = 0;

    for (y = 0; y < input->height; ++y) {
        for (x = 0; x < input->width; ++x) {
            const uint8_t pixel = input->source[(y * input->width) + (input->width - 1 - x)];
            if (pixel == DM1_V1_D2C_DISPATCH_PC34_COLOR_TRANSPARENT) {
                ++output->transparent_skips;
                continue;
            }
            input->destination[(y * input->destination_stride) + x] = pixel;
            ++output->writes;
        }
    }

    output->first_destination_byte = input->destination[0];
    output->last_destination_byte =
        input->destination[((input->height - 1) * input->destination_stride) +
                           (input->width - 1)];
    return true;
}

const DM1_V1_D2CDispatchEvidencePc34 *
dm1_v1_viewport_d2c_stairs_pit_dispatch_evidence_pc34(void)
{
    return &s_evidence;
}
