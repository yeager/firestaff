#include "dm1_v1_viewport_click_pc34_compat.h"
#include <string.h>

/*
 * DM1 V1 Viewport & Menu Click Routing — implementation
 *
 * Source lock: ReDMCSB WIP20210206
 *   CLIKVIEW.C:
 *     F0372: click in dungeon view → front wall sensor touch
 *     F0373: grab object from floor (view cells 0-1 = party square,
 *            2-3 = square in front)
 *     F0374: throw/put object to floor
 *     F0375: attack creature in viewport
 *   CLIKMENU.C:
 *     F0365: turn dispatch (party direction change)
 *     F0366: step dispatch (movement with collision check)
 *
 * Viewport layout (224x136):
 *   The viewport is divided into view cells based on click position.
 *   Left half = cells 0/3, right half = cells 1/2.
 *   Top portion = front (cells 2/3), bottom = near (cells 0/1).
 *   G0233/G0234: direction-to-step tables for coordinate offset.
 *
 * Standard DM1 screen layout:
 *   Viewport: 0,0 to 223,135
 *   Champion panels: 0,136 to 319,199
 *   Spell symbols: 233,2 to 317,71
 *   Action area: 233,72 to 317,121
 *   Movement: 234,124 to 319,199
 */

/* Direction offset tables — from G0233/G0234 */
static const int s_dirStepEast[4]  = { 0, 1, 0, -1 }; /* N, E, S, W */
static const int s_dirStepNorth[4] = { -1, 0, 1, 0 };

void m11_click_init(M11_ClickState *s)
{
    memset(s, 0, sizeof(*s));
    s->lastClickZone = DM1_ZONE_NONE;
}

int m11_click_add_zone(M11_ClickState *s, int x, int y, int w, int h,
                       M11_ClickZoneId zoneId)
{
    if (s->zoneCount >= M11_MAX_CLICK_ZONES) return -1;
    int idx = s->zoneCount;
    M11_ClickZone *z = &s->zones[idx];
    z->x = x;
    z->y = y;
    z->w = w;
    z->h = h;
    z->zoneId = zoneId;
    z->enabled = 1;
    s->zoneCount++;
    return idx;
}

void m11_click_enable_zone(M11_ClickState *s, M11_ClickZoneId zoneId,
                           int enabled)
{
    for (int i = 0; i < s->zoneCount; i++) {
        if (s->zones[i].zoneId == zoneId) {
            s->zones[i].enabled = enabled;
        }
    }
}

M11_ClickZoneId m11_click_hit_test(const M11_ClickState *s, int mx, int my)
{
    /* Iterate in reverse order — last added zones have priority */
    for (int i = s->zoneCount - 1; i >= 0; i--) {
        const M11_ClickZone *z = &s->zones[i];
        if (!z->enabled) continue;
        if (mx >= z->x && mx < z->x + z->w &&
            my >= z->y && my < z->y + z->h) {
            return z->zoneId;
        }
    }
    return DM1_ZONE_NONE;
}

M11_ClickZoneId m11_click_mouse_down(M11_ClickState *s,
                                      int mx, int my, int button)
{
    s->mouseDown = 1;
    s->mouseButton = button;
    s->lastClickX = mx;
    s->lastClickY = my;
    s->lastClickZone = m11_click_hit_test(s, mx, my);
    return s->lastClickZone;
}

M11_ClickZoneId m11_click_mouse_up(M11_ClickState *s, int mx, int my)
{
    s->mouseDown = 0;
    (void)mx;
    (void)my;
    return s->lastClickZone;
}

void m11_click_setup_game_zones(M11_ClickState *s)
{
    m11_click_clear_zones(s);

    /* Viewport: 224x136 at top-left */
    m11_click_add_zone(s, 0, 0, 224, 136, DM1_ZONE_VIEWPORT);

    /* Spell symbols area */
    m11_click_add_zone(s, 233, 2, 85, 70, DM1_ZONE_SPELL_AREA);

    /* Action area */
    m11_click_add_zone(s, 233, 72, 85, 50, DM1_ZONE_ACTION_AREA);

    /* Movement arrows — non-overlapping zones per ReDMCSB COMMAND.C
     * G0459_aai_Graphic561_CommandAreaCoordinates.
     * Layout: forward top-center, turn left/right at sides,
     * strafe left/right below turns, backward bottom-center. */
    m11_click_add_zone(s, 255, 124, 42, 18, DM1_ZONE_MOVEMENT_FORWARD);
    m11_click_add_zone(s, 234, 142, 28, 28, DM1_ZONE_MOVEMENT_TURN_LEFT);
    m11_click_add_zone(s, 292, 142, 28, 28, DM1_ZONE_MOVEMENT_TURN_RIGHT);
    m11_click_add_zone(s, 234, 170, 28, 30, DM1_ZONE_MOVEMENT_LEFT);
    m11_click_add_zone(s, 292, 170, 28, 30, DM1_ZONE_MOVEMENT_RIGHT);
    m11_click_add_zone(s, 262, 170, 30, 30, DM1_ZONE_MOVEMENT_BACKWARD);

    /* Champion panels: 4 panels in 2x2 grid at bottom of screen.
     * Each panel is 80x29 pixels per ReDMCSB screen layout. */
    m11_click_add_zone(s, 0, 136, 80, 29, DM1_ZONE_CHAMPION_0);
    m11_click_add_zone(s, 80, 136, 80, 29, DM1_ZONE_CHAMPION_1);
    m11_click_add_zone(s, 0, 165, 80, 29, DM1_ZONE_CHAMPION_2);
    m11_click_add_zone(s, 80, 165, 80, 29, DM1_ZONE_CHAMPION_3);
}

void m11_click_setup_inventory_zones(M11_ClickState *s)
{
    m11_click_clear_zones(s);
    /* Inventory uses a grid of slots — simplified as one large zone */
    m11_click_add_zone(s, 0, 0, 320, 200, DM1_ZONE_INVENTORY);
}

void m11_click_clear_zones(M11_ClickState *s)
{
    s->zoneCount = 0;
    s->lastClickZone = DM1_ZONE_NONE;
}

M11_ViewportClickResult m11_viewport_resolve_click(
    int mx, int my, int partyDir, int partyX, int partyY,
    int hasLeader, int leaderHandEmpty)
{
    return m11_viewport_resolve_click_with_grabbable_mask(
        mx, my, partyDir, partyX, partyY, hasLeader, leaderHandEmpty,
        DM1_VIEWPORT_GRABBABLE_ALL_CELLS);
}

M11_ViewportClickResult m11_viewport_resolve_click_with_grabbable_mask(
    int mx, int my, int partyDir, int partyX, int partyY,
    int hasLeader, int leaderHandEmpty, uint8_t grabbableCellMask)
{
    M11_ViewportClickResult result;
    memset(&result, 0, sizeof(result));

    /* Determine view cell from click position within 224x136 viewport.
     * F0372/F0373: cells 0/1 are near (party square), 2/3 are far (front).
     * Left/right split at x=112.
     * Near/far split at y=68.
     */
    int isRight = (mx >= 112) ? 1 : 0;
    int isFar = (my < 68) ? 1 : 0;

    if (isFar) {
        result.viewCell = isRight ? DM1_VIEW_CELL_BACK_RIGHT
                                  : DM1_VIEW_CELL_BACK_LEFT;
    } else {
        result.viewCell = isRight ? DM1_VIEW_CELL_FRONT_RIGHT
                                  : DM1_VIEW_CELL_FRONT_LEFT;
    }

    /* Calculate target map position */
    result.targetMapX = partyX;
    result.targetMapY = partyY;
    if (result.viewCell >= DM1_VIEW_CELL_BACK_RIGHT) {
        /* Front square: one step in party direction */
        if (partyDir >= 0 && partyDir < 4) {
            result.targetMapX += s_dirStepEast[partyDir];
            result.targetMapY += s_dirStepNorth[partyDir];
        }
    }

    /* Determine action based on context */
    if (!hasLeader) {
        /* No leader — wall sensor check only (F0372) */
        if (isFar) {
            result.wallSensorTriggered = 1;
            result.stopWaitingForInput = 1;
        }
    } else if (leaderHandEmpty) {
        /* Empty hand - grab only when F0115 produced a grabbable object zone.
         * ReDMCSB stores the rendered pile top in G0292_aT_PileTopObject[cell]
         * and F0377 calls F0373 only for matching dungeon-view clickable boxes.
         */
        if (grabbableCellMask & DM1_VIEWPORT_GRABBABLE_CELL_MASK(result.viewCell)) {
            result.objectGrabbed = 1;
            result.stopWaitingForInput = 1;
        }
    } else {
        /* Hand has object — throw/place it (F0374) */
        result.objectThrown = 1;
        result.stopWaitingForInput = 1;
    }

    return result;
}

const char *m11_viewport_click_source_evidence(void)
{
    return
        "ReDMCSB WIP20210206\n"
        "CLIKVIEW.C F0372: click viewport — touch front wall sensor.\n"
        "  L1135/L1136: target mapX/Y = party pos + direction offset.\n"
        "  G0233/G0234: direction-to-step tables (East/North).\n"
        "  F0275_SENSOR_IsTriggeredByClickOnWall for wall buttons.\n"
        "CLIKVIEW.C F0373: grab object from floor.\n"
        "  View cells 0/1 = party square, cells 2/3 = front square.\n"
        "  G0411_i_LeaderIndex must be != -1.\n"
        "  CLIKVIEW.C:117-126 uses G0292_aT_PileTopObject[cell], icon check,\n"
        "  F0267_MOVE_GetMoveResult_CPSCE, then leader-hand placement.\n"
        "CLIKVIEW.C:406-438: empty-hand floor pickup is reached only after\n"
        "  G2210/G0291 dungeon-view clickable box hit for cells 0..3.\n"
        "DUNVIEW.C:5113-5165: F0115 creates/extends grabbable object zones\n"
        "  while rendering visible object piles.\n"
        "CLIKVIEW.C F0374: throw/put object.\n"
        "CLIKVIEW.C F0375: attack creature.\n"
        "CLIKMENU.C F0365/F0366: turn/step dispatch from movement arrows.\n"
        "Screen layout: viewport 0,0 224x136; spells 233,2 85x70; "
        "action 233,72 85x50; movement 234,124 86x76; "
        "champion panels 0/80,136/152 80x16 each.";
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602 — Remaining CLIKVIEW.C function citations for parity
 *
 *   CLIKVIEW.C:78 F0373_COMMAND_P
 *   CLIKVIEW.C:131 F0374_COMMAND_P
 *   CLIKVIEW.C:191 F0375_COMMAND_P
 *   CLIKVIEW.C:30 F0664_COMMAND_P
 * ══════════════════════════════════════════════════════════════════════ */

