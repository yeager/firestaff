#ifndef FIRESTAFF_DM1_V2_VIEWPORT_RENDERER_PC34_H
#define FIRESTAFF_DM1_V2_VIEWPORT_RENDERER_PC34_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V2_VIEWPORT_W 224
#define DM1_V2_VIEWPORT_H 136
#define DM1_V2_MAX_DEPTH 4
#define DM1_V2_FOG_LEVELS 8
#define DM1_V2_MAX_DRAW_COMMANDS 64
#define DM1_V2_MAX_DUNGEON_MAPS 16

#define DM1_V2_ELEMENT_WALL 0
#define DM1_V2_ELEMENT_CORRIDOR 1
#define DM1_V2_ELEMENT_PIT 2
#define DM1_V2_ELEMENT_TELEPORTER 5
#define DM1_V2_ELEMENT_DOOR_FRONT 17
#define DM1_V2_ELEMENT_STAIRS_FRONT 19
#define DM1_V2_ELEMENT_DOOR_SIDE 16
#define DM1_V2_ELEMENT_STAIRS_SIDE 18

typedef enum {
    DM1_V2_VIEW_SQUARE_D3L = 0,
    DM1_V2_VIEW_SQUARE_D3R = 1,
    DM1_V2_VIEW_SQUARE_D3C = 2,
    DM1_V2_VIEW_SQUARE_D2L = 3,
    DM1_V2_VIEW_SQUARE_D2R = 4,
    DM1_V2_VIEW_SQUARE_D2C = 5,
    DM1_V2_VIEW_SQUARE_D1L = 6,
    DM1_V2_VIEW_SQUARE_D1R = 7,
    DM1_V2_VIEW_SQUARE_D1C = 8,
    DM1_V2_VIEW_SQUARE_D0L = 9,
    DM1_V2_VIEW_SQUARE_D0R = 10,
    DM1_V2_VIEW_SQUARE_D0C = 11,
    DM1_V2_VIEW_SQUARE_OTHER = 12
} DM1_V2_ViewSquare;

typedef enum {
    DM1_V2_DRAW_FLOOR_CEILING = 1,
    DM1_V2_DRAW_FLOOR_ORNAMENT = 2,
    DM1_V2_DRAW_WALL = 3,
    DM1_V2_DRAW_DOOR_FRONT = 4,
    DM1_V2_DRAW_STAIRS_FRONT = 5,
    DM1_V2_DRAW_OBJECTS_CREATURES_PROJECTILES = 6,
    DM1_V2_DRAW_FIELD = 7,
    DM1_V2_DRAW_PIT = 8
} DM1_V2_DrawOp;

typedef struct {
    int element;
    int hasObjects;
    int hasField;
} DM1_V2_ViewportSquareInput;

typedef struct {
    int mapX;
    int mapY;
    int direction;
    DM1_V2_ViewportSquareInput squares[4][3]; /* depth D0-D3, lateral L/C/R */
} DM1_V2_ViewportCompositionInput;


typedef struct {
    uint16_t rawMapDataByteOffset;
    uint8_t offsetMapX;
    uint8_t offsetMapY;
    uint16_t packedA;
    uint16_t packedB;
    uint16_t packedC;
    uint16_t packedD;
    int level;
    int width;
    int height;
    const uint8_t* column0;
} DM1_V2_DungeonDatMap;

typedef struct {
    const uint8_t* bytes;
    int byteCount;
    uint16_t ornamentRandomSeed;
    uint16_t rawMapDataByteCount;
    uint8_t mapCount;
    uint16_t textDataWordCount;
    uint16_t initialPartyLocation;
    uint16_t squareFirstThingCount;
    int initialMapX;
    int initialMapY;
    int initialDirection;
    int rawMapDataFileOffset;
    int checksumFileOffset;
    DM1_V2_DungeonDatMap maps[DM1_V2_MAX_DUNGEON_MAPS];
} DM1_V2_DungeonDatState;

typedef struct {
    int mapX;
    int mapY;
    int element;
    int hasObjects;
    int hasField;
} DM1_V2_DungeonFixtureSquare;

typedef struct {
    const char* name;
    const char* sourceRef;
    const char* dungeonDatSha256;
    int mapIndex;
    int startMapX;
    int startMapY;
    int startDirection;
    int defaultElement;
    const DM1_V2_DungeonFixtureSquare* squares;
    int squareCount;
} DM1_V2_DungeonStateFixture;

typedef struct {
    int x;
    int y;
    int width;
    int height;
    const char* name;
} DM1_V2_ViewportRegion;

typedef struct {
    int comparedPixels;
    int mismatchedPixels;
    int firstMismatchX;
    int firstMismatchY;
} DM1_V2_RegionCompareResult;

typedef struct {
    DM1_V2_DrawOp op;
    DM1_V2_ViewSquare square;
    int depth;
    int lateral;
    int element;
    int order;
    const char* sourceRef;
} DM1_V2_DrawCommand;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} DM1_V2_Color;

typedef struct {
    int16_t scrollOffX;
    int16_t scrollOffY;
    int16_t scrollTargetX;
    int16_t scrollTargetY;
    int16_t scrollSpeed;
    int16_t scrollProgress;
} DM1_V2_ScrollState;

typedef struct {
    uint8_t fogDensity[DM1_V2_MAX_DEPTH];
    uint8_t lightLevel;
    uint8_t torchRadius;
    uint8_t ambientR;
    uint8_t ambientG;
    uint8_t ambientB;
} DM1_V2_LightConfig;

typedef struct {
    DM1_V2_Color framebuffer[DM1_V2_VIEWPORT_H][DM1_V2_VIEWPORT_W];
    DM1_V2_ScrollState scroll;
    DM1_V2_LightConfig light;
    int dirty;
    int frameCount;
    int32_t lastRenderMs;
} DM1_V2_ViewportState;

void dm1_v2_vp_init(DM1_V2_ViewportState* vp);
void dm1_v2_vp_begin_scroll(DM1_V2_ViewportState* vp, int dx, int dy, int speed);
void dm1_v2_vp_tick_scroll(DM1_V2_ViewportState* vp, int dtMs);
int dm1_v2_vp_is_scrolling(const DM1_V2_ViewportState* vp);
void dm1_v2_vp_set_pixel(DM1_V2_ViewportState* vp, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
DM1_V2_Color dm1_v2_vp_get_pixel(const DM1_V2_ViewportState* vp, int x, int y);
void dm1_v2_vp_clear(DM1_V2_ViewportState* vp, uint8_t r, uint8_t g, uint8_t b);
void dm1_v2_vp_apply_fog(DM1_V2_ViewportState* vp, int depth);
void dm1_v2_vp_apply_light(DM1_V2_ViewportState* vp, int cx, int cy, int radius, uint8_t intensity);
void dm1_v2_vp_mark_dirty(DM1_V2_ViewportState* vp);
int dm1_v2_vp_is_dirty(const DM1_V2_ViewportState* vp);
int dm1_v2_vp_use_flipped_wall_bitmaps(int mapX, int mapY, int direction);
int dm1_v2_vp_square_occludes_beyond(DM1_V2_ViewSquare square, int element);
void dm1_v2_vp_present(DM1_V2_ViewportState* vp, int32_t nowMs);
void dm1_v2_vp_composition_init(DM1_V2_ViewportCompositionInput* input);
int dm1_v2_vp_relative_coords(int direction, int mapX, int mapY, int forward, int right, int* outX, int* outY);
const DM1_V2_DungeonStateFixture* dm1_v2_vp_dm1_pc34_entry_state_fixture(void);
int dm1_v2_vp_dungeon_dat_init(DM1_V2_DungeonDatState* outState, const uint8_t* bytes, int byteCount);
int dm1_v2_vp_dungeon_dat_get_square_raw(const DM1_V2_DungeonDatState* state, int mapIndex, int mapX, int mapY, uint8_t* outSquare);
int dm1_v2_vp_square_element_from_raw(uint8_t square, int direction);
int dm1_v2_vp_build_composition_from_dungeon(const DM1_V2_DungeonDatState* state, int mapIndex, int mapX, int mapY, int direction, DM1_V2_ViewportCompositionInput* outInput);
int dm1_v2_vp_build_composition_from_fixture(const DM1_V2_DungeonStateFixture* fixture, int mapX, int mapY, int direction, DM1_V2_ViewportCompositionInput* outInput);
int dm1_v2_vp_compare_viewport_region(const DM1_V2_Color* expected, const DM1_V2_Color* actual, int stride, DM1_V2_ViewportRegion region, DM1_V2_RegionCompareResult* result);
int dm1_v2_vp_emit_d0_d3_draw_list(const DM1_V2_ViewportCompositionInput* input, DM1_V2_DrawCommand* outCommands, int maxCommands);
int dm1_v2_vp_compare_draw_lists(const DM1_V2_DrawCommand* expected, int expectedCount, const DM1_V2_DrawCommand* actual, int actualCount, int* mismatchIndex);
int dm1_v2_vp_render_composition_flat(DM1_V2_ViewportState* vp, const DM1_V2_ViewportCompositionInput* input);
int dm1_v2_vp_write_png_rgba(const char* path, const DM1_V2_Color* pixels, int width, int height, int stride);

/* ── DM1 V2.1 EPX Viewport ─────────────────────────────────────────── */

/* V2.1 viewport state lives in g_v21_viewport (static in .c).
 * These functions manage the EPX full render pipeline. */

/* Initialize V2.1 viewport. scale: 2 for 640x400, 4 for 1280x800. */
void v21_viewport_init(int scale_factor);

/* Load DMA palette (DM1 6-level VGA palette, 256 entries). */
void v21_viewport_set_palette(const uint32_t *palette, int count);

/* Returns mutable pointer to the 320x200 indexed V1 framebuffer.
 * Game systems render into this via dm1_v1_viewport_3d_pc34_compat(). */
uint8_t *v21_viewport_get_v1_framebuffer_mut(void);

/* Get const pointer to V1 framebuffer (read-only). */
const uint8_t *v21_viewport_get_v1_framebuffer(void);

/* Get RGBA output after v21_viewport_render_full_pipeline() completes.
 * out_w/out_h: physical output dimensions (640x400 or 1280x800).
 * Returns pointer into g_v21_viewport.rgba_output. */
const uint32_t *v21_viewport_get_rgba(int *out_w, int *out_h);

/* DM1 V2.1 EPX full render pipeline entry point.
 *
 * Source-lock: ReDMCSB DUNVIEW.C:8318-8542 composition order preserved
 * in indexed v1_framebuffer; EPX (Scale2x family) doubles resolution
 * without blending palette indices — preserving edge sharpness.
 *
 * Pipeline:
 *   1. EPX 2x (indexed) — v1_framebuffer[320×200] → epx_buffer[640×400]
 *   2. Palette expand — epx_buffer[640×400] → rgba_output[640×400]
 *   3. Present hook updates viewport dirty/frame state
 *
 * Call v21_viewport_set_palette() before this, and copy indexed
 * framebuffer data into the V1 framebuffer first.
 *
 * Creature/object/projectile surfaces: no separate code needed —
 * dm1_v1_viewport_3d_pc34_compat() renders all gameplay content
 * (walls + creatures + objects + projectiles) to the indexed
 * framebuffer before EPX. Source-lock: DUNVIEW.C:4547-4602 F0115. */
void v21_viewport_render_full_pipeline(void);

/* Source evidence string for the V2.1 EPX pipeline. */
const char *v21_viewport_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_VIEWPORT_RENDERER_PC34_H */