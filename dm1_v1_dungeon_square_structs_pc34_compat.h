#ifndef DM1_V1_DUNGEON_SQUARE_STRUCTS_PC34_COMPAT_H
#define DM1_V1_DUNGEON_SQUARE_STRUCTS_PC34_COMPAT_H

/*
 * DM1 V1 Dungeon Square Data Structures — Wall Zones, Depth Zones, Occlusion
 * ============================================================================
 *
 * Geometrisk grund för viewport-rendering och rörelselogik.
 * Definierar hur dungeonrutor lagras i minnet, vilka väggar som
 * syns vid varje djup, och ocklusionslogik (front walls blockerar).
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *
 *   DEFS.H — Alla strukt-definitioner, square-typ-makron, element-enum,
 *            väggmasker, thing-typ-enum, cell/direction-enum, view square
 *            och view wall index-tabeller, square aspect-accessorer.
 *
 *   DUNGEON.C — F0151_DUNGEON_GetSquare (rad 937–978):
 *     Returnerar en byte per ruta; bits 7–5 = element-typ, bits 4–0 = flaggor.
 *     Gränskontroll: utanför karta → returnerar C00_ELEMENT_WALL med
 *     riktningsspecifika random-ornament-flaggor.
 *
 *   DUNGEON.C — F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement:
 *     Koordinattransform: direction + steg framåt/höger → absolut (X,Y).
 *     Använder G0233_ai_DirectionToStepEastCount[4] och
 *     G0234_ai_DirectionToStepNorthCount[4].
 *
 *   DUNGEON.C — F0172_DUNGEON_SetSquareAspect (rad 1170–1327):
 *     Beräknar square aspect[5/7 ints] per ruta i viewport:
 *     [0]=element, [1]=firstThing, [2..4]=wallOrnamentOrdinals (R/F/L),
 *     [4]=floorOrnamentOrdinal, etc. Hanterar WALL, PIT, CORRIDOR,
 *     FAKEWALL, TELEPORTER, STAIRS, DOOR.
 *
 *   DUNVIEW.C — (rad 700+): Viewport-ritningsloop. Itererar depth 3→0,
 *     lane C/L/R. Vid varje depth anropas F0172 för square aspect;
 *     front wall (element==WALL) vid djup D ockluderar allt bakom.
 *     Väggar ritas med index M575..M587 (13/15 view wall positions).
 *
 *   DRAWVIEW.C — F0093_DUNGEONVIEW_DrawDungeon:
 *     Master-loop som itererar depth 3→0, lane center/left/right.
 *     Anropar underfunktioner för floor/ceiling, walls, doors, objects,
 *     creatures, projectiles, explosions.
 *
 *   COORD.C — Koordinathjälpfunktioner; offsetberäkningar per karta.
 *
 * Viewport-geometri (DM1 V1 = Atari ST / Amiga versions 1.x/2.x):
 *
 *   224×136 pixel viewport (112 bytes bred, 136 rader).
 *   Depth 0 = rutorna vid partyt; Depth 3 = längst bort.
 *   Tre "lanes" per depth: Center, Left, Right.
 *   Totalt 12 synliga rutor + 3 vid depth 4 (data finns men ritas ej).
 *
 *   Synliga rutor (partyt vid D0C):
 *     D4L(-2)  D4C(-3)  D4R(-1)
 *     D3L(1)   D3C(0)   D3R(2)
 *     D2L(4)   D2C(3)   D2R(5)
 *     D1L(7)   D1C(6)   D1R(8)
 *     D0L(10)  D0C(9)   D0R(11)
 *
 *   Se DEFS.H view square diagram och M597..M611 index-definitioner.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * § 1. Square-byte layout (DEFS.H M034_SQUARE_TYPE / M035_SQUARE)
 * =======================================================================
 * Varje ruta i kartan lagras som EN byte:
 *   Bits 7-5: element-typ (0..6)
 *   Bit  4:   thing-list present (MASK0x0010_THING_LIST_PRESENT)
 *   Bits 3-0: typspecifika flaggor
 */

/* --- Element-typer (square types) --- */
#define DM1_ELEMENT_WALL         0
#define DM1_ELEMENT_CORRIDOR     1
#define DM1_ELEMENT_PIT          2
#define DM1_ELEMENT_STAIRS       3
#define DM1_ELEMENT_DOOR         4
#define DM1_ELEMENT_TELEPORTER   5
#define DM1_ELEMENT_FAKEWALL     6

/* Utökade element-typer (square aspect, ej lagrade i kartan) */
#define DM1_ELEMENT_DOOR_SIDE    16
#define DM1_ELEMENT_DOOR_FRONT   17
#define DM1_ELEMENT_STAIRS_SIDE  18
#define DM1_ELEMENT_STAIRS_FRONT 19

/* --- Square-byte extraktion --- */
#define DM1_SQUARE_TYPE(sq)       ((sq) >> 5)
#define DM1_SQUARE_FLAGS(sq)      ((sq) & 0x1F)
#define DM1_SQUARE_HAS_THINGS(sq) (((sq) & 0x10) != 0)

/* --- Wall-specifika flaggor (bits 3-0 för element-typ WALL) --- */
#define DM1_WALL_WEST_RANDOM_ORN   0x01  /* MASK0x0001 */
#define DM1_WALL_SOUTH_RANDOM_ORN  0x02  /* MASK0x0002 */
#define DM1_WALL_EAST_RANDOM_ORN   0x04  /* MASK0x0004 */
#define DM1_WALL_NORTH_RANDOM_ORN  0x08  /* MASK0x0008 */

/* --- Corridor-specifika --- */
#define DM1_CORRIDOR_RANDOM_ORN    0x08  /* MASK0x0008 */

/* --- Pit-specifika --- */
#define DM1_PIT_IMAGINARY          0x01  /* MASK0x0001 */
#define DM1_PIT_INVISIBLE          0x04  /* MASK0x0004 */
#define DM1_PIT_OPEN               0x08  /* MASK0x0008 */

/* --- Stairs-specifika --- */
#define DM1_STAIRS_UP              0x04  /* MASK0x0004 */
#define DM1_STAIRS_NS_ORIENTATION  0x08  /* MASK0x0008 */

/* --- Door-specifika --- */
#define DM1_DOOR_NS_ORIENTATION    0x08  /* MASK0x0008 */
#define DM1_DOOR_STATE_MASK        0x07  /* Low 3 bits */

/* --- Teleporter-specifika --- */
#define DM1_TELEPORTER_VISIBLE     0x04  /* MASK0x0004 */
#define DM1_TELEPORTER_OPEN        0x08  /* MASK0x0008 */

/* --- Fakewall-specifika --- */
#define DM1_FAKEWALL_IMAGINARY     0x01  /* MASK0x0001 */
#define DM1_FAKEWALL_OPEN          0x04  /* MASK0x0004 */
#define DM1_FAKEWALL_RANDOM_ORN    0x08  /* MASK0x0008 */

/* --- Thing-list present (gemensam) --- */
#define DM1_THING_LIST_PRESENT     0x10  /* MASK0x0010 */


/* =======================================================================
 * § 2. Directions och Cells (DEFS.H)
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
 * DM1 V1 (Atari ST / Amiga 2.x) viewport: 5 rader, 3 kolumner.
 * Depth = antal steg framåt från partyt:
 *   Depth 0: D0C (partyruta), D0L (vänster), D0R (höger)
 *   Depth 1: D1C, D1L, D1R
 *   Depth 2: D2C, D2L, D2R
 *   Depth 3: D3C, D3L, D3R
 *   Depth 4: D4C, D4L, D4R (data existerar men ritas normalt inte)
 *
 * DEFS.H definierar view square indices (versions 1.x/2.x):
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

#define DM1_VIEW_SQUARE_COUNT  15  /* D4L..D0R = 15 positioner (inkl D4) */
#define DM1_VISIBLE_DEPTH_MAX   3  /* Max renderbart djup (depth 0..3) */
#define DM1_DEPTH_ZONE_COUNT    4  /* Depth 0, 1, 2, 3 */
#define DM1_LANES_PER_DEPTH     3  /* Center, Left, Right */


/* =======================================================================
 * § 4. View Wall Indices (DEFS.H M575..M587)
 * =======================================================================
 * DM1 V1 har 13 view wall positions:
 *   D3L right(0), D3R left(1),
 *   D3L front(2), D3C front(3), D3R front(4),
 *   D2L right(5), D2R left(6),
 *   D2L front(7), D2C front(8), D2R front(9),
 *   D1L right(10), D1R left(11),
 *   D1C front(12)
 *
 * En "front wall" vid depth D blockerar allt bakom (occlusion).
 * Side walls (right/left) avgränsar lane-kanter.
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

#define DM1_VIEW_WALL_COUNT_V1  13

/* View cell indices (för objekt/kreatur-rendering inom en ruta) */
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
 * F0172_DUNGEON_SetSquareAspect bygger en int16_t-array per synlig ruta.
 * DM1 V1 (versions 1.x/2.x): 5 element.
 * Index (DEFS.H M550..M559):
 */
#define DM1_SQA_ELEMENT               0  /* C0_ELEMENT */
#define DM1_SQA_FIRST_THING           1  /* M550: THING index, eller ENDOFLIST */
#define DM1_SQA_RIGHT_WALL_ORN_ORD    2  /* M551: höger väggornament ordinal */
#define DM1_SQA_FRONT_WALL_ORN_ORD    3  /* M552: front väggornament ordinal */
#define DM1_SQA_LEFT_WALL_ORN_ORD     4  /* M553: vänster väggornament ordinal */
/* För icke-WALL-rutor överlappar index 2..4: */
#define DM1_SQA_PIT_TELEPORTER_VIS    2  /* M554: synlig pit/teleporter */
#define DM1_SQA_STAIRS_UP             2  /* M555 */
#define DM1_SQA_DOOR_STATE            2  /* M556 */
#define DM1_SQA_DOOR_THING_INDEX      3  /* M557 */
#define DM1_SQA_FLOOR_ORN_ORDINAL     4  /* M558 */

#define DM1_SQA_COUNT_V1              5  /* M559: antal int16_t i aspect-array */

#define DM1_FOOTPRINTS_MASK  0x8000  /* MASK0x8000_FOOTPRINTS i floor orn */


/* =======================================================================
 * § 6. Direction-to-Step Lookup Tables
 * =======================================================================
 * Från DUNGEON.C G0233/G0234. Konverterar direction → (dX, dY).
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
 * § 7. In-memory Dungeon Square Struct (vår representation)
 * =======================================================================
 * I originalet lagras varje ruta som EN byte (element|flags).
 * Thing-listor lagras separat i G0283_pT_SquareFirstThings.
 * Vår struct håller den dekodade informationen per ruta.
 */

typedef struct {
    uint8_t  raw_byte;         /* Originalbyte: bits 7-5 = element, bits 4-0 = flaggor */

    /* Dekodade fält */
    uint8_t  element;          /* DM1_ELEMENT_WALL..DM1_ELEMENT_FAKEWALL (0..6) */
    bool     has_thing_list;   /* Bit 4 av raw_byte */

    /* Typspecifika fält (union, beror på element) */
    union {
        struct {
            bool west_random_orn;   /* Bit 0: västra vägg tillåter random orn */
            bool south_random_orn;  /* Bit 1 */
            bool east_random_orn;   /* Bit 2 */
            bool north_random_orn;  /* Bit 3 */
        } wall;

        struct {
            bool random_orn_allowed; /* Bit 3 */
        } corridor;

        struct {
            bool imaginary;   /* Bit 0: osynlig pit, inget fall */
            bool invisible;   /* Bit 2: osynlig grafiskt */
            bool open;        /* Bit 3: öppen → fall igenom */
        } pit;

        struct {
            bool up;               /* Bit 2: trappa uppåt */
            bool ns_orientation;   /* Bit 3: nord-syd-orienterad */
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
 * § 8. View Square Descriptor (per synlig ruta i viewport)
 * ======================================================================= */

typedef struct {
    int map_x;         /* Absolut karta-X */
    int map_y;         /* Absolut karta-Y */
    int depth;         /* 0..3 (avstånd från party) */
    int lane;          /* 0=Center, 1=Left, 2=Right */
    int view_index;    /* DM1_VS_D0C..DM1_VS_D3R (view square index) */

    /* Square aspect (runtime-beräknat, se § 5) */
    int16_t aspect[DM1_SQA_COUNT_V1];

    /* Occlusion */
    bool occluded;          /* true om en front wall vid närmare djup blockerar */
    bool is_front_wall;     /* true om denna ruta ÄR en front wall (element==WALL) */
} dm1_view_square_t;


/* =======================================================================
 * § 9. Viewport State (alla synliga rutor + ocklusion)
 * ======================================================================= */

/* Max antal rutor i ett komplett viewport (depth 0..3, 3 lanes) */
#define DM1_VIEWPORT_SQUARE_COUNT  12  /* 4 djup × 3 lanes */

typedef struct {
    /* Partyposition */
    int party_x;
    int party_y;
    int party_dir;   /* DM1_DIR_NORTH..DM1_DIR_WEST */
    int party_map;   /* Kartindex */

    /* De 12 synliga rutorna (depth 0..3, lane C/L/R) */
    dm1_view_square_t squares[DM1_VIEWPORT_SQUARE_COUNT];

    /* Ocklusionsflaggor per depth (true = front wall blockerar detta djup+bortom) */
    bool depth_occluded[DM1_DEPTH_ZONE_COUNT];

    /* Antal giltiga rutor */
    int valid_count;
} dm1_viewport_state_t;


/* =======================================================================
 * § 10. API — Funktionsdeklarationer
 * ======================================================================= */

/*
 * dm1_decode_square — Dekoda en raw square-byte till dm1_dungeon_square_t.
 * Motsvarar logiken i F0151_DUNGEON_GetSquare (DUNGEON.C:937) +
 * dekodningen i F0172_DUNGEON_SetSquareAspect (DUNGEON.C:1170).
 */
void dm1_decode_square(uint8_t raw_byte, dm1_dungeon_square_t *out);

/*
 * dm1_get_relative_map_coords — Givet party-position + riktning + steg,
 * beräkna absolut (X,Y). Motsvarar F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement.
 * Parametrar: steps_forward (positiv = framåt), steps_right (positiv = höger).
 */
void dm1_get_relative_map_coords(int party_x, int party_y, int direction,
                                  int steps_forward, int steps_right,
                                  int *out_x, int *out_y);

/*
 * dm1_compute_view_square_coords — Beräkna kartkoordinater för alla
 * 12 (eller 15 inkl D4) viewport-rutor givet partyposition och riktning.
 * Fyller i map_x, map_y, depth, lane, view_index per ruta.
 *
 * Geometrin deriveras från DUNVIEW.C viewport-loop och
 * DUNGEON.C:F0150 koordinattransform.
 *
 *   D3: 3 steg framåt, lane: 0/−1/+1 höger
 *   D2: 2 steg framåt, lane: 0/−1/+1 höger
 *   D1: 1 steg framåt, lane: 0/−1/+1 höger
 *   D0: 0 steg framåt, lane: 0/−1/+1 höger
 */
void dm1_compute_view_square_coords(int party_x, int party_y, int direction,
                                     dm1_view_square_t out_squares[DM1_VIEWPORT_SQUARE_COUNT]);

/*
 * dm1_compute_wall_visibility — Givet en viewport-rutas square aspect,
 * avgör vilka view walls (index 0..12) som ska ritas.
 *
 * Logik (DUNVIEW.C):
 * - En WALL-ruta vid depth D genererar en front wall → ockluderar djup D+1..3.
 * - Side walls (right/left) ritas vid gränsen mot WALL-ruta i grann-lane.
 * - Front walls ritas BARA om rutan FRAMFÖR (mot partyt) INTE är en vägg.
 *
 * Returnerar en 13-bit bitmask: bit N = view wall N ska ritas.
 */
uint16_t dm1_compute_wall_visibility(const dm1_view_square_t *square,
                                      int direction);

/*
 * dm1_build_viewport — Huvudfunktion: bygg komplett viewport-state.
 *
 * Givet party-position, riktning, och en callback som returnerar
 * raw square-byte för given (mapX, mapY), bygger viewport med:
 * 1. Kartkoordinater per synlig ruta
 * 2. Square aspect per ruta (via callback + dm1_decode_square)
 * 3. Ocklusionsberäkning: depth 3→0, front walls blockerar bakåt
 *
 * square_reader: callback(map_x, map_y, user_data) → raw square byte.
 *                Om utanför karta: returnera (DM1_ELEMENT_WALL << 5).
 */
typedef uint8_t (*dm1_square_reader_fn)(int map_x, int map_y, void *user_data);

void dm1_build_viewport(int party_x, int party_y, int direction, int party_map,
                         dm1_square_reader_fn reader, void *user_data,
                         dm1_viewport_state_t *out);

/*
 * dm1_is_front_wall_at_depth — Returnerar true om en front wall finns
 * vid angivet djup i center-lane. Används för enkel ocklusionscheck.
 */
bool dm1_is_front_wall_at_depth(const dm1_viewport_state_t *vp, int depth);

/*
 * dm1_get_visible_squares — Returnerar antal icke-ockluderade rutor.
 * Fyller visible_indices[] med index i vp->squares[].
 * Ordning: depth 3→0 (back-to-front), per lane center→left→right.
 */
int dm1_get_visible_squares(const dm1_viewport_state_t *vp,
                             int visible_indices[DM1_VIEWPORT_SQUARE_COUNT]);

/*
 * dm1_square_blocks_movement — Avgör om en square-byte blockerar rörelse.
 * Samordnas med dm1_v1_collision_door_pc34_compat men är en fristående
 * snabbcheck baserad enbart på square-byte-data.
 *
 * Blockering (CLIKMENU.C:F0366, DUNGEON.C:F0151):
 *   WALL: alltid blockerad
 *   DOOR: state >= 2 och != 5 → blockerad
 *   FAKEWALL: !open && !imaginary → blockerad
 *   Allt annat: passabelt (pit/corridor/stairs/teleporter)
 */
bool dm1_square_blocks_movement(uint8_t raw_byte);


#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_DUNGEON_SQUARE_STRUCTS_PC34_COMPAT_H */
