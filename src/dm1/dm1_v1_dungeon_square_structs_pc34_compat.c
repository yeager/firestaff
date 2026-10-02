/*
 * DM1 V1 Dungeon Square Data Structures — Implementation
 * ========================================================
 *
 * Implements square decoding, viewport coordinate calculation,
 * depth-zone traversal, wall-zone determination, and occlusion logic.
 *
 * Source reference (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *
 *   DUNGEON.C:
 *     G0233_ai_Graphic559_DirectionToStepEastCount[4]   (lines 30-34)
 *     G0234_ai_Graphic559_DirectionToStepNorthCount[4]  (lines 35-39)
 *     F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement (lines 1371-1421)
 *     F0151_DUNGEON_GetSquare                            (lines 1423-1475)
 *     F0152_DUNGEON_GetRelativeSquare                    (source-adjacent helper)
 *     F0172_DUNGEON_SetSquareAspect                      (lines 2466-2721)
 *
 *   DEFS.H:
 *     M034_SQUARE_TYPE(square) = ((square) >> 5)
 *     M035_SQUARE(element, mask) = (((element) << 5) | mask)
 *     M016_IS_ORIENTED_WEST_EAST(direction) = ((direction) & 0x0001)
 *     M597..M611 view square indices
 *     M575..M587 view wall indices
 *
 *   DUNVIEW.C (line 700+):
 *     Viewport draw loop: depth 3→0, per lane, calls
 *     F0172 SetSquareAspect. Front walls (element==WALL in the center lane)
 *     occlude everything behind them.
 *
 *   DRAWVIEW.C:
 *     F0093_DUNGEONVIEW_DrawDungeon — master viewport draw.
 */

#include "dm1_v1_dungeon_square_structs_pc34_compat.h"
#include <string.h>


/* =======================================================================
 * Direction-to-Step Lookup Tables
 * (DUNGEON.C lines 30-39: G0233, G0234)
 *
 *   Map: X increases eastward, Y increases southward (standard DM convention).
 *   North = (0, -1), East = (1, 0), South = (0, 1), West = (-1, 0).
 * ======================================================================= */

const int dm1_direction_to_step_east[4] = {
     0,   /* North */
     1,   /* East */
     0,   /* South */
    -1    /* West */
};

const int dm1_direction_to_step_north[4] = {
    -1,   /* North: Y decreases */
     0,   /* East */
     1,   /* South: Y increases */
     0    /* West */
};


/* =======================================================================
 * dm1_decode_square
 *
 * Decodes a raw square byte (as returned by F0151_DUNGEON_GetSquare)
 * into structured fields.
 *
 * ReDMCSB-referens:
 *   DEFS.H — M034_SQUARE_TYPE, all MASK definitions for each element type.
 *   DUNGEON.C:F0172 — How fields are interpreted for each square type.
 * ======================================================================= */

void dm1_decode_square(uint8_t raw_byte, dm1_dungeon_square_t *out) {
    memset(out, 0, sizeof(*out));

    out->raw_byte = raw_byte;
    out->element = DM1_SQUARE_TYPE(raw_byte);
    out->has_thing_list = DM1_SQUARE_HAS_THINGS(raw_byte);

    uint8_t flags = raw_byte & 0x0F;  /* Bits 3-0 */

    switch (out->element) {
        case DM1_ELEMENT_WALL:
            out->flags.wall.west_random_orn  = (flags & DM1_WALL_WEST_RANDOM_ORN) != 0;
            out->flags.wall.south_random_orn = (flags & DM1_WALL_SOUTH_RANDOM_ORN) != 0;
            out->flags.wall.east_random_orn  = (flags & DM1_WALL_EAST_RANDOM_ORN) != 0;
            out->flags.wall.north_random_orn = (flags & DM1_WALL_NORTH_RANDOM_ORN) != 0;
            break;

        case DM1_ELEMENT_CORRIDOR:
            out->flags.corridor.random_orn_allowed = (flags & DM1_CORRIDOR_RANDOM_ORN) != 0;
            break;

        case DM1_ELEMENT_PIT:
            out->flags.pit.imaginary  = (flags & DM1_PIT_IMAGINARY) != 0;
            out->flags.pit.invisible  = (flags & DM1_PIT_INVISIBLE) != 0;
            out->flags.pit.open       = (flags & DM1_PIT_OPEN) != 0;
            break;

        case DM1_ELEMENT_STAIRS:
            out->flags.stairs.up              = (flags & DM1_STAIRS_UP) != 0;
            out->flags.stairs.ns_orientation  = (flags & DM1_STAIRS_NS_ORIENTATION) != 0;
            break;

        case DM1_ELEMENT_DOOR:
            out->flags.door.state            = flags & DM1_DOOR_STATE_MASK;
            out->flags.door.ns_orientation   = (flags & DM1_DOOR_NS_ORIENTATION) != 0;
            break;

        case DM1_ELEMENT_TELEPORTER:
            out->flags.teleporter.visible = (flags & DM1_TELEPORTER_VISIBLE) != 0;
            out->flags.teleporter.open    = (flags & DM1_TELEPORTER_OPEN) != 0;
            break;

        case DM1_ELEMENT_FAKEWALL:
            out->flags.fakewall.imaginary          = (flags & DM1_FAKEWALL_IMAGINARY) != 0;
            out->flags.fakewall.open               = (flags & DM1_FAKEWALL_OPEN) != 0;
            out->flags.fakewall.random_orn_allowed = (flags & DM1_FAKEWALL_RANDOM_ORN) != 0;
            break;

        default:
            /* Unknown type — leave zeroed */
            break;
    }
}


/* =======================================================================
 * dm1_get_relative_map_coords
 *
 * Motsvarar F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement
 * (DUNGEON.C lines 867-935).
 *
 * Given the party position (X,Y), direction, forward steps, and right steps,
 * calculate absolute coordinates (out_x, out_y).
 *
 * Algorithm (directly from the source code):
 *   1. Apply forward steps in the party's direction:
 *      X += east_count[direction] * steps_forward
 *      Y += north_count[direction] * steps_forward
 *   2. Simulate a right turn (direction + 1) & 3:
 *      X += east_count[right_dir] * steps_right
 *      Y += north_count[right_dir] * steps_right
 * ======================================================================= */

void dm1_get_relative_map_coords(int party_x, int party_y, int direction,
                                  int steps_forward, int steps_right,
                                  int *out_x, int *out_y) {
    int dir = direction & 3;
    int x = party_x;
    int y = party_y;

    /* Step 1: Forward in the party's direction */
    x += dm1_direction_to_step_east[dir]  * steps_forward;
    y += dm1_direction_to_step_north[dir] * steps_forward;

    /* Step 2: Right = direction + 1 */
    int right_dir = (dir + 1) & 3;
    x += dm1_direction_to_step_east[right_dir]  * steps_right;
    y += dm1_direction_to_step_north[right_dir] * steps_right;

    *out_x = x;
    *out_y = y;
}

static uint8_t dm1_v1_square_byte_pc34(int element, int flags)
{
    return (uint8_t)(((element & 7) << 5) | (flags & 0x1f));
}

int dm1_v1_dungeon_f0150_update_map_coordinates_after_relative_movement_pc34(
    int party_x,
    int party_y,
    int direction,
    int steps_forward,
    int steps_right,
    DM1_V1_DungeonF0150CoordinatesPc34 *out)
{
    int x;
    int y;

    if (!out) {
        return 0;
    }
    out->valid = 0;
    out->x = party_x;
    out->y = party_y;

    dm1_get_relative_map_coords(party_x, party_y, direction,
                                steps_forward, steps_right, &x, &y);
    out->valid = 1;
    out->x = x;
    out->y = y;
    return 1;
}

uint8_t dm1_v1_dungeon_f0151_get_square_pc34(
    const uint8_t *column_major_squares,
    int width,
    int height,
    int map_x,
    int map_y)
{
    int x_in_bounds;
    int y_in_bounds;

    if (!column_major_squares || width <= 0 || height <= 0) {
        return dm1_v1_square_byte_pc34(DM1_ELEMENT_WALL, 0);
    }

    x_in_bounds = map_x >= 0 && map_x < width;
    y_in_bounds = map_y >= 0 && map_y < height;

    if (x_in_bounds && y_in_bounds) {
        return column_major_squares[(map_x * height) + map_y];
    }

    /* ReDMCSB DUNGEON.C F0151 returns a synthetic wall just outside a
     * corridor/pit map edge, with the random-ornament flag facing back into
     * the map. Diagonal and farther-out coordinates are plain walls. */
    if (y_in_bounds) {
        if (map_x == -1) {
            int edge_type = DM1_SQUARE_TYPE(column_major_squares[map_y]);
            if (edge_type == DM1_ELEMENT_CORRIDOR ||
                edge_type == DM1_ELEMENT_PIT) {
                return dm1_v1_square_byte_pc34(
                    DM1_ELEMENT_WALL, DM1_WALL_EAST_RANDOM_ORN);
            }
        } else if (map_x == width) {
            int edge_type =
                DM1_SQUARE_TYPE(column_major_squares[((width - 1) * height) +
                                                     map_y]);
            if (edge_type == DM1_ELEMENT_CORRIDOR ||
                edge_type == DM1_ELEMENT_PIT) {
                return dm1_v1_square_byte_pc34(
                    DM1_ELEMENT_WALL, DM1_WALL_WEST_RANDOM_ORN);
            }
        }
    } else if (x_in_bounds) {
        if (map_y == -1) {
            int edge_type =
                DM1_SQUARE_TYPE(column_major_squares[map_x * height]);
            if (edge_type == DM1_ELEMENT_CORRIDOR ||
                edge_type == DM1_ELEMENT_PIT) {
                return dm1_v1_square_byte_pc34(
                    DM1_ELEMENT_WALL, DM1_WALL_SOUTH_RANDOM_ORN);
            }
        } else if (map_y == height) {
            int edge_type =
                DM1_SQUARE_TYPE(column_major_squares[(map_x * height) +
                                                     (height - 1)]);
            if (edge_type == DM1_ELEMENT_CORRIDOR ||
                edge_type == DM1_ELEMENT_PIT) {
                return dm1_v1_square_byte_pc34(
                    DM1_ELEMENT_WALL, DM1_WALL_NORTH_RANDOM_ORN);
            }
        }
    }

    return dm1_v1_square_byte_pc34(DM1_ELEMENT_WALL, 0);
}

uint8_t dm1_v1_dungeon_f0152_get_relative_square_pc34(
    const uint8_t *column_major_squares,
    int width,
    int height,
    int party_x,
    int party_y,
    int direction,
    int steps_forward,
    int steps_right)
{
    DM1_V1_DungeonF0150CoordinatesPc34 coords;

    if (!dm1_v1_dungeon_f0150_update_map_coordinates_after_relative_movement_pc34(
            party_x, party_y, direction, steps_forward, steps_right,
            &coords) ||
        !coords.valid) {
        return dm1_v1_square_byte_pc34(DM1_ELEMENT_WALL, 0);
    }
    return dm1_v1_dungeon_f0151_get_square_pc34(
        column_major_squares, width, height, coords.x, coords.y);
}

int dm1_v1_dungeon_f0153_get_relative_square_type_pc34(
    const uint8_t *column_major_squares,
    int width,
    int height,
    int party_x,
    int party_y,
    int direction,
    int steps_forward,
    int steps_right)
{
    return DM1_SQUARE_TYPE(dm1_v1_dungeon_f0152_get_relative_square_pc34(
        column_major_squares, width, height, party_x, party_y, direction,
        steps_forward, steps_right));
}


/* =======================================================================
 * dm1_compute_view_square_coords
 *
 * Calculate map coordinates for all 12 viewport squares.
 *
 * Viewport-layout (DEFS.H view square diagram):
 *   Depth 3: C=3 forward, L=3 forward + 1 left, R=3 forward + 1 right
 *   Depth 2: C=2 forward, L=2 forward + 1 left, R=2 forward + 1 right
 *   Depth 1: C=1 forward, L=1 forward + 1 left, R=1 forward + 1 right
 *   Depth 0: C=0 forward, L=0 forward + 1 left, R=0 forward + 1 right
 *
 * "Left" = −1 in the rightward direction.
 *
 * Array order: depth 3→0, each depth: C, L, R.
 * Array indices: [0]=D3C, [1]=D3L, [2]=D3R, [3]=D2C, [4]=D2L, [5]=D2R,
 *                  [6]=D1C, [7]=D1L, [8]=D1R, [9]=D0C, [10]=D0L, [11]=D0R
 *
 * View indices (DM1_VS_*): D3C=0, D3L=1, D3R=2, D2C=3, ...
 * ======================================================================= */

/* Internal lookup: depth + lane → (steps_forward, steps_right) */
static const struct {
    int depth;          /* 3, 2, 1, 0 */
    int lane;           /* 0=C, 1=L, 2=R */
    int steps_forward;
    int steps_right;    /* Negative = left */
    int view_index;     /* DM1_VS_* */
} viewport_layout[DM1_VIEWPORT_SQUARE_COUNT] = {
    /* depth 3 */ { 3, 0,  3,  0, DM1_VS_D3C },
                  { 3, 1,  3, -1, DM1_VS_D3L },
                  { 3, 2,  3,  1, DM1_VS_D3R },
    /* depth 2 */ { 2, 0,  2,  0, DM1_VS_D2C },
                  { 2, 1,  2, -1, DM1_VS_D2L },
                  { 2, 2,  2,  1, DM1_VS_D2R },
    /* depth 1 */ { 1, 0,  1,  0, DM1_VS_D1C },
                  { 1, 1,  1, -1, DM1_VS_D1L },
                  { 1, 2,  1,  1, DM1_VS_D1R },
    /* depth 0 */ { 0, 0,  0,  0, DM1_VS_D0C },
                  { 0, 1,  0, -1, DM1_VS_D0L },
                  { 0, 2,  0,  1, DM1_VS_D0R },
};

void dm1_compute_view_square_coords(int party_x, int party_y, int direction,
                                     dm1_view_square_t out_squares[DM1_VIEWPORT_SQUARE_COUNT]) {
    for (int i = 0; i < DM1_VIEWPORT_SQUARE_COUNT; i++) {
        dm1_view_square_t *vs = &out_squares[i];
        vs->depth      = viewport_layout[i].depth;
        vs->lane       = viewport_layout[i].lane;
        vs->view_index = viewport_layout[i].view_index;
        vs->occluded   = false;
        vs->is_front_wall = false;
        memset(vs->aspect, 0, sizeof(vs->aspect));

        dm1_get_relative_map_coords(party_x, party_y, direction,
                                     viewport_layout[i].steps_forward,
                                     viewport_layout[i].steps_right,
                                     &vs->map_x, &vs->map_y);
    }
}


/* =======================================================================
 * dm1_compute_wall_visibility
 *
 * Given a viewport square and party direction, determine which view walls
 * should be drawn.
 *
 * Wall zone-logik (DUNVIEW.C + DEFS.H):
 *
 * A square has a front wall when its element == WALL.
 * Front walls are drawn for that square at the matching depth/lane.
 *
 * Side walls occur when a neighboring square (left/right) is WALL.
 * For example, the D3L right wall (index 0) is drawn when D3L's RIGHT
 * neighbor (= D3C) is NOT a wall but D3L IS a wall.
 *
 * Simplified bitmask approach: report which of the 13 wall positions
 * are affected by this specific square. The full rendering loop iterates
 * over all squares and aggregates the results.
 *
 * Return value: 15-bit bitmask, bit N = view wall index N.
 * D0L/D0R are nearest side-wall planes; they do not center-occlude, but
 * ReDMCSB still draws their side wall bitmaps before returning.
 * ======================================================================= */

uint16_t dm1_compute_wall_visibility(const dm1_view_square_t *square,
                                      int direction) {
    (void)direction;  /* Direction is already baked into the aspect calculation */

    uint16_t mask = 0;
    int elem = square->aspect[DM1_SQA_ELEMENT];

    /* Only WALL and closed FAKEWALL produce walls */
    if (elem != DM1_ELEMENT_WALL) {
        return 0;
    }

    /* Mappa (depth, lane) → relevanta view wall indices */
    switch (square->depth) {
        case 3:
            switch (square->lane) {
                case 0: mask = (1 << DM1_VW_D3C_FRONT); break;
                case 1: mask = (1 << DM1_VW_D3L_FRONT) | (1 << DM1_VW_D3L_RIGHT); break;
                case 2: mask = (1 << DM1_VW_D3R_FRONT) | (1 << DM1_VW_D3R_LEFT); break;
            }
            break;
        case 2:
            switch (square->lane) {
                case 0: mask = (1 << DM1_VW_D2C_FRONT); break;
                case 1: mask = (1 << DM1_VW_D2L_FRONT) | (1 << DM1_VW_D2L_RIGHT); break;
                case 2: mask = (1 << DM1_VW_D2R_FRONT) | (1 << DM1_VW_D2R_LEFT); break;
            }
            break;
        case 1:
            switch (square->lane) {
                case 0: mask = (1 << DM1_VW_D1C_FRONT); break;
                case 1: mask = (1 << DM1_VW_D1L_RIGHT); break;
                case 2: mask = (1 << DM1_VW_D1R_LEFT); break;
            }
            break;
        case 0:
            /* D0 has no center front wall, but nearest side walls are drawn.
             * Source: ReDMCSB DUNVIEW.C F0125/F0126 wall branches
             * draw C716_ZONE_WALL_D0L / C717_ZONE_WALL_D0R and return. */
            switch (square->lane) {
                case 1: mask = (1 << DM1_VW_D0L_SIDE); break;
                case 2: mask = (1 << DM1_VW_D0R_SIDE); break;
            }
            break;
    }

    return mask;
}


/* =======================================================================
 * dm1_classify_square_aspect_element
 *
 * Source-lock: ReDMCSB DUNGEON.C:F0172_DUNGEON_SetSquareAspect
 * (lines 2522-2523 reads raw type; 2628-2648 turns closed pits into
 * corridors; 2651-2664 turns closed fakewalls into walls and open fakewalls
 * into corridors; 2693-2707 maps stairs/doors to side/front aspect elements).
 * ======================================================================= */

int dm1_classify_square_aspect_element(uint8_t raw_byte, int direction) {
    uint8_t element = DM1_SQUARE_TYPE(raw_byte);
    uint8_t flags = raw_byte & 0x0F;
    int oriented_west_east = direction & 1;

    switch (element) {
        case DM1_ELEMENT_PIT:
            return (flags & DM1_PIT_OPEN) ? DM1_ELEMENT_PIT : DM1_ELEMENT_CORRIDOR;

        case DM1_ELEMENT_FAKEWALL:
            return (flags & DM1_FAKEWALL_OPEN) ? DM1_ELEMENT_CORRIDOR : DM1_ELEMENT_WALL;

        case DM1_ELEMENT_STAIRS:
            return (((flags & DM1_STAIRS_NS_ORIENTATION) >> 3) == oriented_west_east)
                ? DM1_ELEMENT_STAIRS_SIDE
                : DM1_ELEMENT_STAIRS_FRONT;

        case DM1_ELEMENT_DOOR:
            return (((flags & DM1_DOOR_NS_ORIENTATION) >> 3) == oriented_west_east)
                ? DM1_ELEMENT_DOOR_SIDE
                : DM1_ELEMENT_DOOR_FRONT;

        default:
            return element;
    }
}



/* =======================================================================
 * dm1_get_pc34_extra_side_wall_coords / dm1_compute_pc34_extra_side_wall_visibility
 *
 * Source-lock: ReDMCSB DUNVIEW.C MEDIA720/I34E adds supplemental far side
 * wall planes outside the 12-square core.  F0128 calls D3L2/D3R2 after the
 * D4 object pass (DUNVIEW.C:8479-8486) and D2L2/D2R2 before D2L/D2R/D2C
 * (DUNVIEW.C:8501-8508).  The helpers themselves draw only when F0172
 * classifies the square aspect as WALL (DUNVIEW.C:6253-6264,6320-6331,
 * 6846-6862,6877-6893).
 * ======================================================================= */

bool dm1_get_pc34_extra_side_wall_coords(int party_x, int party_y, int direction,
                                          int depth, int steps_right,
                                          int *out_x, int *out_y) {
    if (!(((depth == 3) || (depth == 2)) &&
          ((steps_right == -2) || (steps_right == 2)))) {
        return false;
    }
    dm1_get_relative_map_coords(party_x, party_y, direction, depth, steps_right, out_x, out_y);
    return true;
}

uint8_t dm1_compute_pc34_extra_side_wall_visibility(int depth, int steps_right,
                                                    uint8_t raw_byte, int direction) {
    int element;

    if (!(((depth == 3) || (depth == 2)) &&
          ((steps_right == -2) || (steps_right == 2)))) {
        return 0;
    }

    element = dm1_classify_square_aspect_element(raw_byte, direction);
    if (element != DM1_ELEMENT_WALL) {
        return 0;
    }

    if (depth == 3) {
        return (uint8_t)(1u << ((steps_right < 0) ? DM1_PC34_EXTRA_WALL_D3L2 : DM1_PC34_EXTRA_WALL_D3R2));
    }
    return (uint8_t)(1u << ((steps_right < 0) ? DM1_PC34_EXTRA_WALL_D2L2 : DM1_PC34_EXTRA_WALL_D2R2));
}

/* =======================================================================
 * dm1_build_viewport
 *
 * Huvudfunktion: bygg komplett viewport-state.
 *
 * Algorithm (derived from the DUNVIEW.C viewport draw loop):
 *
 *   1. Calculate map coordinates for all 12 viewport squares.
 *   2. For each square, read the raw square byte through the callback.
 *   3. Decode the square byte → populate aspect[0] (element).
 *      (Full SetSquareAspect processing with ornaments requires thing-list access,
 *       which is handled by the consumer; here we set the element + basic flags.)
 *   4. Occlusion calculation: iterate depth 3→0.
 *      If the center square at depth D has element == WALL:
 *        → depth_occluded[D+1..3] = true
 *        → mark all squares at depth > D as occluded = true.
 *
 * IMPORTANT: The DM1 V1 occlusion model (DRAWVIEW.C) is:
 *   - A front wall in the CENTER lane at depth D blocks rendering
 *     of everything at depth D+1, D+2, and D+3.
 *   - Side walls in the L/R lanes do NOT occlude the squares behind them
 *     (only center-lane front walls fully occlude).
 *   - Closed fakewalls behave like walls (occlusion).
 *   - Doors with state >= 2 (half-closed to closed) are drawn as
 *     front-wall elements but do NOT fully occlude
 *     (you can see through/over them).
 * ======================================================================= */

void dm1_build_viewport(int party_x, int party_y, int direction, int party_map,
                         dm1_square_reader_fn reader, void *user_data,
                         dm1_viewport_state_t *out) {
    memset(out, 0, sizeof(*out));

    out->party_x   = party_x;
    out->party_y   = party_y;
    out->party_dir = direction;
    out->party_map = party_map;

    /* Step 1: Calculate coordinates */
    dm1_compute_view_square_coords(party_x, party_y, direction, out->squares);
    out->valid_count = DM1_VIEWPORT_SQUARE_COUNT;

    /* Steps 2–3: Read and decode each square */
    for (int i = 0; i < DM1_VIEWPORT_SQUARE_COUNT; i++) {
        dm1_view_square_t *vs = &out->squares[i];
        uint8_t raw = reader(vs->map_x, vs->map_y, user_data);

        /* Populate the basic aspect according to F0172. */
        int element = dm1_classify_square_aspect_element(raw, direction);

        vs->aspect[DM1_SQA_ELEMENT] = (int16_t)element;
        vs->is_front_wall = (element == DM1_ELEMENT_WALL);
        if (element == DM1_ELEMENT_PIT) {
            vs->aspect[DM1_SQA_PIT_TELEPORTER_VIS] = (raw & DM1_PIT_INVISIBLE) ? 1 : 0;
        } else if (element == DM1_ELEMENT_TELEPORTER) {
            vs->aspect[DM1_SQA_PIT_TELEPORTER_VIS] =
                ((raw & DM1_TELEPORTER_OPEN) && (raw & DM1_TELEPORTER_VISIBLE)) ? 1 : 0;
        } else if (element == DM1_ELEMENT_STAIRS_SIDE || element == DM1_ELEMENT_STAIRS_FRONT) {
            vs->aspect[DM1_SQA_STAIRS_UP] = (raw & DM1_STAIRS_UP) ? 1 : 0;
        } else if (element == DM1_ELEMENT_DOOR_SIDE || element == DM1_ELEMENT_DOOR_FRONT) {
            vs->aspect[DM1_SQA_DOOR_STATE] = raw & DM1_DOOR_STATE_MASK;
        }
    }

    /* Step 4: Occlusion calculation — depth 0→3 (nearest → farthest).
     *
     * Logic: if the center square at depth D is a front wall,
     * occlude all squares at depth D+1, D+2, and D+3.
     *
     * Iterate depth 0→3 and mark occlusion progressively.
     * (DRAWVIEW.C iterates 3→0 and skips rendering; here we mark occlusion instead.)
     */
    bool occluded_beyond[DM1_DEPTH_ZONE_COUNT + 1];
    memset(occluded_beyond, 0, sizeof(occluded_beyond));

    /* Find center squares at each depth and determine occlusion.
     * In this array layout, the center square at depth D has index = (3-D)*3.
     * depth 3 → index 0, depth 2 → index 3, depth 1 → index 6, depth 0 → index 9.
     */
    for (int d = 0; d <= DM1_VISIBLE_DEPTH_MAX; d++) {
        int center_idx = (DM1_VISIBLE_DEPTH_MAX - d) * 3;  /* D3→0, D2→3, D1→6, D0→9 */

        if (out->squares[center_idx].is_front_wall) {
            /* Everything at depth > d is occluded (farther away) */
            for (int d2 = d + 1; d2 <= DM1_VISIBLE_DEPTH_MAX; d2++) {
                occluded_beyond[d2] = true;
                out->depth_occluded[d2] = true;
            }
        }
    }

    /* Apply occlusion flags to individual squares */
    for (int i = 0; i < DM1_VIEWPORT_SQUARE_COUNT; i++) {
        int d = out->squares[i].depth;
        if (occluded_beyond[d]) {
            out->squares[i].occluded = true;
        }
    }
}


/* =======================================================================
 * dm1_is_front_wall_at_depth
 * ======================================================================= */

bool dm1_is_front_wall_at_depth(const dm1_viewport_state_t *vp, int depth) {
    if (depth < 0 || depth > DM1_VISIBLE_DEPTH_MAX) return false;

    /* Center square at this depth: index = (3-depth)*3 */
    int center_idx = (DM1_VISIBLE_DEPTH_MAX - depth) * 3;
    return vp->squares[center_idx].is_front_wall;
}


/* =======================================================================
 * dm1_get_visible_squares
 *
 * Returns indices of non-occluded squares, ordered back-to-front
 * (depth 3→0, each depth: L, R, C, matching DUNVIEW.C:8490-8542).
 * This order matches the DM1 rendering order (DRAWVIEW.C).
 * ======================================================================= */

int dm1_get_visible_squares(const dm1_viewport_state_t *vp,
                             int visible_indices[DM1_VIEWPORT_SQUARE_COUNT]) {
    static const int draw_order[DM1_VIEWPORT_SQUARE_COUNT] = {
        1, 2, 0,   /* D3L, D3R, D3C */
        4, 5, 3,   /* D2L, D2R, D2C */
        7, 8, 6,   /* D1L, D1R, D1C */
        10, 11, 9  /* D0L, D0R, D0C */
    };
    int count = 0;

    for (int o = 0; o < DM1_VIEWPORT_SQUARE_COUNT; o++) {
        int i = draw_order[o];
        if (!vp->squares[i].occluded) {
            visible_indices[count++] = i;
        }
    }
    return count;
}


/* =======================================================================
 * dm1_square_blocks_movement
 *
 * Quick check: does the square byte block movement?
 *
 * Based on CLIKMENU.C:F0366 (lines 274-290):
 *   WALL → blocked
 *   DOOR → state >= 2 and != 5 (destroyed) → blocked
 *   FAKEWALL → !open && !imaginary → blocked
 *   Everything else (CORRIDOR, PIT, STAIRS, TELEPORTER) → passable
 *
 * NOTE: Pit falls and creature blocking are handled by other modules
 * (dm1_v1_collision_door_pc34_compat, dm1_v1_movement_pipeline_pc34_compat).
 * ======================================================================= */

bool dm1_square_blocks_movement(uint8_t raw_byte) {
    uint8_t element = DM1_SQUARE_TYPE(raw_byte);
    uint8_t flags   = raw_byte & 0x0F;

    switch (element) {
        case DM1_ELEMENT_WALL:
            return true;

        case DM1_ELEMENT_DOOR: {
            uint8_t state = flags & DM1_DOOR_STATE_MASK;
            /* Passable if state < 2 (open / one-fourth) or == 5 (destroyed) */
            return (state >= 2) && (state != 5);
        }

        case DM1_ELEMENT_FAKEWALL:
            /* Passable if open OR imaginary */
            if (flags & DM1_FAKEWALL_OPEN)      return false;
            if (flags & DM1_FAKEWALL_IMAGINARY)  return false;
            return true;

        default:
            return false;
    }
}


/* =======================================================================
 * dm1_viewport_uses_flipped_wall_and_footprints
 *
 * Source-lock: ReDMCSB DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF line 8357.
 * The parity flag drives flipped wall/floor-set selection in F0128 and
 * center-lane footprint floor ornament flipping in F0108 lines 3967-3980.
 * ======================================================================= */

bool dm1_viewport_uses_flipped_wall_and_footprints(int map_x, int map_y, int direction) {
    return ((map_x + map_y + (direction & 3)) & 1) != 0;
}
