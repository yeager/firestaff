/*
 * DM1 V1 Dungeon Square Data Structures — Implementation
 * ========================================================
 *
 * Implementerar square-dekodning, viewport-koordinatberäkning,
 * depth zone-traversering, wall zone-bestämning och ocklusionslogik.
 *
 * Source-referens (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *
 *   DUNGEON.C:
 *     G0233_ai_Graphic559_DirectionToStepEastCount[4]   (rad 30-34)
 *     G0234_ai_Graphic559_DirectionToStepNorthCount[4]  (rad 35-39)
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
 *   DUNVIEW.C (rad 700+):
 *     Viewport-ritningsloop: depth 3→0, per lane, anropar
 *     F0172 SetSquareAspect. Front walls (element==WALL i center-lane)
 *     ockluderar allt bakom dem.
 *
 *   DRAWVIEW.C:
 *     F0093_DUNGEONVIEW_DrawDungeon — master viewport draw.
 */

#include "dm1_v1_dungeon_square_structs_pc34_compat.h"
#include <string.h>


/* =======================================================================
 * Direction-to-Step Lookup Tables
 * (DUNGEON.C rad 30-39: G0233, G0234)
 *
 *   Karta: X ökar österut, Y ökar söderut (standard DM-konvention).
 *   North = (0, -1), East = (1, 0), South = (0, 1), West = (-1, 0).
 * ======================================================================= */

const int dm1_direction_to_step_east[4] = {
     0,   /* North */
     1,   /* East */
     0,   /* South */
    -1    /* West */
};

const int dm1_direction_to_step_north[4] = {
    -1,   /* North: Y minskar */
     0,   /* East */
     1,   /* South: Y ökar */
     0    /* West */
};


/* =======================================================================
 * dm1_decode_square
 *
 * Dekoderar en raw square-byte (som returneras av F0151_DUNGEON_GetSquare)
 * till strukturerade fält.
 *
 * ReDMCSB-referens:
 *   DEFS.H — M034_SQUARE_TYPE, alla MASK-definitioner per element-typ.
 *   DUNGEON.C:F0172 — Hur fält tolkas per square type.
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
            /* Okänd typ — lämna nollat */
            break;
    }
}


/* =======================================================================
 * dm1_get_relative_map_coords
 *
 * Motsvarar F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement
 * (DUNGEON.C rad 867-935).
 *
 * Givet party-position (X,Y), riktning, steg framåt och steg höger,
 * beräkna absolut (out_x, out_y).
 *
 * Algoritm (direkt från källkoden):
 *   1. Applicera steg framåt i partyts riktning:
 *      X += east_count[direction] * steps_forward
 *      Y += north_count[direction] * steps_forward
 *   2. Simulera höger-sväng (direction + 1) & 3:
 *      X += east_count[right_dir] * steps_right
 *      Y += north_count[right_dir] * steps_right
 * ======================================================================= */

void dm1_get_relative_map_coords(int party_x, int party_y, int direction,
                                  int steps_forward, int steps_right,
                                  int *out_x, int *out_y) {
    int dir = direction & 3;
    int x = party_x;
    int y = party_y;

    /* Steg 1: Framåt i partyts riktning */
    x += dm1_direction_to_step_east[dir]  * steps_forward;
    y += dm1_direction_to_step_north[dir] * steps_forward;

    /* Steg 2: Höger = direction + 1 */
    int right_dir = (dir + 1) & 3;
    x += dm1_direction_to_step_east[right_dir]  * steps_right;
    y += dm1_direction_to_step_north[right_dir] * steps_right;

    *out_x = x;
    *out_y = y;
}


/* =======================================================================
 * dm1_compute_view_square_coords
 *
 * Beräkna kartkoordinater för alla 12 viewport-rutor.
 *
 * Viewport-layout (DEFS.H view square diagram):
 *   Depth 3: C=3 framåt, L=3 framåt + 1 vänster, R=3 framåt + 1 höger
 *   Depth 2: C=2 framåt, L=2 framåt + 1 vänster, R=2 framåt + 1 höger
 *   Depth 1: C=1 framåt, L=1 framåt + 1 vänster, R=1 framåt + 1 höger
 *   Depth 0: C=0 framåt, L=0 framåt + 1 vänster, R=0 framåt + 1 höger
 *
 * "Vänster" = −1 i högerriktning.
 *
 * Ordning i arrayen: depth 3→0, per depth: C, L, R.
 * Index i arrayen: [0]=D3C, [1]=D3L, [2]=D3R, [3]=D2C, [4]=D2L, [5]=D2R,
 *                  [6]=D1C, [7]=D1L, [8]=D1R, [9]=D0C, [10]=D0L, [11]=D0R
 *
 * View indices (DM1_VS_*): D3C=0, D3L=1, D3R=2, D2C=3, ...
 * ======================================================================= */

/* Intern lookup: depth + lane → (steps_forward, steps_right) */
static const struct {
    int depth;          /* 3, 2, 1, 0 */
    int lane;           /* 0=C, 1=L, 2=R */
    int steps_forward;
    int steps_right;    /* Negativ = vänster */
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
 * Givet en viewport-ruta och party-riktning, avgör vilka view walls
 * som ska ritas.
 *
 * Wall zone-logik (DUNVIEW.C + DEFS.H):
 *
 * Front wall vid en ruta = rutan har element == WALL.
 * Front walls ritas för den rutan vid matchande depth/lane.
 *
 * Side walls uppstår när en grannruta (left/right) är WALL.
 * T.ex. D3L right wall (index 0) ritas om D3L:s HÖGRA granne (= D3C)
 * INTE är en vägg men D3L ÄR en vägg.
 *
 * Förenklad bitmask-approach: vi rapporterar vilka av de 13 wall positions
 * som berörs av denna specifika ruta. Den fulla renderingsloopen itererar
 * alla rutor och aggregerar.
 *
 * Returvärde: 15-bit bitmask, bit N = view wall index N.
 * D0L/D0R are nearest side-wall planes; they do not center-occlude, but
 * ReDMCSB still draws their side wall bitmaps before returning.
 * ======================================================================= */

uint16_t dm1_compute_wall_visibility(const dm1_view_square_t *square,
                                      int direction) {
    (void)direction;  /* Riktning bakat redan i aspect-beräkningen */

    uint16_t mask = 0;
    int elem = square->aspect[DM1_SQA_ELEMENT];

    /* Bara WALL och stängd FAKEWALL producerar väggar */
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
 * Algoritm (deriverad från DUNVIEW.C viewport-ritningsloop):
 *
 *   1. Beräkna kartkoordinater för alla 12 viewport-rutor.
 *   2. För varje ruta, läs raw square byte via callback.
 *   3. Dekoda square byte → fyll aspect[0] (element).
 *      (Fullständig SetSquareAspect med ornament kräver thing-list access
 *       som hanteras av konsumenten; vi sätter element + grundflaggor.)
 *   4. Ocklusionsberäkning: iterera depth 3→0.
 *      Om center-rutan vid depth D har element == WALL:
 *        → depth_occluded[D+1..3] = true
 *        → alla rutor vid djup > D markeras occluded = true.
 *
 * VIKTIGT: Ocklusionsmodellen i DM1 V1 (DRAWVIEW.C) är:
 *   - En front wall i CENTER-lane vid depth D blockerar rendering
 *     av allt vid depth D+1, D+2, D+3.
 *   - Side walls i L/R-lane ockluderar INTE rutorna bakom dem
 *     (bara center-lane front walls har full ocklusion).
 *   - Stängda fakewalls beter sig som väggar (ocklusion).
 *   - Dörrar med state >= 2 (halv-stängd till stängd) ritas som
 *     front-wall-element men ockluderar INTE fullt
 *     (man kan se genom/över dem).
 * ======================================================================= */

void dm1_build_viewport(int party_x, int party_y, int direction, int party_map,
                         dm1_square_reader_fn reader, void *user_data,
                         dm1_viewport_state_t *out) {
    memset(out, 0, sizeof(*out));

    out->party_x   = party_x;
    out->party_y   = party_y;
    out->party_dir = direction;
    out->party_map = party_map;

    /* Steg 1: Beräkna koordinater */
    dm1_compute_view_square_coords(party_x, party_y, direction, out->squares);
    out->valid_count = DM1_VIEWPORT_SQUARE_COUNT;

    /* Steg 2–3: Läs och dekoda varje ruta */
    for (int i = 0; i < DM1_VIEWPORT_SQUARE_COUNT; i++) {
        dm1_view_square_t *vs = &out->squares[i];
        uint8_t raw = reader(vs->map_x, vs->map_y, user_data);

        /* Fyll grundläggande aspect enligt F0172. */
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

    /* Steg 4: Ocklusionsberäkning — depth 0→3 (närmast → längst bort).
     *
     * Logik: om center-rutan vid depth D är en front wall,
     * ockludera alla rutor vid depth D+1, D+2, D+3.
     *
     * Vi itererar depth 0→3 och markerar ocklusion progressivt.
     * (DRAWVIEW.C itererar 3→0 och skippar rendering; vi markerar istället.)
     */
    bool occluded_beyond[DM1_DEPTH_ZONE_COUNT + 1];
    memset(occluded_beyond, 0, sizeof(occluded_beyond));

    /* Hitta center-rutor per depth och bestäm ocklusion.
     * I vår array-layout: center-rutan vid depth D har index = (3-D)*3.
     * depth 3 → index 0, depth 2 → index 3, depth 1 → index 6, depth 0 → index 9.
     */
    for (int d = 0; d <= DM1_VISIBLE_DEPTH_MAX; d++) {
        int center_idx = (DM1_VISIBLE_DEPTH_MAX - d) * 3;  /* D3→0, D2→3, D1→6, D0→9 */

        if (out->squares[center_idx].is_front_wall) {
            /* Allt vid djup > d är ockluderat (längre bort) */
            for (int d2 = d + 1; d2 <= DM1_VISIBLE_DEPTH_MAX; d2++) {
                occluded_beyond[d2] = true;
                out->depth_occluded[d2] = true;
            }
        }
    }

    /* Applicera ocklusionsflaggor på individuella rutor */
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

    /* Center-ruta vid depth: index = (3-depth)*3 */
    int center_idx = (DM1_VISIBLE_DEPTH_MAX - depth) * 3;
    return vp->squares[center_idx].is_front_wall;
}


/* =======================================================================
 * dm1_get_visible_squares
 *
 * Returnerar index för icke-ockluderade rutor, ordnade back-to-front
 * (depth 3→0, per depth: L, R, C, matching DUNVIEW.C:8490-8542).
 * Denna ordning matchar DM1:s renderingsordning (DRAWVIEW.C).
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
 * Snabbcheck: blockerar square-byte rörelse?
 *
 * Baserat på CLIKMENU.C:F0366 (rad 274-290):
 *   WALL → blockerad
 *   DOOR → state >= 2 och != 5 (destroyed) → blockerad
 *   FAKEWALL → !open && !imaginary → blockerad
 *   Allt annat (CORRIDOR, PIT, STAIRS, TELEPORTER) → passabelt
 *
 * OBS: Pit-fall och kreatur-blockering hanteras av andra moduler
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
            /* Passabelt om state < 2 (open / one-fourth) eller == 5 (destroyed) */
            return (state >= 2) && (state != 5);
        }

        case DM1_ELEMENT_FAKEWALL:
            /* Passabelt om open ELLER imaginary */
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
