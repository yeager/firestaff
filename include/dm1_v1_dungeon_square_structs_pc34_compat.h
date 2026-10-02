#ifndef DM1_V1_DUNGEON_SQUARE_STRUCTS_PC34_COMPAT_H
#define DM1_V1_DUNGEON_SQUARE_STRUCTS_PC34_COMPAT_H

/*
 * DM1 V1 Dungeon Square Data Structures — Wall Zones, Depth Zones, Occlusion
 * ============================================================================
 *
 * Geometric foundation for viewport rendering and movement logic.
 * Defines how dungeon squares are stored in memory, which walls are
 * visible at each depth, and occlusion logic (front walls block).
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *
 *   DEFS.H — All structure definitions, square-type macros, element enum,
 *            wall masks, thing-type enum, cell/direction enum, view-square
 *            and view-wall index tables, and square-aspect accessors.
 *
 *   DUNGEON.C — F0151_DUNGEON_GetSquare (lines 1423–1475):
 *     Returns one byte per square; bits 7–5 = element type, bits 4–0 = flags.
 *     Bounds check: outside the map → returns C00_ELEMENT_WALL with
 *     direction-specific random-ornament flags.
 *
 *   DUNGEON.C — F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement:
 *     Coordinate transform: direction + forward/right steps → absolute (X,Y).
 *     Uses G0233_ai_DirectionToStepEastCount[4] and
 *     G0234_ai_DirectionToStepNorthCount[4].
 *
 *   DUNGEON.C — F0172_DUNGEON_SetSquareAspect (lines 2466–2721):
 *     Calculates square aspect[5/7 ints] for each viewport square:
 *     [0]=element, [1]=firstThing, [2..4]=wallOrnamentOrdinals (R/F/L),
 *     [4]=floorOrnamentOrdinal, etc. Handles WALL, PIT, CORRIDOR,
 *     FAKEWALL, TELEPORTER, STAIRS, DOOR.
 *
 *   DUNVIEW.C — F0128_DUNGEONVIEW_DrawDungeon (lines 8445–8542): Viewport draw loop. Iterates depth 3→0,
 *     lane C/L/R. At each depth F0172 is called for the square aspect;
 *     a front wall (element==WALL) at depth D occludes everything behind it.
 *     Walls are drawn using indices M575..M587 (13/15 view-wall positions).
 *
 *   DRAWVIEW.C — F0093_DUNGEONVIEW_DrawDungeon:
 *     Master loop that iterates depth 3→0, lane center/left/right.
 *     Calls helper functions for floor/ceiling, walls, doors, objects,
 *     creatures, projectiles, explosions.
 *
 *   COORD.C — Coordinate helper functions; offset calculations per map.
 *
 * Viewport-geometri (DM1 V1 = Atari ST / Amiga versions 1.x/2.x):
 *
 *   224×136 pixel viewport (112 bytes wide, 136 rows).
 *   Depth 0 = squares at the party; Depth 3 = farthest away.
 *   Three "lanes" per depth: Center, Left, Right.
 *   Total: 12 visible squares + 3 at depth 4 (data exists but is not drawn).
 *
 *   Visible squares (party at D0C):
 *     D4L(-2)  D4C(-3)  D4R(-1)
 *     D3L(1)   D3C(0)   D3R(2)
 *     D2L(4)   D2C(3)   D2R(5)
 *     D1L(7)   D1C(6)   D1R(8)
 *     D0L(10)  D0C(9)   D0R(11)
 *
 *   See the DEFS.H view-square diagram and M597..M611 index definitions.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * § 1. Square-byte layout (DEFS.H M034_SQUARE_TYPE / M035_SQUARE)
 * =======================================================================
 * Each square in the map is stored as ONE byte:
 *   Bits 7-5: element type (0..6)
 *   Bit  4:   thing-list present (MASK0x0010_THING_LIST_PRESENT)
 *   Bits 3-0: type-specific flags
 */

/* --- Element-typer (square types) --- */
#define DM1_ELEMENT_WALL         0
#define DM1_ELEMENT_CORRIDOR     1
#define DM1_ELEMENT_PIT          2
#define DM1_ELEMENT_STAIRS       3
#define DM1_ELEMENT_DOOR         4
#define DM1_ELEMENT_TELEPORTER   5
#define DM1_ELEMENT_FAKEWALL     6

/* Extended element types (square aspect, not stored in the map) */
#define DM1_ELEMENT_DOOR_SIDE    16
#define DM1_ELEMENT_DOOR_FRONT   17
#define DM1_ELEMENT_STAIRS_SIDE  18
#define DM1_ELEMENT_STAIRS_FRONT 19

/* --- Square-byte extraktion --- */
#define DM1_SQUARE_TYPE(sq)       ((sq) >> 5)
#define DM1_SQUARE_FLAGS(sq)      ((sq) & 0x1F)
#define DM1_SQUARE_HAS_THINGS(sq) (((sq) & 0x10) != 0)

/* --- Wall-specific flags (bits 3-0 for element type WALL) --- */
#define DM1_WALL_WEST_RANDOM_ORN   0x01  /* MASK0x0001 */
#define DM1_WALL_SOUTH_RANDOM_ORN  0x02  /* MASK0x0002 */
#define DM1_WALL_EAST_RANDOM_ORN   0x04  /* MASK0x0004 */
#define DM1_WALL_NORTH_RANDOM_ORN  0x08  /* MASK0x0008 */

/* --- Corridor-specific --- */
#define DM1_CORRIDOR_RANDOM_ORN    0x08  /* MASK0x0008 */

/* --- Pit-specific --- */
#define DM1_PIT_IMAGINARY          0x01  /* MASK0x0001 */
#define DM1_PIT_INVISIBLE          0x04  /* MASK0x0004 */
#define DM1_PIT_OPEN               0x08  /* MASK0x0008 */

/* --- Stairs-specific --- */
#define DM1_STAIRS_UP              0x04  /* MASK0x0004 */
#define DM1_STAIRS_NS_ORIENTATION  0x08  /* MASK0x0008 */

/* --- Door-specific --- */
#define DM1_DOOR_NS_ORIENTATION    0x08  /* MASK0x0008 */
#define DM1_DOOR_STATE_MASK        0x07  /* Low 3 bits */

/* --- Teleporter-specific --- */
#define DM1_TELEPORTER_VISIBLE     0x04  /* MASK0x0004 */
#define DM1_TELEPORTER_OPEN        0x08  /* MASK0x0008 */

/* --- Fakewall-specific --- */
#define DM1_FAKEWALL_IMAGINARY     0x01  /* MASK0x0001 */
#define DM1_FAKEWALL_OPEN          0x04  /* MASK0x0004 */
#define DM1_FAKEWALL_RANDOM_ORN    0x08  /* MASK0x0008 */

/* --- Thing-list present (shared) --- */
#define DM1_THING_LIST_PRESENT     0x10  /* MASK0x0010 */


/* =======================================================================
 * § 2. Directions and Cells (DEFS.H)
 * ======================================================================= */
#define DM1_DIR_NORTH  0
#define DM1_DIR_EAST   1
#define DM1_DIR_SOUTH  2
#define DM1_DIR_WEST   3

#define DM1_CELL_NORTHWEST  0
#define DM1_CELL_NORTHEAST  1
#define DM1_CELL_SOUTHEAST  2
#define DM1_CELL_SOUTHWEST  3

#define DM1_DIR_NEXT(d)     (((d) + 1) & 3)
#define DM1_DIR_OPPOSITE(d) (((d) + 2) & 3)
#define DM1_DIR_PREV(d)     (((d) + 3) & 3)


/* =======================================================================
 * § 3. Depth Zones — Viewport Square Grid
 * =======================================================================
 * DM1 V1 (Atari ST / Amiga 2.x) viewport: 5 rows, 3 columns.
 * Depth = number of steps forward from the party:
 *   Depth 0: D0C (party square), D0L (left), D0R (right)
 *   Depth 1: D1C, D1L, D1R
 *   Depth 2: D2C, D2L, D2R
 *   Depth 3: D3C, D3L, D3R
 *   Depth 4: D4C, D4L, D4R (data exists but is normally not drawn)
 *
 * DEFS.H defines view-square indices (versions 1.x/2.x):
 *   D4C=-3, D4L=-2, D4R=-1,
 *   D3C=0, D3L=1, D3R=2, D2C=3, D2L=4, D2R=5,
 *   D1C=6, D1L=7, D1R=8, D0C=9, D0L=10, D0R=11
 */

/* View square index (DM1 V1 layout) */
#define DM1_VS_D4C  (-3)
#define DM1_VS_D4L  (-2)
#define DM1_VS_D4R  (-1)
#define DM1_VS_D3C    0
#define DM1_VS_D3L    1
#define DM1_VS_D3R    2
#define DM1_VS_D2C    3
#define DM1_VS_D2L    4
#define DM1_VS_D2R    5
#define DM1_VS_D1C    6
#define DM1_VS_D1L    7
#define DM1_VS_D1R    8
#define DM1_VS_D0C    9
#define DM1_VS_D0L   10
#define DM1_VS_D0R   11

#define DM1_VIEW_SQUARE_COUNT  15  /* D4L..D0R = 15 positions (including D4) */
#define DM1_VISIBLE_DEPTH_MAX   3  /* Maximum renderable depth (depth 0..3) */
#define DM1_DEPTH_ZONE_COUNT    4  /* Depth 0, 1, 2, 3 */
#define DM1_LANES_PER_DEPTH     3  /* Center, Left, Right */


/* =======================================================================
 * § 4. View Wall Indices (DEFS.H M575..M587)
 * =======================================================================
 * DM1 V1 has 13 view-wall positions:
 *   D3L right(0), D3R left(1),
 *   D3L front(2), D3C front(3), D3R front(4),
 *   D2L right(5), D2R left(6),
 *   D2L front(7), D2C front(8), D2R front(9),
 *   D1L right(10), D1R left(11),
 *   D1C front(12)
 *
 * A "front wall" at depth D blocks everything behind it (occlusion).
 * Side walls (right/left) bound the lane edges.
 */
#define DM1_VW_D3L_RIGHT   0
#define DM1_VW_D3R_LEFT    1
#define DM1_VW_D3L_FRONT   2
#define DM1_VW_D3C_FRONT   3
#define DM1_VW_D3R_FRONT   4
#define DM1_VW_D2L_RIGHT   5
#define DM1_VW_D2R_LEFT    6
#define DM1_VW_D2L_FRONT   7
#define DM1_VW_D2C_FRONT   8
#define DM1_VW_D2R_FRONT   9
#define DM1_VW_D1L_RIGHT  10
#define DM1_VW_D1R_LEFT   11
#define DM1_VW_D1C_FRONT  12
#define DM1_VW_D0L_SIDE   13
#define DM1_VW_D0R_SIDE   14

#define DM1_VIEW_WALL_COUNT_V1  15

/* PC34/I34E supplemental far side wall planes drawn outside the 12-square core.
 * ReDMCSB MEDIA720 adds D3L2/D3R2 and D2L2/D2R2 draw calls around the
 * normal D3/D2 rows; these are tracked separately so the legacy 15-bit
 * view-wall mask remains stable for existing callers. */
#define DM1_PC34_EXTRA_WALL_D3L2 0
#define DM1_PC34_EXTRA_WALL_D3R2 1
#define DM1_PC34_EXTRA_WALL_D2L2 2
#define DM1_PC34_EXTRA_WALL_D2R2 3

#define DM1_PC34_EXTRA_WALL_COUNT 4

/* View-cell indices (for object/creature rendering within a square) */
#define DM1_VCELL_FRONT_LEFT   0
#define DM1_VCELL_FRONT_RIGHT  1
#define DM1_VCELL_BACK_RIGHT   2
#define DM1_VCELL_BACK_LEFT    3
#define DM1_VCELL_ALCOVE       4
#define DM1_VCELL_DOOR_BUTTON  5

/* View floor indices */
#define DM1_VF_D3L   0
#define DM1_VF_D3C   1
#define DM1_VF_D3R   2
#define DM1_VF_D2L   3
#define DM1_VF_D2C   4
#define DM1_VF_D2R   5
#define DM1_VF_D1L   6
#define DM1_VF_D1C   7
#define DM1_VF_D1R   8


/* =======================================================================
 * § 5. Square Aspect (runtime per-square visibility data)
 * =======================================================================
 * F0172_DUNGEON_SetSquareAspect builds an int16_t array for each visible square.
 * DM1 V1 (versions 1.x/2.x): 5 elements.
 * Indices (DEFS.H M550..M559):
 */
#define DM1_SQA_ELEMENT               0  /* C0_ELEMENT */
#define DM1_SQA_FIRST_THING           1  /* M550: THING index, or ENDOFLIST */
#define DM1_SQA_RIGHT_WALL_ORN_ORD    2  /* M551: right wall ornament ordinal */
#define DM1_SQA_FRONT_WALL_ORN_ORD    3  /* M552: front wall ornament ordinal */
#define DM1_SQA_LEFT_WALL_ORN_ORD     4  /* M553: left wall ornament ordinal */
/* For non-WALL squares, indices 2..4 overlap: */
#define DM1_SQA_PIT_TELEPORTER_VIS    2  /* M554: visible pit/teleporter */
#define DM1_SQA_STAIRS_UP             2  /* M555 */
#define DM1_SQA_DOOR_STATE            2  /* M556 */
#define DM1_SQA_DOOR_THING_INDEX      3  /* M557 */
#define DM1_SQA_FLOOR_ORN_ORDINAL     4  /* M558 */

#define DM1_SQA_COUNT_V1              5  /* M559: number of int16_t values in aspect array */

#define DM1_FOOTPRINTS_MASK  0x8000  /* MASK0x8000_FOOTPRINTS i floor orn */


/* =======================================================================
 * § 6. Direction-to-Step Lookup Tables
 * =======================================================================
 * From DUNGEON.C G0233/G0234. Converts direction → (dX, dY).
 *
 *   Direction  EastCount  NorthCount
 *   North(0)      0          -1
 *   East(1)       1           0
 *   South(2)      0           1
 *   West(3)      -1           0
 */

/* Dessa tabeller deklareras i .c-filen */
extern const int dm1_direction_to_step_east[4];
extern const int dm1_direction_to_step_north[4];


/* =======================================================================
 * § 7. In-memory Dungeon Square Struct (our representation)
 * =======================================================================
 * In the original, each square is stored as ONE byte (element|flags).
 * Thing lists are stored separately in G0283_pT_SquareFirstThings.
 * This struct holds the decoded information for each square.
 */

typedef struct {
    uint8_t  raw_byte;         /* Original byte: bits 7-5 = element, bits 4-0 = flags */

    /* Decoded fields */
    uint8_t  element;          /* DM1_ELEMENT_WALL..DM1_ELEMENT_FAKEWALL (0..6) */
    bool     has_thing_list;   /* Bit 4 of raw_byte */

    /* Type-specific fields (union, depends on element) */
    union {
        struct {
            bool west_random_orn;   /* Bit 0: west wall allows random ornament */
            bool south_random_orn;  /* Bit 1 */
            bool east_random_orn;   /* Bit 2 */
            bool north_random_orn;  /* Bit 3 */
        } wall;

        struct {
            bool random_orn_allowed; /* Bit 3 */
        } corridor;

        struct {
            bool imaginary;   /* Bit 0: imaginary pit, no fall */
            bool invisible;   /* Bit 2: graphically invisible */
            bool open;        /* Bit 3: open → fall through */
        } pit;

        struct {
            bool up;               /* Bit 2: stairs go up */
            bool ns_orientation;   /* Bit 3: north-south orientation */
        } stairs;

        struct {
            uint8_t state;         /* Bits 2-0: door state (0..5) */
            bool    ns_orientation; /* Bit 3 */
        } door;

        struct {
            bool visible;   /* Bit 2 */
            bool open;      /* Bit 3 */
        } teleporter;

        struct {
            bool imaginary;         /* Bit 0 */
            bool open;              /* Bit 2 */
            bool random_orn_allowed; /* Bit 3 */
        } fakewall;
    } flags;

} dm1_dungeon_square_t;


/* =======================================================================
 * § 8. View Square Descriptor (per visible square in the viewport)
 * ======================================================================= */

typedef struct {
    int map_x;         /* Absolute map X */
    int map_y;         /* Absolute map Y */
    int depth;         /* 0..3 (distance from party) */
    int lane;          /* 0=Center, 1=Left, 2=Right */
    int view_index;    /* DM1_VS_D0C..DM1_VS_D3R (view square index) */

    /* Square aspect (calculated at runtime, see § 5) */
    int16_t aspect[DM1_SQA_COUNT_V1];

    /* Occlusion */
    bool occluded;          /* true if a front wall at a nearer depth blocks this square */
    bool is_front_wall;     /* true if this square IS a front wall (element==WALL) */
} dm1_view_square_t;


/* =======================================================================
 * § 9. Viewport State (all visible squares + occlusion)
 * ======================================================================= */

/* Maximum number of squares in a complete viewport (depth 0..3, 3 lanes) */
#define DM1_VIEWPORT_SQUARE_COUNT  12  /* 4 depths × 3 lanes */

typedef struct {
    /* Party position */
    int party_x;
    int party_y;
    int party_dir;   /* DM1_DIR_NORTH..DM1_DIR_WEST */
    int party_map;   /* Map index */

    /* The 12 visible squares (depth 0..3, lane C/L/R) */
    dm1_view_square_t squares[DM1_VIEWPORT_SQUARE_COUNT];

    /* Occlusion flags per depth (true = a front wall blocks this depth and beyond) */
    bool depth_occluded[DM1_DEPTH_ZONE_COUNT];

    /* Number of valid squares */
    int valid_count;
} dm1_viewport_state_t;


/* =======================================================================
 * § 10. API — Funktionsdeklarationer
 * ======================================================================= */

/*
 * dm1_decode_square — Decode a raw square byte into dm1_dungeon_square_t.
 * Corresponds to the logic in F0151_DUNGEON_GetSquare (DUNGEON.C:937) +
 * decoding in F0172_DUNGEON_SetSquareAspect (DUNGEON.C:1170).
 */
void dm1_decode_square(uint8_t raw_byte, dm1_dungeon_square_t *out);

/*
 * dm1_get_relative_map_coords — Given party position + direction + steps,
 * calculate absolute (X,Y). Corresponds to F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement.
 * Parameters: steps_forward (positive = forward), steps_right (positive = right).
 */
void dm1_get_relative_map_coords(int party_x, int party_y, int direction,
                                  int steps_forward, int steps_right,
                                  int *out_x, int *out_y);

typedef struct {
    int valid;
    int x;
    int y;
} DM1_V1_DungeonF0150CoordinatesPc34;

int dm1_v1_dungeon_f0150_update_map_coordinates_after_relative_movement_pc34(
    int party_x,
    int party_y,
    int direction,
    int steps_forward,
    int steps_right,
    DM1_V1_DungeonF0150CoordinatesPc34 *out);

uint8_t dm1_v1_dungeon_f0151_get_square_pc34(
    const uint8_t *column_major_squares,
    int width,
    int height,
    int map_x,
    int map_y);

uint8_t dm1_v1_dungeon_f0152_get_relative_square_pc34(
    const uint8_t *column_major_squares,
    int width,
    int height,
    int party_x,
    int party_y,
    int direction,
    int steps_forward,
    int steps_right);

int dm1_v1_dungeon_f0153_get_relative_square_type_pc34(
    const uint8_t *column_major_squares,
    int width,
    int height,
    int party_x,
    int party_y,
    int direction,
    int steps_forward,
    int steps_right);

/*
 * dm1_compute_view_square_coords — Calculate map coordinates for all
 * 12 (or 15 including D4) viewport squares given party position and direction.
 * Populates map_x, map_y, depth, lane, and view_index for each square.
 *
 * Geometry is derived from the DUNVIEW.C viewport loop and
 * the DUNGEON.C:F0150 coordinate transform.
 *
 *   D3: 3 steps forward, lane: 0/−1/+1 right
 *   D2: 2 steps forward, lane: 0/−1/+1 right
 *   D1: 1 step forward, lane: 0/−1/+1 right
 *   D0: 0 steps forward, lane: 0/−1/+1 right
 */
void dm1_compute_view_square_coords(int party_x, int party_y, int direction,
                                     dm1_view_square_t out_squares[DM1_VIEWPORT_SQUARE_COUNT]);

/*
 * dm1_compute_wall_visibility — Given a viewport square's square aspect,
 * determine which view walls (indices 0..12) should be drawn.
 *
 * Logik (DUNVIEW.C):
 * - A WALL square at depth D generates a front wall → occludes depths D+1..3.
 * - Side walls (right/left) are drawn at the boundary with a WALL square in the neighboring lane.
 * - Front walls are drawn ONLY if the square IN FRONT (toward the party) is NOT a wall.
 *
 * Returns a 15-bit bitmask: bit N = view wall N should be drawn.
 */
uint16_t dm1_compute_wall_visibility(const dm1_view_square_t *square,
                                      int direction);

/*
 * dm1_get_pc34_extra_side_wall_coords / dm1_compute_pc34_extra_side_wall_visibility
 * — source-lock the I34E/PC34 supplemental far side wall planes: D3L2, D3R2,
 * D2L2, D2R2.  Return false/0 for non-PC34 supplemental coordinates.
 */
bool dm1_get_pc34_extra_side_wall_coords(int party_x, int party_y, int direction,
                                          int depth, int steps_right,
                                          int *out_x, int *out_y);

uint8_t dm1_compute_pc34_extra_side_wall_visibility(int depth, int steps_right,
                                                    uint8_t raw_byte, int direction);


/*
 * dm1_classify_square_aspect_element — Source-locked F0172 element
 * classification for one square in viewport context.  Converts raw stored
 * square types to runtime square-aspect element values: closed pits become
 * corridors, closed fakewalls become walls, and doors/stairs become side or
 * front elements according to orientation versus party direction.
 */
int dm1_classify_square_aspect_element(uint8_t raw_byte, int direction);

/*
 * dm1_build_viewport — Main function: build the complete viewport state.
 *
 * Given party position, direction, and a callback that returns
 * the raw square byte for a given (mapX, mapY), build a viewport with:
 * 1. Map coordinates for each visible square
 * 2. Square aspect for each square (via callback + dm1_decode_square)
 * 3. Occlusion calculation: depth 3→0, front walls block behind them
 *
 * square_reader: callback(map_x, map_y, user_data) → raw square byte.
 *                If outside the map: return (DM1_ELEMENT_WALL << 5).
 */
typedef uint8_t (*dm1_square_reader_fn)(int map_x, int map_y, void *user_data);

void dm1_build_viewport(int party_x, int party_y, int direction, int party_map,
                         dm1_square_reader_fn reader, void *user_data,
                         dm1_viewport_state_t *out);

/*
 * dm1_is_front_wall_at_depth — Returns true if a front wall exists
 * at the specified depth in the center lane. Used for a simple occlusion check.
 */
bool dm1_is_front_wall_at_depth(const dm1_viewport_state_t *vp, int depth);

/*
 * dm1_get_visible_squares — Returns the number of non-occluded squares.
 * Fills visible_indices[] with indices into vp->squares[].
 * Order: depth 3→0 (back-to-front), per DUNVIEW draw row: left→right→center.
 */
int dm1_get_visible_squares(const dm1_viewport_state_t *vp,
                             int visible_indices[DM1_VIEWPORT_SQUARE_COUNT]);

/*
 * dm1_square_blocks_movement — Determines whether a square byte blocks movement.
 * Coordinates with dm1_v1_collision_door_pc34_compat but is a standalone
 * quick check based only on square-byte data.
 *
 * Blockering (CLIKMENU.C:F0366, DUNGEON.C:F0151):
 *   WALL: always blocked
 *   DOOR: state >= 2 and != 5 → blocked
 *   FAKEWALL: !open && !imaginary → blocked
 *   Everything else: passable (pit/corridor/stairs/teleporter)
 */
bool dm1_square_blocks_movement(uint8_t raw_byte);

/*
 * dm1_viewport_uses_flipped_wall_and_footprints — ReDMCSB DUNVIEW.C:F0128
 * parity switch for alternate wall/floor rendering.  Source line 8357 assigns
 * G0076_B_UseFlippedWallAndFootprintsBitmaps = (mapX + mapY + direction) & 1;
 * F0108 then reuses that flag for center-lane footprint floor ornaments
 * (DUNVIEW.C:3967-3980), while wall and floor blits use it throughout
 * F0128/F0122-F0127.
 */
bool dm1_viewport_uses_flipped_wall_and_footprints(int map_x, int map_y, int direction);


#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_DUNGEON_SQUARE_STRUCTS_PC34_COMPAT_H */
