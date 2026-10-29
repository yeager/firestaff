#include "csb_v1_atari_f0115_creature_plan.h"

#include <string.h>

enum {
    CSB_V1_GI_FLIP_NON_ATTACK = 0x0004,
    CSB_V1_GI_SIDE = 0x0008,
    CSB_V1_GI_BACK = 0x0010,
    CSB_V1_GI_ATTACK = 0x0020,
    CSB_V1_GI_SPECIAL_D2_FRONT = 0x0080,
    CSB_V1_GI_SPECIAL_D2_FRONT_FLIPPED = 0x0100,
    CSB_V1_ASPECT_FLIP = 0x40,
    CSB_V1_ASPECT_ATTACKING = 0x80
};

static int csb_v1_atari_creature_ordinal_in_cell(
    const CSB_V1_AtariF0115CreatureInput *input,
    int absolute_cell,
    int *out_index)
{
    int index;
    int query_cell = absolute_cell;

    if (input->cells == CSB_V1_ATARI_F0115_SINGLE_CENTERED) {
        *out_index = 0;
        return 1;
    }
    if (input->creature_size == 1 &&
        ((input->directions & 1u) == ((unsigned)absolute_cell & 1u))) {
        query_cell = (absolute_cell + 3) & 3;
    }
    for (index = input->creature_count_minus_one; index >= 0; --index) {
        const int cell = (input->cells >> (index * 2)) & 3;
        if (cell == query_cell ||
            (input->creature_size == 1 && cell == ((query_cell + 1) & 3))) {
            *out_index = index;
            return 1;
        }
    }
    *out_index = -1;
    return 0;
}

static int csb_v1_atari_f0115_direction_delta(int party_direction,
                                               uint16_t directions,
                                               int creature_index)
{
    const int facing = (directions >> (creature_index * 2)) & 3;
    return (party_direction - facing) & 3;
}

int csb_v1_atari_f0115_plan_creature(
    const CSB_V1_AtariF0115CreatureInput *input,
    CSB_V1_AtariF0115CreaturePlan *out_plan)
{
    const uint8_t *aspect;
    unsigned int first_native;
    unsigned int first_derived;
    unsigned int coordinate_set;
    unsigned int coordinate_offset;
    unsigned int coordinate_row;
    int absolute_cell;
    int creature_index;
    int direction_delta;
    int coordinate_cell;
    int found;
    int bitmap_offset = 0;
    int byte_width;
    int height;
    int source_depth;
    int uses_side = 0;
    int uses_back = 0;
    int uses_attack = 0;
    int uses_special_d2 = 0;
    int flipped = 0;
    uint8_t aspect_frame;
    uint16_t gi;
    uint8_t x;
    uint8_t y;

    if (!input || !out_plan || !input->c558 ||
        input->c558_size != CSB_V1_ATARI_F0115_C558_SIZE ||
        input->creature_type < 0 ||
        input->creature_type >= CSB_V1_ATARI_F0115_G0219_COUNT ||
        input->creature_count_minus_one < 0 ||
        input->creature_count_minus_one > 3 ||
        input->creature_size < 0 || input->creature_size > 2 ||
        input->party_direction < 0 || input->party_direction > 3 ||
        input->view_square < 0 || input->view_square > 11 ||
        input->view_cell < 0 || input->view_cell > 3 ||
        input->door_front_view_pass < 0 || input->door_front_view_pass > 2) {
        return -1;
    }
    memset(out_plan, 0, sizeof(*out_plan));

    /* ReDMCSB F0115 explicitly suppresses creature drawing on D0C. That
     * square has no G0224 creature-coordinate row; row 9 is D0L, and the
     * D0L/D0R indices 10/11 fold to rows 9/10 below. */
    if (input->view_square == 9) return 0;

    aspect = input->c558 + CSB_V1_ATARI_F0115_G0219_OFFSET +
        (size_t)input->creature_type * CSB_V1_ATARI_F0115_G0219_STRIDE;
    first_native = ((unsigned int)aspect[0] << 8) | aspect[1];
    first_derived = ((unsigned int)aspect[2] << 8) | aspect[3];
    coordinate_set = aspect[10] >> 4;
    if (first_native >= 117u || coordinate_set >= 3u ||
        aspect[4] == 0u || aspect[5] == 0u) {
        return -1;
    }

    absolute_cell = (input->view_cell + input->party_direction) & 3;
    found = csb_v1_atari_creature_ordinal_in_cell(
        input, absolute_cell, &creature_index);
    if (!found && input->creature_size != 1) return 0;
    if (!found) creature_index = -1;
    direction_delta = csb_v1_atari_f0115_direction_delta(
        input->party_direction, input->directions,
        creature_index >= 0 ? creature_index : 0);
    aspect_frame = input->aspect[creature_index >= 0 ? creature_index : 0];
    coordinate_cell = input->view_cell;

    if (input->cells == CSB_V1_ATARI_F0115_SINGLE_CENTERED) {
        /* F0115 draws a centered group only after the square's floor-cell
         * ordinals have been exhausted, and never on door pass one. */
        if (input->remaining_view_cells || input->door_front_view_pass == 1)
            return 0;
        if (input->creature_size == 1 && (direction_delta & 1))
            coordinate_cell = 3; /* C03_VIEW_CELL_CENTER_COLUMN */
        else
            coordinate_cell = 4; /* C04_VIEW_CELL_FRONT_ROW */
        creature_index = 0;
    } else if (input->creature_size == 1 &&
               (input->drawing_last_back_row_cell ||
                !input->remaining_view_cells || creature_index < 0)) {
        if (input->drawing_last_back_row_cell &&
            input->door_front_view_pass != 2) {
            if (creature_index < 0 || !(direction_delta & 1)) return 0;
            coordinate_cell = 2; /* C02_VIEW_CELL_BACK_ROW */
        } else if (input->door_front_view_pass != 1 &&
                   !input->remaining_view_cells) {
            if (direction_delta & 1) {
                if (creature_index < 0) return 0;
                coordinate_cell = 4; /* C04_VIEW_CELL_FRONT_ROW */
            } else {
                int occupied_cell;
                if (creature_index < 0) creature_index = 0;
                occupied_cell = (input->cells >> (creature_index * 2)) & 3;
                coordinate_cell =
                    (occupied_cell == input->party_direction ||
                     occupied_cell == ((input->party_direction + 3) & 3))
                    ? 0 : 1; /* C00/C01_VIEW_CELL_LEFT/RIGHT_COLUMN */
            }
        } else {
            return 0;
        }
    } else if (input->creature_size != 0) {
        return 0;
    }

    /* F0115 folds D0L/D0R source indices 10/11 onto G0224 rows 9/10. */
    coordinate_row = (unsigned int)input->view_square;
    if (input->view_square > 9) --coordinate_row;
    coordinate_offset = CSB_V1_ATARI_F0115_G0224_OFFSET +
        (((coordinate_set * 11u + coordinate_row) * 5u +
          (unsigned int)coordinate_cell) * 2u);
    if (coordinate_offset + 1u >=
        CSB_V1_ATARI_F0115_G0224_OFFSET +
            CSB_V1_ATARI_F0115_G0224_SIZE) {
        return -1;
    }
    x = input->c558[coordinate_offset];
    y = input->c558[coordinate_offset + 1u];
    if (y == 0u) return 0;
    if (x >= 224u || y > 135u) return -1;

    gi = input->graphic_info;
    if ((gi & CSB_V1_GI_SIDE) && (direction_delta & 1)) {
        uses_side = 1;
        bitmap_offset = 1;
        byte_width = aspect[6];
        height = aspect[7];
        flipped = direction_delta == 1;
    } else {
        uses_back = (gi & CSB_V1_GI_BACK) && direction_delta == 0;
        uses_attack = !uses_back && (aspect_frame & CSB_V1_ASPECT_ATTACKING) &&
            (gi & CSB_V1_GI_ATTACK);
        if (uses_attack) {
            bitmap_offset = 1;
            if (gi & CSB_V1_GI_SIDE) ++bitmap_offset;
            if (gi & CSB_V1_GI_BACK) ++bitmap_offset;
            byte_width = aspect[8];
            height = aspect[9];
            flipped = (aspect_frame & CSB_V1_ASPECT_FLIP) != 0;
        } else {
            byte_width = aspect[4];
            height = aspect[5];
            if (uses_back) {
                bitmap_offset = (gi & CSB_V1_GI_SIDE) ? 2 : 1;
            } else {
                flipped = (gi & CSB_V1_GI_FLIP_NON_ATTACK) &&
                    (aspect_frame & CSB_V1_ASPECT_FLIP);
            }
        }
    }
    if ((uses_side && (aspect[6] == 0u || aspect[7] == 0u)) ||
        (uses_attack && (aspect[8] == 0u || aspect[9] == 0u))) {
        return -1;
    }
    uses_special_d2 = input->view_square >= 3 && input->view_square <= 5 &&
        (gi & CSB_V1_GI_SPECIAL_D2_FRONT) && !uses_side && !uses_back &&
        !uses_attack;
    if (uses_special_d2 &&
        (gi & CSB_V1_GI_SPECIAL_D2_FRONT_FLIPPED)) flipped = 1;
    source_depth = input->view_square <= 2 ? 2 :
        input->view_square <= 5 ? 1 : 0;

    out_plan->creature_index = creature_index;
    out_plan->absolute_cell = absolute_cell;
    out_plan->coordinate_set = (int)coordinate_set;
    out_plan->coordinate_cell = coordinate_cell;
    out_plan->anchor_x = x;
    out_plan->anchor_y = y;
    out_plan->source_depth = source_depth;
    out_plan->bitmap_index = CSB_V1_ATARI_F0115_GRAPHIC_FIRST_CREATURE +
        (int)first_native + bitmap_offset;
    out_plan->source_byte_width = byte_width;
    out_plan->height = height;
    out_plan->transparent_color = aspect[10] & 0x0f;
    if (source_depth == 2) {
        static const uint8_t d3_palette[16] = {
            0,12,1,3,4,3,0,6,3,0,0,11,0,2,0,13
        };
        out_plan->palette_transparent_color = d3_palette[aspect[10] & 0x0f];
    } else if (source_depth == 1) {
        static const uint8_t d2_palette[16] = {
            0,1,2,3,4,3,6,7,5,0,0,11,12,13,14,15
        };
        out_plan->palette_transparent_color = d2_palette[aspect[10] & 0x0f];
    } else {
        out_plan->palette_transparent_color = out_plan->transparent_color;
    }
    out_plan->direction_delta = direction_delta;
    out_plan->mirrored = flipped;
    out_plan->attacking = (aspect_frame & CSB_V1_ASPECT_ATTACKING) != 0;
    out_plan->uses_side_bitmap = uses_side;
    out_plan->uses_back_bitmap = uses_back;
    out_plan->uses_attack_bitmap = uses_attack;
    out_plan->uses_special_d2_front = uses_special_d2;
    out_plan->flipped_special_d2_front = uses_special_d2 &&
        (gi & CSB_V1_GI_SPECIAL_D2_FRONT_FLIPPED) != 0;
    out_plan->requires_derived_depth_bitmap = out_plan->source_depth > 0;
    (void)first_derived; /* The renderer owns Atari derived-bitmap cache keys. */
    return 1;
}

int csb_v1_atari_f0115_decode_cell_order(
    uint16_t ordered_view_cell_ordinals,
    CSB_V1_AtariF0115CellOrder *out_order)
{
    uint16_t remaining = ordered_view_cell_ordinals;
    unsigned cell_counter = 0u;

    if (!out_order) return -1;
    memset(out_order, 0, sizeof(*out_order));
    if (remaining == 0u) {
        /* ReDMCSB F0115 sets DrawAlcoveObjects when the packed order is zero
         * and invokes one alcove pass at C04_VIEW_CELL_ALCOVE (index 4). */
        out_order->alcove_only = 1u;
        return 0;
    }
    if ((remaining & 0x0008u) != 0u) {
        const unsigned door_nibble = remaining & 0x000fu;
        if (door_nibble != 8u && door_nibble != 9u) return -1;
        out_order->door_front_pass = (uint8_t)((remaining & 1u) + 1u);
        remaining >>= 4;
    }
    while (remaining != 0u) {
        const unsigned nibble = remaining & 0x000fu;
        const uint16_t after = (uint16_t)(remaining >> 4);
        const int view_cell = (int)nibble - 1;
        const unsigned slot = out_order->count;

        if (nibble == 0u || nibble > 4u || slot >= 4u) {
            memset(out_order, 0, sizeof(*out_order));
            return -1;
        }
        out_order->view_cell[slot] = (uint8_t)view_cell;
        out_order->remaining_after[slot] = after;
        /* DUNVIEW.C:5202-5204. Source calls the current back-row item the
         * last one if the next ordinal starts the front row (3 or 4), or no
         * ordinal remains; the first two-cell call has its own special case. */
        out_order->drawing_last_back_row_cell[slot] =
            (uint8_t)(((view_cell <= 1 || cell_counter == 1u) &&
                       (after == 0u || (after & 0x000fu) >= 3u)) ? 1u : 0u);
        ++out_order->count;
        ++cell_counter;
        remaining = after;
    }
    return (int)out_order->count;
}

int csb_v1_atari_f0115_plan_pass_events(
    uint16_t ordered_view_cell_ordinals,
    CSB_V1_AtariF0115PassEvent *out_events,
    size_t event_capacity)
{
    CSB_V1_AtariF0115CellOrder order;
    unsigned int cell_index;
    size_t required_events;
    size_t event_index;
    const int decoded = csb_v1_atari_f0115_decode_cell_order(
        ordered_view_cell_ordinals, &order);

    if (decoded < 0) return -1;
    if (decoded == 0) return 0;
    required_events = (size_t)decoded * 3u + 1u +
        (order.door_front_pass == 1u ? 0u : 1u);
    if (!out_events || event_capacity < required_events) return -1;

    /* ReDMCSB DUNVIEW.C F0115's function contract (4547-4582) and labels
     * T0115129/T0115214: visit each cell for objects, creature and projectile;
     * scan explosions after the cell loop; draw Fluxcage as a field last.
     * The first door pass records but does not draw Fluxcage (4795 and 6199). */
    event_index = 0u;
    for (cell_index = 0u; cell_index < order.count; ++cell_index) {
        static const uint8_t cell_layers[] = {
            CSB_V1_ATARI_F0115_OBJECT_LAYER,
            CSB_V1_ATARI_F0115_CREATURE_LAYER,
            CSB_V1_ATARI_F0115_PROJECTILE_LAYER
        };
        unsigned int layer_index;
        for (layer_index = 0u;
             layer_index < sizeof(cell_layers) / sizeof(cell_layers[0]);
             ++layer_index) {
            CSB_V1_AtariF0115PassEvent *event = &out_events[event_index++];
            event->view_cell = order.view_cell[cell_index];
            event->layer = cell_layers[layer_index];
            event->remaining_view_cells = order.remaining_after[cell_index];
            event->drawing_last_back_row_cell =
                order.drawing_last_back_row_cell[cell_index];
            event->door_front_view_pass = order.door_front_pass;
        }
    }
    out_events[event_index].view_cell = CSB_V1_ATARI_F0115_POST_CELL;
    out_events[event_index].layer = CSB_V1_ATARI_F0115_EXPLOSION_LAYER;
    out_events[event_index].remaining_view_cells = 0u;
    out_events[event_index].drawing_last_back_row_cell = 0u;
    out_events[event_index].door_front_view_pass = order.door_front_pass;
    ++event_index;
    if (order.door_front_pass != 1u) {
        out_events[event_index].view_cell = CSB_V1_ATARI_F0115_POST_CELL;
        out_events[event_index].layer = CSB_V1_ATARI_F0115_FLUXCAGE_LAYER;
        out_events[event_index].remaining_view_cells = 0u;
        out_events[event_index].drawing_last_back_row_cell = 0u;
        out_events[event_index].door_front_view_pass = order.door_front_pass;
        ++event_index;
    }
    return (int)event_index;
}

int csb_v1_atari_f0115_object_shift_index(
    int party_direction, int view_cell, unsigned int pile_ordinal,
    unsigned int *out_shift_index)
{
    unsigned int absolute_cell;
    if (!out_shift_index || party_direction < 0 || party_direction > 3 ||
        view_cell < 0 || view_cell > 3 || pile_ordinal > 7u) {
        return 0;
    }
    absolute_cell = ((unsigned int)view_cell +
                     (unsigned int)party_direction) & 3u;
    *out_shift_index = ((absolute_cell & 1u) << 3) + pile_ordinal;
    return 1;
}

unsigned int csb_v1_atari_f0115_scaled_dimension(
    unsigned int dimension, unsigned int scale)
{
    /* ReDMCSB DEFS.H M078_SCALED_DIMENSION(dimension, scale). */
    return (dimension * scale + (scale >> 1)) >> 5;
}

unsigned int csb_v1_atari_f0115_scale_for_depth(int source_depth)
{
    /* ReDMCSB DEFS.H:2489-2513: D3=16/32, D2=20/32, D1/D0=32/32. */
    static const unsigned int scale_by_depth[] = {32u, 20u, 16u};
    if (source_depth < 0 || source_depth > 2) return 0u;
    return scale_by_depth[source_depth];
}
