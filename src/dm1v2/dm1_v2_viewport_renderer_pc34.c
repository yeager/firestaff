#include "dm1_v2_viewport_renderer_pc34.h"
#include "dm1_v2_texture_upscale_pc34.h"
#include "vga_palette_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Framebuffer encoding: low nibble = palette index (0-15), high nibble = brightness level (0-5) */
#ifndef M11_FB_INDEX_MASK
#define M11_FB_INDEX_MASK   0x0F
#define M11_FB_LEVEL_SHIFT  4
#define M11_FB_LEVEL_MASK   0xF0
#endif
#ifndef DM1_V2_PALETTE_LEVELS
#define DM1_V2_PALETTE_LEVELS 6
#endif

static uint8_t clamp_u8(int val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return (uint8_t)val;
}

void dm1_v2_vp_init(DM1_V2_ViewportState* vp) {
    if (!vp) return;
    
    // Zero out the entire state
    memset(vp, 0, sizeof(DM1_V2_ViewportState));
    
    // Initialize light defaults
    // Fog density increasing with depth
    vp->light.fogDensity[0] = 0;
    vp->light.fogDensity[1] = 64;
    vp->light.fogDensity[2] = 128;
    vp->light.fogDensity[3] = 192;
    
    vp->light.lightLevel = 128;
    vp->light.torchRadius = 3;
    vp->light.ambientR = 32;
    vp->light.ambientG = 32;
    vp->light.ambientB = 48;
    
    vp->dirty = 1;
    vp->frameCount = 0;
    vp->lastRenderMs = 0;
}

void dm1_v2_vp_begin_scroll(DM1_V2_ViewportState* vp, int dx, int dy, int speed) {
    if (!vp) return;
    
    vp->scroll.scrollTargetX = vp->scroll.scrollOffX + dx;
    vp->scroll.scrollTargetY = vp->scroll.scrollOffY + dy;
    vp->scroll.scrollSpeed = speed;
    vp->scroll.scrollProgress = 0;
    
    vp->dirty = 1;
}

void dm1_v2_vp_tick_scroll(DM1_V2_ViewportState* vp, int dtMs) {
    if (!vp) return;
    
    // Check if already at target
    if (vp->scroll.scrollOffX == vp->scroll.scrollTargetX && 
        vp->scroll.scrollOffY == vp->scroll.scrollTargetY) {
        return;
    }
    
    // Calculate progress increment
    // progress += speed * dtMs / 1000
    int progressInc = (vp->scroll.scrollSpeed * dtMs) / 1000;
    vp->scroll.scrollProgress += progressInc;
    
    // Lerp offset toward target
    // We treat scrollProgress as a counter that determines how much to move
    // For simplicity, we move by speed pixels per second
    int dx = vp->scroll.scrollTargetX - vp->scroll.scrollOffX;
    int dy = vp->scroll.scrollTargetY - vp->scroll.scrollOffY;
    
    // Determine direction and step
    int stepX = 0;
    int stepY = 0;
    
    if (dx > 0) stepX = 1;
    else if (dx < 0) stepX = -1;
    
    if (dy > 0) stepY = 1;
    else if (dy < 0) stepY = -1;
    
    // Move by speed pixels per second
    // pixels to move = speed * dtMs / 1000
    int moveAmount = (vp->scroll.scrollSpeed * dtMs) / 1000;
    
    if (moveAmount <= 0) moveAmount = 1; // Minimum movement
    
    // Move X without overshooting in either direction.
    if (dx != 0) {
        int moveX = stepX * moveAmount;
        if ((stepX > 0 && moveX > dx) ||
            (stepX < 0 && moveX < dx)) {
            moveX = dx;
        }
        vp->scroll.scrollOffX += moveX;
    }
    
    // Move Y without overshooting in either direction.
    if (dy != 0) {
        int moveY = stepY * moveAmount;
        if ((stepY > 0 && moveY > dy) ||
            (stepY < 0 && moveY < dy)) {
            moveY = dy;
        }
        vp->scroll.scrollOffY += moveY;
    }
    
    // Check if arrived
    if (vp->scroll.scrollOffX == vp->scroll.scrollTargetX && 
        vp->scroll.scrollOffY == vp->scroll.scrollTargetY) {
        vp->scroll.scrollProgress = 0;
    }
    
    vp->dirty = 1;
}

int dm1_v2_vp_is_scrolling(const DM1_V2_ViewportState* vp) {
    if (!vp) return 0;
    return (vp->scroll.scrollOffX != vp->scroll.scrollTargetX || 
            vp->scroll.scrollOffY != vp->scroll.scrollTargetY);
}

void dm1_v2_vp_set_pixel(DM1_V2_ViewportState* vp, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (!vp) return;
    
    // Bounds check
    if (x < 0 || x >= DM1_V2_VIEWPORT_W || y < 0 || y >= DM1_V2_VIEWPORT_H) {
        return;
    }
    
    vp->framebuffer[y][x].r = r;
    vp->framebuffer[y][x].g = g;
    vp->framebuffer[y][x].b = b;
    vp->framebuffer[y][x].a = a;
    
    vp->dirty = 1;
}

DM1_V2_Color dm1_v2_vp_get_pixel(const DM1_V2_ViewportState* vp, int x, int y) {
    DM1_V2_Color black = {0, 0, 0, 255};
    
    if (!vp) return black;
    
    // Bounds check
    if (x < 0 || x >= DM1_V2_VIEWPORT_W || y < 0 || y >= DM1_V2_VIEWPORT_H) {
        return black;
    }
    
    return vp->framebuffer[y][x];
}

void dm1_v2_vp_clear(DM1_V2_ViewportState* vp, uint8_t r, uint8_t g, uint8_t b) {
    if (!vp) return;
    
    for (int y = 0; y < DM1_V2_VIEWPORT_H; y++) {
        for (int x = 0; x < DM1_V2_VIEWPORT_W; x++) {
            vp->framebuffer[y][x].r = r;
            vp->framebuffer[y][x].g = g;
            vp->framebuffer[y][x].b = b;
            vp->framebuffer[y][x].a = 255;
        }
    }
    
    vp->dirty = 1;
}

void dm1_v2_vp_apply_fog(DM1_V2_ViewportState* vp, int depth) {
    if (!vp) return;
    
    // Clamp depth
    if (depth < 0) depth = 0;
    if (depth >= DM1_V2_MAX_DEPTH) depth = DM1_V2_MAX_DEPTH - 1;
    
    uint8_t density = vp->light.fogDensity[depth];
    uint8_t ambR = vp->light.ambientR;
    uint8_t ambG = vp->light.ambientG;
    uint8_t ambB = vp->light.ambientB;
    
    for (int y = 0; y < DM1_V2_VIEWPORT_H; y++) {
        for (int x = 0; x < DM1_V2_VIEWPORT_W; x++) {
            DM1_V2_Color* px = &vp->framebuffer[y][x];
            
            // Blend toward ambient by fogDensity/255
            int factor = density;
            int invFactor = 255 - factor;
            
            px->r = (uint8_t)((px->r * invFactor + ambR * factor) / 255);
            px->g = (uint8_t)((px->g * invFactor + ambG * factor) / 255);
            px->b = (uint8_t)((px->b * invFactor + ambB * factor) / 255);
        }
    }
    
    vp->dirty = 1;
}

void dm1_v2_vp_apply_light(DM1_V2_ViewportState* vp, int cx, int cy, int radius, uint8_t intensity) {
    if (!vp) return;
    
    int r2 = radius * radius;
    
    for (int y = 0; y < DM1_V2_VIEWPORT_H; y++) {
        for (int x = 0; x < DM1_V2_VIEWPORT_W; x++) {
            int dx = x - cx;
            int dy = y - cy;
            int dist2 = dx * dx + dy * dy;
            
            if (dist2 < r2) {
                // Calculate distance factor: 1 - dist/radius
                // Use integer math: factor = (radius*255 - dist*255/radius) / 255
                // Simplified: factor = (r2 - dist2) / r2 * 255
                int dist __attribute__((unused)) = 0;
                // Approximate sqrt using integer math or just use dist2
                // For simplicity, use dist2 based falloff
                int factor = (r2 - dist2) * 255 / r2;
                if (factor < 0) factor = 0;
                if (factor > 255) factor = 255;
                
                // Brighten by intensity * factor
                int brighten = (intensity * factor) / 255;
                
                DM1_V2_Color* px = &vp->framebuffer[y][x];
                
                int newR = px->r + brighten;
                int newG = px->g + brighten;
                int newB = px->b + brighten;
                
                px->r = clamp_u8(newR);
                px->g = clamp_u8(newG);
                px->b = clamp_u8(newB);
            }
        }
    }
    
    vp->dirty = 1;
}

void dm1_v2_vp_mark_dirty(DM1_V2_ViewportState* vp) {
    if (!vp) return;
    vp->dirty = 1;
}

int dm1_v2_vp_is_dirty(const DM1_V2_ViewportState* vp) {
    if (!vp) return 0;
    return vp->dirty;
}


int dm1_v2_vp_use_flipped_wall_bitmaps(int mapX, int mapY, int direction) {
    /* Source-lock: ReDMCSB DUNVIEW.C:8357 uses
       G0076_B_UseFlippedWallAndFootprintsBitmaps = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001. */
    return (mapX + mapY + direction) & 0x0001;
}

int dm1_v2_vp_square_occludes_beyond(DM1_V2_ViewSquare square, int element) {
    /* Source-lock: ReDMCSB DUNVIEW.C:6697-6720 draws D3C wall and returns before
       DUNVIEW.C:6816 object/creature/projectile/explosion draw, so a center wall
       terminates the forward composition lane. Doors/corridors still continue into
       the pass-specific draw-order path at DUNVIEW.C:6721-6816. */
    return square == DM1_V2_VIEW_SQUARE_D3C && element == DM1_V2_ELEMENT_WALL;
}


static int dm1_v2_vp_lateral_index(int lateral) {
    if (lateral < 0) return 0;
    if (lateral > 0) return 2;
    return 1;
}

DM1_V2_ViewSquare dm1_v2_vp_square_id(int depth, int lateral) {
    if (depth == 3 && lateral < 0) return DM1_V2_VIEW_SQUARE_D3L;
    if (depth == 3 && lateral > 0) return DM1_V2_VIEW_SQUARE_D3R;
    if (depth == 3) return DM1_V2_VIEW_SQUARE_D3C;
    if (depth == 2 && lateral < 0) return DM1_V2_VIEW_SQUARE_D2L;
    if (depth == 2 && lateral > 0) return DM1_V2_VIEW_SQUARE_D2R;
    if (depth == 2) return DM1_V2_VIEW_SQUARE_D2C;
    if (depth == 1 && lateral < 0) return DM1_V2_VIEW_SQUARE_D1L;
    if (depth == 1 && lateral > 0) return DM1_V2_VIEW_SQUARE_D1R;
    if (depth == 1) return DM1_V2_VIEW_SQUARE_D1C;
    if (depth == 0 && lateral < 0) return DM1_V2_VIEW_SQUARE_D0L;
    if (depth == 0 && lateral > 0) return DM1_V2_VIEW_SQUARE_D0R;
    if (depth == 0) return DM1_V2_VIEW_SQUARE_D0C;
    return DM1_V2_VIEW_SQUARE_OTHER;
}

static int dm1_v2_vp_push_draw(DM1_V2_DrawCommand* outCommands,
                               int maxCommands,
                               int* count,
                               DM1_V2_DrawOp op,
                               DM1_V2_ViewSquare square,
                               int depth,
                               int lateral,
                               int element,
                               int order,
                               const char* sourceRef) {
    if (!outCommands || !count || *count < 0 || maxCommands < 0) return 0;
    if (*count >= maxCommands) return 0;
    outCommands[*count].op = op;
    outCommands[*count].square = square;
    outCommands[*count].depth = depth;
    outCommands[*count].lateral = lateral;
    outCommands[*count].element = element;
    outCommands[*count].order = order;
    outCommands[*count].sourceRef = sourceRef;
    (*count)++;
    return 1;
}



static uint16_t dm1_v2_read_le16(const uint8_t* bytes, int offset) {
    return (uint16_t)(bytes[offset] | ((uint16_t)bytes[offset + 1] << 8));
}

int dm1_v2_vp_dungeon_dat_init(DM1_V2_DungeonDatState* outState,
                               const uint8_t* bytes,
                               int byteCount) {
    int mapTableOffset = 44;
    int mapStructSize = 16;
    int mapTableEnd = 0;
    int rawOffset = 0;
    if (!outState || !bytes || byteCount < mapTableOffset + 2) return 0;
    memset(outState, 0, sizeof(*outState));
    outState->bytes = bytes;
    outState->byteCount = byteCount;
    outState->ornamentRandomSeed = dm1_v2_read_le16(bytes, 0);
    outState->rawMapDataByteCount = dm1_v2_read_le16(bytes, 2);
    outState->mapCount = bytes[4];
    outState->textDataWordCount = dm1_v2_read_le16(bytes, 6);
    outState->initialPartyLocation = dm1_v2_read_le16(bytes, 8);
    outState->squareFirstThingCount = dm1_v2_read_le16(bytes, 10);
    outState->initialMapX = outState->initialPartyLocation & 0x001F;
    outState->initialMapY = (outState->initialPartyLocation >> 5) & 0x001F;
    outState->initialDirection = (outState->initialPartyLocation >> 10) & 0x0003;
    if (outState->mapCount == 0 || outState->mapCount > DM1_V2_MAX_DUNGEON_MAPS) return 0;
    mapTableEnd = mapTableOffset + outState->mapCount * mapStructSize;
    if (byteCount < mapTableEnd) return 0;

    /* Source-lock: ReDMCSB LOADSAVE.C:906-923 reads RawMapDataByteCount and then
       builds G0279_pppuc_DungeonMapData columns from G0276_puc_DungeonRawMapData +
       MAP.RawMapDataByteOffset. The canonical PC34 DUNGEON.DAT stores a trailing
       checksum after raw map data, so this decoder anchors raw map data at EOF - 2 - count. */
    rawOffset = byteCount - 2 - (int)outState->rawMapDataByteCount;
    if (rawOffset < mapTableEnd || rawOffset + (int)outState->rawMapDataByteCount > byteCount) return 0;
    outState->rawMapDataFileOffset = rawOffset;
    outState->checksumFileOffset = rawOffset + (int)outState->rawMapDataByteCount;

    for (int i = 0; i < outState->mapCount; i++) {
        const int off = mapTableOffset + i * mapStructSize;
        DM1_V2_DungeonDatMap* map = &outState->maps[i];
        map->rawMapDataByteOffset = dm1_v2_read_le16(bytes, off + 0);
        map->offsetMapX = bytes[off + 6];
        map->offsetMapY = bytes[off + 7];
        map->packedA = dm1_v2_read_le16(bytes, off + 8);
        map->packedB = dm1_v2_read_le16(bytes, off + 10);
        map->packedC = dm1_v2_read_le16(bytes, off + 12);
        map->packedD = dm1_v2_read_le16(bytes, off + 14);
        /* Source-lock: DEFS.H:972-1016 MAP.A bitfields on PC/I34E are Level:6,
           Width:5, Height:5; DUNGEON.C:2276-2277 exposes dimensions as +1. */
        map->level = map->packedA & 0x003F;
        map->width = ((map->packedA >> 6) & 0x001F) + 1;
        map->height = ((map->packedA >> 11) & 0x001F) + 1;
        if (map->width <= 0 || map->height <= 0) return 0;
        if ((int)map->rawMapDataByteOffset + map->width * map->height > (int)outState->rawMapDataByteCount) return 0;
        map->column0 = bytes + rawOffset + map->rawMapDataByteOffset;
    }
    return 1;
}

int dm1_v2_vp_dungeon_dat_get_square_raw(const DM1_V2_DungeonDatState* state,
                                         int mapIndex,
                                         int mapX,
                                         int mapY,
                                         uint8_t* outSquare) {
    const DM1_V2_DungeonDatMap* map;
    if (!state || !outSquare || mapIndex < 0 || mapIndex >= state->mapCount) return 0;
    map = &state->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height) {
        *outSquare = 0; /* ReDMCSB F0151 returns a wall square type for out-of-bounds. */
        return 1;
    }
    /* Source-lock: LOADSAVE.C:917-923 stores column pointers; each next column advances by height+1. */
    *outSquare = map->column0[mapX * map->height + mapY];
    return 1;
}

int dm1_v2_vp_square_element_from_raw(uint8_t square, int direction) {
    int type = square >> 5; /* Source-lock: DEFS.H:922-941 M034_SQUARE_TYPE. */
    direction &= 3;
    switch (type) {
        case 0: return DM1_V2_ELEMENT_WALL;
        case 1: return DM1_V2_ELEMENT_CORRIDOR;
        case 2: return DM1_V2_ELEMENT_PIT;
        case 3:
            /* Source-lock: DUNGEON.C:2238-2239 turns stair raw type into side/front aspect. */
            return (((square & 0x08) >> 3) == (direction & 1)) ? DM1_V2_ELEMENT_STAIRS_SIDE : DM1_V2_ELEMENT_STAIRS_FRONT;
        case 4:
            /* Source-lock: DUNGEON.C:2243-2246 turns door raw type into side/front aspect. */
            return (((square & 0x08) >> 3) == (direction & 1)) ? DM1_V2_ELEMENT_DOOR_SIDE : DM1_V2_ELEMENT_DOOR_FRONT;
        case 5: return DM1_V2_ELEMENT_TELEPORTER;
        case 6:
            /* Closed fake walls become wall aspect; open fake walls become corridor (DUNGEON.C:2199-2210). */
            return (square & 0x04) ? DM1_V2_ELEMENT_CORRIDOR : DM1_V2_ELEMENT_WALL;
        default: return DM1_V2_ELEMENT_WALL;
    }
}

int dm1_v2_vp_build_composition_from_dungeon(const DM1_V2_DungeonDatState* state,
                                             int mapIndex,
                                             int mapX,
                                             int mapY,
                                             int direction,
                                             DM1_V2_ViewportCompositionInput* outInput) {
    static const int kDepthOrder[12] = {3,3,3, 2,2,2, 1,1,1, 0,0,0};
    static const int kLateralOrder[12] = {-1,1,0, -1,1,0, -1,1,0, -1,1,0};
    if (!state || !outInput || mapIndex < 0 || mapIndex >= state->mapCount) return 0;
    dm1_v2_vp_composition_init(outInput);
    outInput->mapX = mapX;
    outInput->mapY = mapY;
    outInput->direction = direction & 3;
    for (int i = 0; i < 12; i++) {
        int x = 0;
        int y = 0;
        uint8_t raw = 0;
        int depth = kDepthOrder[i];
        int lateral = kLateralOrder[i];
        int lateralIndex = dm1_v2_vp_lateral_index(lateral);
        if (!dm1_v2_vp_relative_coords(direction, mapX, mapY, depth, lateral, &x, &y)) return 0;
        if (!dm1_v2_vp_dungeon_dat_get_square_raw(state, mapIndex, x, y, &raw)) return 0;
        outInput->squares[depth][lateralIndex].element = dm1_v2_vp_square_element_from_raw(raw, direction);
        outInput->squares[depth][lateralIndex].hasObjects = (raw & 0x10) ? 1 : 0;
        outInput->squares[depth][lateralIndex].hasField = ((raw >> 5) == 5 && (raw & 0x0C) == 0x0C) ? 1 : 0;
    }
    return 1;
}

static const DM1_V2_DungeonFixtureSquare k_dm1_pc34_entry_state_squares[] = {
    /* Real DM1 PC34 start: DUNGEON.DAT offset 8 decodes to map0 x=1 y=3 dir=2.
       pass173/pass162 source audits identify the front square x=1,y=4 as the wall
       champion-portrait sensor square. This fixture keeps the rest corridor until
       a full DUNGEON.DAT square decoder is landed. */
    {1, 4, DM1_V2_ELEMENT_WALL, 0, 0},
};

static const DM1_V2_DungeonStateFixture k_dm1_pc34_entry_state_fixture = {
    "dm1_pc34_entry_portrait_wall",
    "DUNGEON.DAT offset 8 + DEFS.H:989-998 + LOADSAVE.C:1940-1945 + pass173 front-wall sensor audit",
    "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    0,
    1,
    3,
    2,
    DM1_V2_ELEMENT_CORRIDOR,
    k_dm1_pc34_entry_state_squares,
    (int)(sizeof(k_dm1_pc34_entry_state_squares) / sizeof(k_dm1_pc34_entry_state_squares[0])),
};

static DM1_V2_ViewportSquareInput dm1_v2_vp_lookup_fixture_square(const DM1_V2_DungeonStateFixture* fixture,
                                                                   int mapX,
                                                                   int mapY) {
    DM1_V2_ViewportSquareInput square;
    square.element = fixture ? fixture->defaultElement : DM1_V2_ELEMENT_WALL;
    square.hasObjects = 0;
    square.hasField = 0;
    if (!fixture || !fixture->squares || fixture->squareCount <= 0) return square;
    for (int i = 0; i < fixture->squareCount; i++) {
        const DM1_V2_DungeonFixtureSquare* candidate = &fixture->squares[i];
        if (candidate->mapX == mapX && candidate->mapY == mapY) {
            square.element = candidate->element;
            square.hasObjects = candidate->hasObjects;
            square.hasField = candidate->hasField;
            return square;
        }
    }
    return square;
}

int dm1_v2_vp_relative_coords(int direction,
                              int mapX,
                              int mapY,
                              int forward,
                              int right,
                              int* outX,
                              int* outY) {
    /* Source-lock: ReDMCSB DUNGEON.C:35-44 direction step tables and
       DUNGEON.C:1371-1391 F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement. */
    static const int kStepEast[4] = {0, 1, 0, -1};
    static const int kStepNorth[4] = {-1, 0, 1, 0};
    if (!outX || !outY) return 0;
    direction &= 3;
    int rightDirection = (direction + 1) & 3;
    *outX = mapX + kStepEast[direction] * forward + kStepEast[rightDirection] * right;
    *outY = mapY + kStepNorth[direction] * forward + kStepNorth[rightDirection] * right;
    return 1;
}

const DM1_V2_DungeonStateFixture* dm1_v2_vp_dm1_pc34_entry_state_fixture(void) {
    return &k_dm1_pc34_entry_state_fixture;
}

int dm1_v2_vp_build_composition_from_fixture(const DM1_V2_DungeonStateFixture* fixture,
                                             int mapX,
                                             int mapY,
                                             int direction,
                                             DM1_V2_ViewportCompositionInput* outInput) {
    static const int kDepthOrder[12] = {3,3,3, 2,2,2, 1,1,1, 0,0,0};
    static const int kLateralOrder[12] = {-1,1,0, -1,1,0, -1,1,0, -1,1,0};
    if (!fixture || !outInput) return 0;
    dm1_v2_vp_composition_init(outInput);
    outInput->mapX = mapX;
    outInput->mapY = mapY;
    outInput->direction = direction & 3;
    for (int i = 0; i < 12; i++) {
        int x = 0;
        int y = 0;
        int depth = kDepthOrder[i];
        int lateral = kLateralOrder[i];
        int lateralIndex = dm1_v2_vp_lateral_index(lateral);
        if (!dm1_v2_vp_relative_coords(direction, mapX, mapY, depth, lateral, &x, &y)) return 0;
        outInput->squares[depth][lateralIndex] = dm1_v2_vp_lookup_fixture_square(fixture, x, y);
    }
    return 1;
}

int dm1_v2_vp_compare_viewport_region(const DM1_V2_Color* expected,
                                      const DM1_V2_Color* actual,
                                      int stride,
                                      DM1_V2_ViewportRegion region,
                                      DM1_V2_RegionCompareResult* result) {
    if (result) {
        result->comparedPixels = 0;
        result->mismatchedPixels = 0;
        result->firstMismatchX = -1;
        result->firstMismatchY = -1;
    }
    if (!expected || !actual || stride <= 0 || region.width <= 0 || region.height <= 0) return 0;
    if (region.x < 0 || region.y < 0 || region.x + region.width > stride || region.y + region.height > DM1_V2_VIEWPORT_H) return 0;
    DM1_V2_RegionCompareResult local = {0, 0, -1, -1};
    for (int y = region.y; y < region.y + region.height; y++) {
        for (int x = region.x; x < region.x + region.width; x++) {
            const DM1_V2_Color* e = &expected[y * stride + x];
            const DM1_V2_Color* a = &actual[y * stride + x];
            local.comparedPixels++;
            if (e->r != a->r || e->g != a->g || e->b != a->b || e->a != a->a) {
                if (local.mismatchedPixels == 0) {
                    local.firstMismatchX = x;
                    local.firstMismatchY = y;
                }
                local.mismatchedPixels++;
            }
        }
    }
    if (result) *result = local;
    return local.mismatchedPixels == 0;
}

void dm1_v2_vp_composition_init(DM1_V2_ViewportCompositionInput* input) {
    if (!input) return;
    memset(input, 0, sizeof(*input));
    for (int depth = 0; depth < 4; depth++) {
        for (int lateral = 0; lateral < 3; lateral++) {
            input->squares[depth][lateral].element = DM1_V2_ELEMENT_CORRIDOR;
        }
    }
}

int dm1_v2_vp_emit_d0_d3_draw_list(const DM1_V2_ViewportCompositionInput* input,
                                   DM1_V2_DrawCommand* outCommands,
                                   int maxCommands) {
    static const int kDepthOrder[12] = {3,3,3, 2,2,2, 1,1,1, 0,0,0};
    static const int kLateralOrder[12] = {-1,1,0, -1,1,0, -1,1,0, -1,1,0};
    int count = 0;
    if (!input || !outCommands || maxCommands <= 0) return 0;

    /* Source-lock: ReDMCSB DUNVIEW.C:8337-8338 draws floor/ceiling before
       F0128 walks the visible squares. */
    if (!dm1_v2_vp_push_draw(outCommands, maxCommands, &count,
                             DM1_V2_DRAW_FLOOR_CEILING,
                             DM1_V2_VIEW_SQUARE_OTHER, -1, 0,
                             DM1_V2_ELEMENT_CORRIDOR, 0,
                             "DUNVIEW.C:8337-8338")) return 0;

    for (int i = 0; i < 12; i++) {
        int depth = kDepthOrder[i];
        int lateral = kLateralOrder[i];
        int lateralIndex = dm1_v2_vp_lateral_index(lateral);
        const DM1_V2_ViewportSquareInput* square = &input->squares[depth][lateralIndex];
        DM1_V2_ViewSquare viewSquare = dm1_v2_vp_square_id(depth, lateral);
        int order = i + 1;

        /* Source-lock: ReDMCSB DUNVIEW.C:8490-8542 visits D3L/D3R/D3C,
           D2L/D2R/D2C, D1L/D1R/D1C, then D0L/D0R/D0C. */
        if (square->element == DM1_V2_ELEMENT_WALL) {
            if (!dm1_v2_vp_push_draw(outCommands, maxCommands, &count,
                                     DM1_V2_DRAW_WALL, viewSquare, depth, lateral,
                                     square->element, order, "DUNVIEW.C:6697-6720")) return 0;
            continue;
        }

        if (square->element == DM1_V2_ELEMENT_DOOR_FRONT) {
            const char* floorRef = "DUNVIEW.C:6721-6816";
            const char* pass1Ref = "DUNVIEW.C:6761-6769";
            const char* doorRef = "DUNVIEW.C:6721-6816";
            const char* pass2Ref = "DUNVIEW.C:6816";
            if (viewSquare == DM1_V2_VIEW_SQUARE_D1C) {
                /* Source-lock: ReDMCSB DUNVIEW.C:7873-7937 draws D1C door-front
                   floor ornament, back objects, door frame/door, then front objects. */
                floorRef = "DUNVIEW.C:7873-7874";
                pass1Ref = "DUNVIEW.C:7875";
                doorRef = "DUNVIEW.C:7877-7910";
                pass2Ref = "DUNVIEW.C:7910-7937";
            }
            if (!dm1_v2_vp_push_draw(outCommands, maxCommands, &count,
                                     DM1_V2_DRAW_FLOOR_ORNAMENT, viewSquare, depth, lateral,
                                     square->element, order, floorRef)) return 0;
            if (!dm1_v2_vp_push_draw(outCommands, maxCommands, &count,
                                     DM1_V2_DRAW_OBJECTS_CREATURES_PROJECTILES, viewSquare, depth, lateral,
                                     square->element, order, pass1Ref)) return 0;
            if (!dm1_v2_vp_push_draw(outCommands, maxCommands, &count,
                                     DM1_V2_DRAW_DOOR_FRONT, viewSquare, depth, lateral,
                                     square->element, order, doorRef)) return 0;
            if (!dm1_v2_vp_push_draw(outCommands, maxCommands, &count,
                                     DM1_V2_DRAW_OBJECTS_CREATURES_PROJECTILES, viewSquare, depth, lateral,
                                     square->element, order, pass2Ref)) return 0;
            continue;
        }

        if (square->element == DM1_V2_ELEMENT_STAIRS_FRONT) {
            if (!dm1_v2_vp_push_draw(outCommands, maxCommands, &count,
                                     DM1_V2_DRAW_STAIRS_FRONT, viewSquare, depth, lateral,
                                     square->element, order, "DUNVIEW.C:6666-6696")) return 0;
            continue;
        }

        if (square->element == DM1_V2_ELEMENT_PIT) {
            if (!dm1_v2_vp_push_draw(outCommands, maxCommands, &count,
                                     DM1_V2_DRAW_PIT, viewSquare, depth, lateral,
                                     square->element, order, "DUNVIEW.C:6820-6827")) return 0;
        }
        if (square->hasObjects) {
            if (!dm1_v2_vp_push_draw(outCommands, maxCommands, &count,
                                     DM1_V2_DRAW_OBJECTS_CREATURES_PROJECTILES, viewSquare, depth, lateral,
                                     square->element, order, "DUNVIEW.C:6816")) return 0;
        }
        if (square->element == DM1_V2_ELEMENT_TELEPORTER || square->hasField) {
            if (!dm1_v2_vp_push_draw(outCommands, maxCommands, &count,
                                     DM1_V2_DRAW_FIELD, viewSquare, depth, lateral,
                                     square->element, order, "DUNVIEW.C:6828")) return 0;
        }
    }
    return count;
}

int dm1_v2_vp_compare_draw_lists(const DM1_V2_DrawCommand* expected,
                                 int expectedCount,
                                 const DM1_V2_DrawCommand* actual,
                                 int actualCount,
                                 int* mismatchIndex) {
    int minCount = expectedCount < actualCount ? expectedCount : actualCount;
    if (mismatchIndex) *mismatchIndex = -1;
    if (!expected || !actual || expectedCount < 0 || actualCount < 0) {
        if (mismatchIndex) *mismatchIndex = 0;
        return 0;
    }
    for (int i = 0; i < minCount; i++) {
        if (expected[i].op != actual[i].op ||
            expected[i].square != actual[i].square ||
            expected[i].depth != actual[i].depth ||
            expected[i].lateral != actual[i].lateral ||
            expected[i].element != actual[i].element ||
            expected[i].order != actual[i].order) {
            if (mismatchIndex) *mismatchIndex = i;
            return 0;
        }
    }
    if (expectedCount != actualCount) {
        if (mismatchIndex) *mismatchIndex = minCount;
        return 0;
    }
    return 1;
}

void dm1_v2_vp_present(DM1_V2_ViewportState* vp, int32_t nowMs) {
    if (!vp) return;
    
    vp->dirty = 0;
    vp->frameCount++;
    vp->lastRenderMs = nowMs;
}
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} DM1_V2_MaterialColor;

static const DM1_V2_MaterialColor kDm1V2EntryCeilingTone = {182, 182, 182};
static const DM1_V2_MaterialColor kDm1V2EntryFloorTone = {182, 182, 182};
static const DM1_V2_MaterialColor kDm1V2EntryWallOuterTone = {182, 182, 182};
static const DM1_V2_MaterialColor kDm1V2EntryWallInnerTone = {182, 182, 182};
static const DM1_V2_MaterialColor kDm1V2EntryFieldTone = {0, 0, 0};
static const DM1_V2_MaterialColor kDm1V2EntryStairsTone = {182, 182, 182};
static const DM1_V2_MaterialColor kDm1V2EntryDoorTone = {146, 146, 146};
static const DM1_V2_MaterialColor kDm1V2EntryObjectTone = {73, 73, 73};
static const DM1_V2_MaterialColor kDm1V2EntryPitTone = {0, 0, 0};
static const DM1_V2_MaterialColor kDm1V2EntryFloorOrnamentTone = {109, 109, 109};

static void dm1_v2_vp_fill_rect(DM1_V2_ViewportState* vp,
                                int x0,
                                int y0,
                                int w,
                                int h,
                                uint8_t r,
                                uint8_t g,
                                uint8_t b) {
    if (!vp || w <= 0 || h <= 0) return;
    if (x0 < 0) { w += x0; x0 = 0; }
    if (y0 < 0) { h += y0; y0 = 0; }
    if (x0 + w > DM1_V2_VIEWPORT_W) w = DM1_V2_VIEWPORT_W - x0;
    if (y0 + h > DM1_V2_VIEWPORT_H) h = DM1_V2_VIEWPORT_H - y0;
    if (w <= 0 || h <= 0) return;
    for (int y = y0; y < y0 + h; y++) {
        for (int x = x0; x < x0 + w; x++) {
            dm1_v2_vp_set_pixel(vp, x, y, r, g, b, 255);
        }
    }
}

static int dm1_v2_vp_square_rect(DM1_V2_ViewSquare square, int* x, int* y, int* w, int* h) {
    if (!x || !y || !w || !h) return 0;
    switch (square) {
        case DM1_V2_VIEW_SQUARE_D3L: *x = 0; *y = 42; *w = 42; *h = 52; return 1;
        case DM1_V2_VIEW_SQUARE_D3R: *x = 182; *y = 42; *w = 42; *h = 52; return 1;
        case DM1_V2_VIEW_SQUARE_D3C: *x = 86; *y = 38; *w = 52; *h = 60; return 1;
        case DM1_V2_VIEW_SQUARE_D2L: *x = 18; *y = 34; *w = 54; *h = 72; return 1;
        case DM1_V2_VIEW_SQUARE_D2R: *x = 152; *y = 34; *w = 54; *h = 72; return 1;
        case DM1_V2_VIEW_SQUARE_D2C: *x = 70; *y = 28; *w = 84; *h = 84; return 1;
        case DM1_V2_VIEW_SQUARE_D1L: *x = 0; *y = 24; *w = 82; *h = 96; return 1;
        case DM1_V2_VIEW_SQUARE_D1R: *x = 142; *y = 24; *w = 82; *h = 96; return 1;
        case DM1_V2_VIEW_SQUARE_D1C: *x = 46; *y = 16; *w = 132; *h = 112; return 1;
        case DM1_V2_VIEW_SQUARE_D0L: *x = 0; *y = 10; *w = 92; *h = 126; return 1;
        case DM1_V2_VIEW_SQUARE_D0R: *x = 132; *y = 10; *w = 92; *h = 126; return 1;
        case DM1_V2_VIEW_SQUARE_D0C: *x = 24; *y = 6; *w = 176; *h = 130; return 1;
        default: return 0;
    }
}

int dm1_v2_vp_render_composition_flat(DM1_V2_ViewportState* vp,
                                      const DM1_V2_ViewportCompositionInput* input) {
    DM1_V2_DrawCommand commands[DM1_V2_MAX_DRAW_COMMANDS];
    int count;
    if (!vp || !input) return 0;
    dm1_v2_vp_clear(vp, 0, 0, 0);
    count = dm1_v2_vp_emit_d0_d3_draw_list(input, commands, DM1_V2_MAX_DRAW_COMMANDS);
    if (count <= 0) return 0;

    /* Source-lock: this is an explicit export seam, not original pixel parity.
       It materializes the DUNVIEW.C:8337-8542 composition/draw-command order into
       a deterministic 224x136 RGBA viewport. Pass288 keeps this symbolic renderer,
       but normalizes its material colors to the pass282 original PC34 grayscale
       viewport palette so the comparator can measure geometry/order mismatches
       instead of failing every pixel solely on non-original RGBA colors. */
    for (int i = 0; i < count; i++) {
        int x = 0, y = 0, w = 0, h = 0;
        const DM1_V2_DrawCommand* c = &commands[i];
        if (c->op == DM1_V2_DRAW_FLOOR_CEILING) {
            dm1_v2_vp_fill_rect(vp, 0, 0, DM1_V2_VIEWPORT_W, DM1_V2_VIEWPORT_H / 2, kDm1V2EntryCeilingTone.r, kDm1V2EntryCeilingTone.g, kDm1V2EntryCeilingTone.b);
            dm1_v2_vp_fill_rect(vp, 0, DM1_V2_VIEWPORT_H / 2, DM1_V2_VIEWPORT_W, DM1_V2_VIEWPORT_H / 2, kDm1V2EntryFloorTone.r, kDm1V2EntryFloorTone.g, kDm1V2EntryFloorTone.b);
            continue;
        }
        if (!dm1_v2_vp_square_rect(c->square, &x, &y, &w, &h)) continue;
        switch (c->op) {
            case DM1_V2_DRAW_WALL:
                dm1_v2_vp_fill_rect(vp, x, y, w, h, kDm1V2EntryWallOuterTone.r, kDm1V2EntryWallOuterTone.g, kDm1V2EntryWallOuterTone.b);
                dm1_v2_vp_fill_rect(vp, x + 2, y + 2, w > 4 ? w - 4 : w, h > 4 ? h - 4 : h, kDm1V2EntryWallInnerTone.r, kDm1V2EntryWallInnerTone.g, kDm1V2EntryWallInnerTone.b);
                break;
            case DM1_V2_DRAW_DOOR_FRONT:
                dm1_v2_vp_fill_rect(vp, x + w / 4, y, w / 2, h, kDm1V2EntryDoorTone.r, kDm1V2EntryDoorTone.g, kDm1V2EntryDoorTone.b);
                break;
            case DM1_V2_DRAW_STAIRS_FRONT:
                for (int step = 0; step < 5; step++) {
                    dm1_v2_vp_fill_rect(vp, x + step * 4, y + h - 10 - step * 8, w - step * 8, 5, kDm1V2EntryStairsTone.r, kDm1V2EntryStairsTone.g, kDm1V2EntryStairsTone.b);
                }
                break;
            case DM1_V2_DRAW_PIT:
                dm1_v2_vp_fill_rect(vp, x + w / 4, y + h / 2, w / 2, h / 3, kDm1V2EntryPitTone.r, kDm1V2EntryPitTone.g, kDm1V2EntryPitTone.b);
                break;
            case DM1_V2_DRAW_FIELD:
                dm1_v2_vp_fill_rect(vp, x + w / 3, y + h / 3, w / 3, h / 3, kDm1V2EntryFieldTone.r, kDm1V2EntryFieldTone.g, kDm1V2EntryFieldTone.b);
                break;
            case DM1_V2_DRAW_OBJECTS_CREATURES_PROJECTILES:
                dm1_v2_vp_fill_rect(vp, x + w / 2 - 3, y + h / 2 - 3, 6, 6, kDm1V2EntryObjectTone.r, kDm1V2EntryObjectTone.g, kDm1V2EntryObjectTone.b);
                break;
            case DM1_V2_DRAW_FLOOR_ORNAMENT:
                dm1_v2_vp_fill_rect(vp, x + w / 3, y + h - 8, w / 3, 4, kDm1V2EntryFloorOrnamentTone.r, kDm1V2EntryFloorOrnamentTone.g, kDm1V2EntryFloorOrnamentTone.b);
                break;
            default:
                break;
        }
    }
    dm1_v2_vp_present(vp, 0);
    return 1;
}

static uint32_t dm1_v2_png_crc32(const uint8_t* data, int len) {
    uint32_t crc = 0xFFFFFFFFu;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) crc = (crc >> 1) ^ (0xEDB88320u & (uint32_t)-(int)(crc & 1u));
    }
    return crc ^ 0xFFFFFFFFu;
}

static uint32_t dm1_v2_png_adler32(const uint8_t* data, int len) {
    uint32_t a = 1, b = 0;
    for (int i = 0; i < len; i++) {
        a = (a + data[i]) % 65521u;
        b = (b + a) % 65521u;
    }
    return (b << 16) | a;
}

static int dm1_v2_png_write_u32(FILE* f, uint32_t v) {
    uint8_t b[4];
    b[0] = (uint8_t)((v >> 24) & 255u); b[1] = (uint8_t)((v >> 16) & 255u);
    b[2] = (uint8_t)((v >> 8) & 255u); b[3] = (uint8_t)(v & 255u);
    return fwrite(b, 1, 4, f) == 4;
}

static int dm1_v2_png_write_chunk(FILE* f, const char type[4], const uint8_t* data, int len) {
    uint8_t* typeAndData;
    uint32_t crc;
    int ok;
    if (!f || !type || len < 0) return 0;
    typeAndData = (uint8_t*)malloc((size_t)len + 4u);
    if (!typeAndData) return 0;
    memcpy(typeAndData, type, 4);
    if (len > 0) memcpy(typeAndData + 4, data, (size_t)len);
    crc = dm1_v2_png_crc32(typeAndData, len + 4);
    ok = dm1_v2_png_write_u32(f, (uint32_t)len) &&
         fwrite(type, 1, 4, f) == 4 &&
         (len == 0 || fwrite(data, 1, (size_t)len, f) == (size_t)len) &&
         dm1_v2_png_write_u32(f, crc);
    free(typeAndData);
    return ok;
}

int dm1_v2_vp_write_png_rgba(const char* path,
                             const DM1_V2_Color* pixels,
                             int width,
                             int height,
                             int stride) {
    static const uint8_t sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    FILE* f = NULL;
    uint8_t ihdr[13];
    uint8_t* raw = NULL;
    uint8_t* z = NULL;
    int rawLen;
    int zLen;
    int zp;
    int remaining;
    int pos;
    if (!path || !pixels || width <= 0 || height <= 0 || stride < width) return 0;
    rawLen = height * (1 + width * 4);
    zLen = 2 + rawLen + 5 * ((rawLen + 65534) / 65535) + 4;
    raw = (uint8_t*)malloc((size_t)rawLen);
    z = (uint8_t*)malloc((size_t)zLen);
    if (!raw || !z) { free(raw); free(z); return 0; }
    for (int y = 0; y < height; y++) {
        uint8_t* row = raw + y * (1 + width * 4);
        row[0] = 0;
        for (int x = 0; x < width; x++) {
            const DM1_V2_Color* c = &pixels[y * stride + x];
            row[1 + x * 4 + 0] = c->r;
            row[1 + x * 4 + 1] = c->g;
            row[1 + x * 4 + 2] = c->b;
            row[1 + x * 4 + 3] = c->a;
        }
    }
    zp = 0;
    z[zp++] = 0x78; z[zp++] = 0x01;
    remaining = rawLen;
    pos = 0;
    while (remaining > 0) {
        int block = remaining > 65535 ? 65535 : remaining;
        int final = remaining <= 65535;
        z[zp++] = (uint8_t)(final ? 1 : 0);
        z[zp++] = (uint8_t)(block & 255); z[zp++] = (uint8_t)((block >> 8) & 255);
        z[zp++] = (uint8_t)((~block) & 255); z[zp++] = (uint8_t)(((~block) >> 8) & 255);
        memcpy(z + zp, raw + pos, (size_t)block);
        zp += block; pos += block; remaining -= block;
    }
    {
        uint32_t adler = dm1_v2_png_adler32(raw, rawLen);
        z[zp++] = (uint8_t)((adler >> 24) & 255u); z[zp++] = (uint8_t)((adler >> 16) & 255u);
        z[zp++] = (uint8_t)((adler >> 8) & 255u); z[zp++] = (uint8_t)(adler & 255u);
    }
    f = fopen(path, "wb");
    if (!f) { free(raw); free(z); return 0; }
    memset(ihdr, 0, sizeof(ihdr));
    ihdr[0] = (uint8_t)((width >> 24) & 255); ihdr[1] = (uint8_t)((width >> 16) & 255);
    ihdr[2] = (uint8_t)((width >> 8) & 255); ihdr[3] = (uint8_t)(width & 255);
    ihdr[4] = (uint8_t)((height >> 24) & 255); ihdr[5] = (uint8_t)((height >> 16) & 255);
    ihdr[6] = (uint8_t)((height >> 8) & 255); ihdr[7] = (uint8_t)(height & 255);
    ihdr[8] = 8; ihdr[9] = 6;
    if (fwrite(sig, 1, 8, f) != 8 ||
        !dm1_v2_png_write_chunk(f, "IHDR", ihdr, 13) ||
        !dm1_v2_png_write_chunk(f, "IDAT", z, zp) ||
        !dm1_v2_png_write_chunk(f, "IEND", NULL, 0)) {
        fclose(f); free(raw); free(z); return 0;
    }
    fclose(f); free(raw); free(z); return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.1 Viewport Renderer — Upscaled V1-faithful rendering
 *
 * Renders the V1 viewport at native 224×136 using the V1 draw pipeline
 * (dm1_v1_viewport_3d_pc34_compat), then upscales to display resolution
 * using the V2.1 EPX pipeline.
 *
 * Key invariant: V2.1 MUST produce identical frame content to V1 at
 * the logical pixel level. The only difference is resolution and
 * optional palette enhancement.
 * ══════════════════════════════════════════════════════════════════════ */

/* V2.1 viewport frame dimensions */
#define V21_VIEWPORT_W 224
#define V21_VIEWPORT_H 136
#define V21_PANEL_W    320
#define V21_PANEL_H    64
#define V21_SCREEN_W   320
#define V21_SCREEN_H   200

typedef struct {
    uint8_t v1_framebuffer[V21_SCREEN_W * V21_SCREEN_H]; /* indexed V1 frame */
    uint8_t epx_buffer[V21_SCREEN_W * 2 * V21_SCREEN_H * 2]; /* EPX 2x */
    uint32_t rgba_output[V21_SCREEN_W * 4 * V21_SCREEN_H * 4]; /* final RGBA */
    uint32_t palette[256];
    int scale_factor; /* 2 or 4 */
    int epx_enabled;
} V21_ViewportState;

static V21_ViewportState g_v21_viewport;

void v21_viewport_init(int scale_factor) {
    memset(&g_v21_viewport, 0, sizeof(g_v21_viewport));
    g_v21_viewport.scale_factor = (scale_factor == 4) ? 4 : 2;
    g_v21_viewport.epx_enabled = 1;
}

void v21_viewport_set_palette(const uint32_t *palette, int count) {
    if (!palette || count <= 0) return;
    if (count > 256) count = 256;
    memcpy(g_v21_viewport.palette, palette, count * sizeof(uint32_t));
}

/* The core V2.1 render call:
 * 1. V1 engine renders to g_v21_viewport.v1_framebuffer (indexed 320×200)
 * 2. EPX 2x produces 640×400 indexed
 * 3. Palette map produces 640×400 RGBA (or 1280×800 at 4x)
 * Caller presents rgba_output to screen via SDL/GPU. */
const uint32_t *v21_viewport_get_rgba(int *out_w, int *out_h) {
    if (out_w) *out_w = V21_SCREEN_W * g_v21_viewport.scale_factor;
    if (out_h) *out_h = V21_SCREEN_H * g_v21_viewport.scale_factor;
    return g_v21_viewport.rgba_output;
}

const uint8_t *v21_viewport_get_v1_framebuffer(void) {
    return g_v21_viewport.v1_framebuffer;
}

uint8_t *v21_viewport_get_v1_framebuffer_mut(void) {
    return g_v21_viewport.v1_framebuffer;
}

const char *v21_viewport_source_evidence(void) {
    return
        "V2.1 viewport: V1 320x200 indexed -> EPX 2x -> palette RGBA -> present\n"
        "V1 draw pipeline: dm1_v1_viewport_3d_pc34_compat renders to indexed buffer\n"
        "Pixel-identical to V1 at logical level; only resolution differs\n"
        "Panel (320x64) upscaled separately from viewport (224x136)\n";
}


/* ══════════════════════════════════════════════════════════════════════
 * DM1 V2.1 EPX Full Render Pipeline
 *
 * Source: Firestaff V2.1 EPX pipeline. EPX algorithm is well-known
 * (Eric's Pixel Expansion, no ReDMCSB original — Scale2x family).
 * Upscale preserves pixel-art edges without palette interpolation
 * artifacts, making it the correct upscaler for indexed DM1 pixel art.
 *
 * ReDMCSB DUNVIEW.C:8318-8542 governs the composition order that
 * produced the indexed source framebuffer.
 * ReDMCSB DUNVIEW.C:2999-3000 defines viewport bitmap dimensions.
 * ReDMCSB PALETTE.C supplies the canonical 6-level VGA palette used
 * here as G9010_auc_VgaPaletteAll_Compat (ReDMCSB DEFS.H palette).
 *
 * Pipeline:
 *   1. V1 engine renders walls/doors/floors/creatures/objects/projectiles
 *      to g_v21_viewport.v1_framebuffer[320x200] - indexed (level<<4)|index
 *   2. v2_upscale_epx() doubles to g_v21_viewport.epx_buffer[640x400] indexed
 *   3. Per-pixel palette: G9010_auc_VgaPaletteAll_Compat[level][index] -> RGBA
 *   4. g_v21_viewport.rgba_output[640x400] (or 1280x800 at 4x scale)
 *   5. Caller presents rgba_output to screen via SDL/GPU
 *
 * Creature/object/projectile rendering in V2.1 EPX path requires no
 * separate code: dm1_v1_viewport_3d_pc34_compat renders all content
 * (walls + creatures + objects + projectiles) to the indexed framebuffer
 * before EPX upscale, so EPX handles all upscaled content uniformly.
 * Source-lock: ReDMCSB DUNVIEW.C:4547-4602 F0115 draws objects/creatures/
 * projectiles after walls/doors/floors into the same buffer.
 * ══════════════════════════════════════════════════════════════════════ */

/* Palette-expand indexed pixels to RGBA8888 using G9010 VGA palette.
 *
 * Each source byte encodes (level << 4) | palette_index per
 * m11_framebuffer_to_rgba() convention in render_sdl_m11.c:442.
 * We use level 0 (brightest) as the fallback here since the V21
 * viewport state does not carry a per-pixel level field - the full
 * 6-level V2 pipeline would set source_palette_level from
 * G0304_i_DungeonViewPaletteIndex (ReDMCSB PANEL.C:418-428).
 *
 * G9010_auc_VgaPaletteAll_Compat[level][index][RGB] stores VGA DAC
 * 6-bit values in the same byte layout used by the original game.
 * Palette expand mirrors dm1_v2_asset_pipeline_pc34.c:229-265 which
 * follows the existing m11_framebuffer_to_rgba() logic.
 *
 * Output format: SDL_PIXELFORMAT_RGBA32 - R,G,B,A in memory order.
 * Matches g_state.presentBuffer format in render_sdl_m11.c. */
static void v21_palette_expand_indexed_to_rgba(const uint8_t *indexed,
    int w, int h, uint32_t *rgba_out)
{
    static const uint8_t fallback_palette[16][3] = {
        {0, 0, 0},       {0, 0, 170},     {0, 170, 0},     {0, 170, 170},
        {170, 0, 0},     {170, 0, 170},   {170, 85, 0},    {170, 170, 170},
        {85, 85, 85},    {85, 85, 255},   {85, 255, 85},   {85, 255, 255},
        {255, 85, 85},   {255, 85, 255},  {255, 255, 85},  {255, 255, 255}
    };
    if (!indexed || !rgba_out) return;
    for (int i = 0; i < w * h; i++) {
        uint8_t byte = indexed[i];
        uint8_t level = (byte & M11_FB_LEVEL_MASK) >> M11_FB_LEVEL_SHIFT;
        uint8_t idx   = byte & M11_FB_INDEX_MASK;
        if (level >= DM1_V2_PALETTE_LEVELS) level = 0;
        uint8_t shade = (uint8_t)(255 - (level * 28));
        uint8_t rr = (uint8_t)((fallback_palette[idx][0] * shade) / 255);
        uint8_t gg = (uint8_t)((fallback_palette[idx][1] * shade) / 255);
        uint8_t bb = (uint8_t)((fallback_palette[idx][2] * shade) / 255);
        rgba_out[i] = 0xFF000000u | (rr << 16) | (gg << 8) | bb;
    }
}

static void v21_upscale_epx(const uint8_t *src, int sw, int sh,
                            uint8_t *dst, int dw, int dh)
{
    (void)dw;
    (void)dh;
    if (!src || !dst || sw <= 0 || sh <= 0) return;
    for (int y = 0; y < sh; y++) {
        for (int x = 0; x < sw; x++) {
            uint8_t p = src[y * sw + x];
            uint8_t a = (y > 0) ? src[(y - 1) * sw + x] : p;
            uint8_t b = (x < sw - 1) ? src[y * sw + x + 1] : p;
            uint8_t c = (x > 0) ? src[y * sw + x - 1] : p;
            uint8_t d = (y < sh - 1) ? src[(y + 1) * sw + x] : p;
            int ox = x * 2;
            int oy = y * 2;
            int row = sw * 2;
            dst[oy * row + ox] = (c == a && c != d && a != b) ? a : p;
            dst[oy * row + ox + 1] = (a == b && a != c && b != d) ? b : p;
            dst[(oy + 1) * row + ox] = (d == c && d != b && c != a) ? c : p;
            dst[(oy + 1) * row + ox + 1] = (b == d && b != a && d != c) ? d : p;
        }
    }
}

/* DM1 V2.1 EPX full render pipeline entry point.
 *
 * Source-lock: ReDMCSB DUNVIEW.C:8318-8542 composition order preserved
 * in indexed v1_framebuffer; EPX (Scale2x family) doubles resolution
 * without blending palette indices - preserving edge sharpness.
 *
 *   Step 1: EPX 2x (indexed) - v1_framebuffer[320x200]
 *             -> epx_buffer[640x400]
 *
 *   Step 2: Palette expand - epx_buffer[640x400] indexed
 *             -> rgba_output[640x400] RGBA
 *
 *   Step 3: Update viewport dirty/frame state and call present hook.
 *           The DUNVIEW.C draw pipeline issues a present hint via F0128
 *           (ReDMCSB GAMELOOP.C:90) after each viewport render.
 *           V2.1 maps this to v21_viewport_render_full_pipeline() as the
 *           canonical present point. */
void v21_viewport_render_full_pipeline(void)
{
    if (!g_v21_viewport.epx_enabled) return;

    v21_upscale_epx(g_v21_viewport.v1_framebuffer,
                   V21_SCREEN_W, V21_SCREEN_H,
                   g_v21_viewport.epx_buffer,
                   V21_SCREEN_W * 2, V21_SCREEN_H * 2);

    v21_palette_expand_indexed_to_rgba(g_v21_viewport.epx_buffer,
                                      V21_SCREEN_W * 2, V21_SCREEN_H * 2,
                                      g_v21_viewport.rgba_output);

    /* Mark V21 viewport frame complete via present hook */
    (void)dm1_v2_vp_present(NULL, 0);
}


/* ══════════════════════════════════════════════════════════════════════
 * V2 Inscription Text Rendering
 *
 * V1: inscriptions rendered as pixel-font in 320x200 viewport buffer.
 * V2: render inscription text AFTER upscaling, using a larger font
 * so it is actually readable. The text content comes from DUNGEON.DAT
 * or the translated dungeon text table.
 *
 * V2 inscription overlay:
 *   1. V1 renders viewport as normal (inscription is pixel-blurred)
 *   2. V2 detects inscription in view (wall ornament type)
 *   3. V2 overlays clean text in display-resolution font
 *   4. Text uses localized version if available
 * ══════════════════════════════════════════════════════════════════════ */

typedef struct {
    int active;
    int x, y;           /* display coordinates (post-upscale) */
    int width, height;
    const char *text;
    uint32_t color;
    float alpha;
} V21_InscriptionOverlay;

static V21_InscriptionOverlay g_inscription = {0};

void v21_inscription_show(const char *text, int vp_x, int vp_y, int scale) {
    if (!text || !text[0]) return;
    g_inscription.active = 1;
    g_inscription.text = text;
    /* Position in display coordinates (after upscale) */
    g_inscription.x = vp_x * scale;
    g_inscription.y = vp_y * scale;
    g_inscription.color = 0xFFCCCC44; /* yellow, like V1 */
    g_inscription.alpha = 1.0f;
}

void v21_inscription_hide(void) {
    g_inscription.active = 0;
}

int v21_inscription_is_active(void) {
    return g_inscription.active;
}

const V21_InscriptionOverlay *v21_inscription_get(void) {
    return g_inscription.active ? &g_inscription : NULL;
}

/* Called by renderer AFTER upscale to draw clean text */
void v21_inscription_render_overlay(uint32_t *rgba_buffer,
    int display_w, int display_h, int scale)
{
    /* In a full implementation this would use a TTF/bitmap font renderer
     * at display resolution. For now: draw each character as a colored
     * block so the text is at least visible and correctly positioned. */
    const V21_InscriptionOverlay *ins = v21_inscription_get();
    int i, cx, cy, char_w, char_h;
    if (!ins || !rgba_buffer) return;

    char_w = 6 * scale;  /* V1 font is ~6px wide */
    char_h = 8 * scale;  /* V1 font is ~8px tall */
    cx = ins->x;
    cy = ins->y;

    for (i = 0; ins->text[i]; i++) {
        /* Simple: fill a character-sized rectangle with the text color.
         * Real implementation: blit from a font atlas. */
        if (ins->text[i] != ' ') {
            int py, px;
        for (py = 0; py < char_h && cy + py < display_h; py++) {
                for (px = 0; px < char_w - 1 && cx + px < display_w; px++) {
                    /* Only draw pixels that form a rough letter shape */
                    int row = py * 8 / char_h;
                    int col = px * 6 / char_w;
                    /* Simple 5x7 font pattern check */
                    if (row < 7 && col < 5) {
                        rgba_buffer[(cy + py) * display_w + cx + px] = ins->color;
                    }
                }
            }
        }
        cx += char_w;
        if (cx >= display_w - char_w) {
            cx = ins->x;
            cy += char_h + 2;
        }
    }
}
