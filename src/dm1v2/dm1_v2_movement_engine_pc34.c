#include "dm1_v2_movement_engine_pc34.h"

void dm1_v2_pos_init(DM1_V2_PlayerPos* p, int x, int y, int dir) {
    if (!p) return;
    p->xPixel = (int16_t)(x * DM1_V2_SUBPIXEL_SCALE);
    p->yPixel = (int16_t)(y * DM1_V2_SUBPIXEL_SCALE);
    p->xSub = 0;
    p->ySub = 0;
    p->facingDir = (int16_t)dir;
    p->moveState = 0;
    p->lastMoveMs = 0;
    p->prevX = (int16_t)x;
    p->prevY = (int16_t)y;
}

int16_t dm1_v2_get_x(const DM1_V2_PlayerPos* p) {
    if (!p) return 0;
    return p->xPixel + p->xSub;
}

int16_t dm1_v2_get_y(const DM1_V2_PlayerPos* p) {
    if (!p) return 0;
    return p->yPixel + p->ySub;
}

void dm1_v2_set_subpixel(DM1_V2_PlayerPos* p, int16_t xsub, int16_t ysub) {
    if (!p) return;
    if (xsub < 0) xsub = 0;
    if (xsub >= DM1_V2_SUBPIXEL_SCALE) xsub = DM1_V2_SUBPIXEL_SCALE - 1;
    if (ysub < 0) ysub = 0;
    if (ysub >= DM1_V2_SUBPIXEL_SCALE) ysub = DM1_V2_SUBPIXEL_SCALE - 1;
    p->xSub = xsub;
    p->ySub = ysub;
}

void dm1_v2_move_step(DM1_V2_PlayerPos* p, const DM1_V2_MoveParams* params, int dir, int32_t dtMs) {
    if (!p || !params) return;

    if (p->moveState == 0 && dtMs > 0) {
        int32_t distance = (params->moveSpeed * dtMs) / 1000;
        
        // Apply subpixel acceleration logic if defined
        if (params->subPixelAccel > 0) {
             distance += params->subPixelAccel;
        }

        // Determine movement vector based on direction (0..7)
        int dx = 0;
        int dy = 0;
        
        // Simple 8-way direction mapping
        // 0: East, 1: SE, 2: South, 3: SW, 4: West, 5: NW, 6: North, 7: NE
        switch (dir % 8) {
            case 0: dx = 1; dy = 0; break;
            case 1: dx = 1; dy = 1; break;
            case 2: dx = 0; dy = 1; break;
            case 3: dx = -1; dy = 1; break;
            case 4: dx = -1; dy = 0; break;
            case 5: dx = -1; dy = -1; break;
            case 6: dx = 0; dy = -1; break;
            case 7: dx = 1; dy = -1; break;
        }

        // Update subpixels
        int32_t newXSub = p->xSub + (dx * distance);
        int32_t newYSub = p->ySub + (dy * distance);

        // Handle pixel advancement and wrapping
        while (newXSub >= DM1_V2_SUBPIXEL_SCALE) {
            p->xPixel += 1;
            newXSub -= DM1_V2_SUBPIXEL_SCALE;
        }
        while (newXSub < 0) {
            p->xPixel -= 1;
            newXSub += DM1_V2_SUBPIXEL_SCALE;
        }

        while (newYSub >= DM1_V2_SUBPIXEL_SCALE) {
            p->yPixel += 1;
            newYSub -= DM1_V2_SUBPIXEL_SCALE;
        }
        while (newYSub < 0) {
            p->yPixel -= 1;
            newYSub += DM1_V2_SUBPIXEL_SCALE;
        }

        p->xSub = (int16_t)newXSub;
        p->ySub = (int16_t)newYSub;

        p->lastMoveMs = dtMs;
        p->moveState = 1;
        p->prevX = (int16_t)(p->xPixel + p->xSub);
        p->prevY = (int16_t)(p->yPixel + p->ySub);
    }
}

void dm1_v2_turn(DM1_V2_PlayerPos* p, int turnDir) {
    if (!p) return;
    
    if (turnDir == -1) {
        p->facingDir--;
        if (p->facingDir < 0) p->facingDir = 7;
    } else if (turnDir == 1) {
        p->facingDir++;
        if (p->facingDir > 7) p->facingDir = 0;
    }
}

int dm1_v2_has_moved(const DM1_V2_PlayerPos* p) {
    if (!p) return 0;
    return p->moveState == 1;
}

int dm1_v2_snap_to_grid(DM1_V2_PlayerPos* p) {
    if (!p) return 0;
    
    int snapped = 0;
    int threshold = 16; // Snap if within 16 subpixels of boundary

    // Snap X
    if (p->xSub < threshold) {
        p->xSub = 0;
        snapped = 1;
    } else if (p->xSub > (DM1_V2_SUBPIXEL_SCALE - threshold)) {
        p->xSub = 0;
        p->xPixel += 1;
        snapped = 1;
    }

    // Snap Y
    if (p->ySub < threshold) {
        p->ySub = 0;
        snapped = 1;
    } else if (p->ySub > (DM1_V2_SUBPIXEL_SCALE - threshold)) {
        p->ySub = 0;
        p->yPixel += 1;
        snapped = 1;
    }

    return snapped;
}

int dm1_v2_collides_at(int px, int py, const int8_t* map, int mapW, int mapH) {
    if (!map) return 1; // Assume collision if no map

    // Bounds check
    if (px < 0 || px >= mapW || py < 0 || py >= mapH) {
        return 1; // Out of bounds is a collision
    }

    int index = px * mapH + py; /* column-major per ReDMCSB DUNGEON.C F0151 */
    int cell = map[index];

    // Wall if >= 1
    if (cell >= 1) {
        return 1;
    }

    return 0;
}