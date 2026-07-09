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

static int is_valid_view_cell(DM1_V1_ViewCellPc34 cell)
{
    return cell >= DM1_VIEW_CELL_FRONT_LEFT && cell < DM1_VIEW_CELL_COUNT;
}

void DM1_V1_ViewportGrabbable_ClearPc34Compat(DM1_V1_ViewportGrabbableStatePc34 *state)
{
    if (!state) return;
    state->grabbableCellMask = DM1_VIEWPORT_GRABBABLE_NO_CELLS;
    for (int i = 0; i < DM1_VIEW_CELL_COUNT; ++i) {
        state->pileTopObjectId[i] = DM1_VIEWPORT_NO_PILE_TOP_OBJECT;
    }
}

void DM1_V1_ViewportGrabbable_InitPc34Compat(DM1_V1_ViewportGrabbableStatePc34 *state)
{
    DM1_V1_ViewportGrabbable_ClearPc34Compat(state);
}

int DM1_V1_ViewportGrabbable_SetPileTopPc34Compat(DM1_V1_ViewportGrabbableStatePc34 *state,
                                        DM1_V1_ViewCellPc34 cell,
                                        int pileTopObjectId)
{
    if (!state || !is_valid_view_cell(cell)) return 0;
    state->pileTopObjectId[cell] = pileTopObjectId;
    if (pileTopObjectId == DM1_VIEWPORT_NO_PILE_TOP_OBJECT) {
        state->grabbableCellMask &=
            (uint8_t)~DM1_VIEWPORT_GRABBABLE_CELL_MASK(cell);
    } else {
        state->grabbableCellMask |= DM1_VIEWPORT_GRABBABLE_CELL_MASK(cell);
    }
    return 1;
}

int DM1_V1_ViewportGrabbable_PileTopPc34Compat(
    const DM1_V1_ViewportGrabbableStatePc34 *state, DM1_V1_ViewCellPc34 cell)
{
    if (!state || !is_valid_view_cell(cell)) {
        return DM1_VIEWPORT_NO_PILE_TOP_OBJECT;
    }
    if (!(state->grabbableCellMask & DM1_VIEWPORT_GRABBABLE_CELL_MASK(cell))) {
        return DM1_VIEWPORT_NO_PILE_TOP_OBJECT;
    }
    return state->pileTopObjectId[cell];
}

void DM1_V1_Click_InitPc34Compat(DM1_V1_ClickStatePc34 *s)
{
    memset(s, 0, sizeof(*s));
    s->lastClickZone = DM1_ZONE_NONE;
}

int DM1_V1_Click_AddZonePc34Compat(DM1_V1_ClickStatePc34 *s, int x, int y, int w, int h,
                       DM1_V1_ClickZoneIdPc34 zoneId)
{
    if (s->zoneCount >= DM1_V1_MAX_CLICK_ZONES_PC34) return -1;
    int idx = s->zoneCount;
    DM1_V1_ClickZonePc34 *z = &s->zones[idx];
    z->x = x;
    z->y = y;
    z->w = w;
    z->h = h;
    z->zoneId = zoneId;
    z->enabled = 1;
    s->zoneCount++;
    return idx;
}

void DM1_V1_Click_EnableZonePc34Compat(DM1_V1_ClickStatePc34 *s, DM1_V1_ClickZoneIdPc34 zoneId,
                           int enabled)
{
    for (int i = 0; i < s->zoneCount; i++) {
        if (s->zones[i].zoneId == zoneId) {
            s->zones[i].enabled = enabled;
        }
    }
}

DM1_V1_ClickZoneIdPc34 DM1_V1_Click_HitTestPc34Compat(const DM1_V1_ClickStatePc34 *s, int mx, int my)
{
    /* Iterate in reverse order — last added zones have priority */
    for (int i = s->zoneCount - 1; i >= 0; i--) {
        const DM1_V1_ClickZonePc34 *z = &s->zones[i];
        if (!z->enabled) continue;
        if (mx >= z->x && mx < z->x + z->w &&
            my >= z->y && my < z->y + z->h) {
            return z->zoneId;
        }
    }
    return DM1_ZONE_NONE;
}

DM1_V1_ClickZoneIdPc34 DM1_V1_Click_MouseDownPc34Compat(DM1_V1_ClickStatePc34 *s,
                                      int mx, int my, int button)
{
    s->mouseDown = 1;
    s->mouseButton = button;
    s->lastClickX = mx;
    s->lastClickY = my;
    s->lastClickZone = DM1_V1_Click_HitTestPc34Compat(s, mx, my);
    return s->lastClickZone;
}

DM1_V1_ClickZoneIdPc34 DM1_V1_Click_MouseUpPc34Compat(DM1_V1_ClickStatePc34 *s, int mx, int my)
{
    s->mouseDown = 0;
    (void)mx;
    (void)my;
    return s->lastClickZone;
}

void DM1_V1_Click_SetupGameZonesPc34Compat(DM1_V1_ClickStatePc34 *s)
{
    DM1_V1_Click_ClearZonesPc34Compat(s);

    /* Viewport: 224x136 at top-left */
    DM1_V1_Click_AddZonePc34Compat(s, 0, 0, 224, 136, DM1_ZONE_VIEWPORT);

    /* Spell symbols area */
    DM1_V1_Click_AddZonePc34Compat(s, 233, 2, 85, 70, DM1_ZONE_SPELL_AREA);

    /* Action area */
    DM1_V1_Click_AddZonePc34Compat(s, 233, 72, 85, 50, DM1_ZONE_ACTION_AREA);

    /* Movement arrows — non-overlapping zones per ReDMCSB COMMAND.C
     * G0459_aai_Graphic561_CommandAreaCoordinates.
     * Layout: forward top-center, turn left/right at sides,
     * strafe left/right below turns, backward bottom-center. */
    DM1_V1_Click_AddZonePc34Compat(s, 255, 124, 42, 18, DM1_ZONE_MOVEMENT_FORWARD);
    DM1_V1_Click_AddZonePc34Compat(s, 234, 142, 28, 28, DM1_ZONE_MOVEMENT_TURN_LEFT);
    DM1_V1_Click_AddZonePc34Compat(s, 292, 142, 28, 28, DM1_ZONE_MOVEMENT_TURN_RIGHT);
    DM1_V1_Click_AddZonePc34Compat(s, 234, 170, 28, 30, DM1_ZONE_MOVEMENT_LEFT);
    DM1_V1_Click_AddZonePc34Compat(s, 292, 170, 28, 30, DM1_ZONE_MOVEMENT_RIGHT);
    DM1_V1_Click_AddZonePc34Compat(s, 262, 170, 30, 30, DM1_ZONE_MOVEMENT_BACKWARD);

    /* Champion panels: 4 panels in 2x2 grid at bottom of screen.
     * Each panel is 80x29 pixels per ReDMCSB screen layout. */
    DM1_V1_Click_AddZonePc34Compat(s, 0, 136, 80, 29, DM1_ZONE_CHAMPION_0);
    DM1_V1_Click_AddZonePc34Compat(s, 80, 136, 80, 29, DM1_ZONE_CHAMPION_1);
    DM1_V1_Click_AddZonePc34Compat(s, 0, 165, 80, 29, DM1_ZONE_CHAMPION_2);
    DM1_V1_Click_AddZonePc34Compat(s, 80, 165, 80, 29, DM1_ZONE_CHAMPION_3);
}

void DM1_V1_Click_SetupInventoryZonesPc34Compat(DM1_V1_ClickStatePc34 *s)
{
    DM1_V1_Click_ClearZonesPc34Compat(s);
    /* Inventory uses a grid of slots — simplified as one large zone */
    DM1_V1_Click_AddZonePc34Compat(s, 0, 0, 320, 200, DM1_ZONE_INVENTORY);
}

void DM1_V1_Click_ClearZonesPc34Compat(DM1_V1_ClickStatePc34 *s)
{
    s->zoneCount = 0;
    s->lastClickZone = DM1_ZONE_NONE;
}

DM1_V1_ViewportClickResultPc34 DM1_V1_Viewport_ResolveClickPc34Compat(
    int mx, int my, int partyDir, int partyX, int partyY,
    int hasLeader, int leaderHandEmpty)
{
    return DM1_V1_Viewport_ResolveClickWithGrabbableMaskPc34Compat(
        mx, my, partyDir, partyX, partyY, hasLeader, leaderHandEmpty,
        DM1_VIEWPORT_GRABBABLE_NO_CELLS);
}

DM1_V1_ViewportClickResultPc34 DM1_V1_Viewport_ResolveClickWithGrabbableMaskPc34Compat(
    int mx, int my, int partyDir, int partyX, int partyY,
    int hasLeader, int leaderHandEmpty, uint8_t grabbableCellMask)
{
    DM1_V1_ViewportClickResultPc34 result;
    memset(&result, 0, sizeof(result));
    result.pileTopObjectId = DM1_VIEWPORT_NO_PILE_TOP_OBJECT;

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

DM1_V1_ViewportClickResultPc34 DM1_V1_Viewport_ResolveClickWithGrabbableStatePc34Compat(
    int mx, int my, int partyDir, int partyX, int partyY,
    int hasLeader, int leaderHandEmpty,
    const DM1_V1_ViewportGrabbableStatePc34 *grabbableState)
{
    DM1_V1_ViewportClickResultPc34 result = DM1_V1_Viewport_ResolveClickWithGrabbableMaskPc34Compat(
        mx, my, partyDir, partyX, partyY, hasLeader, leaderHandEmpty,
        grabbableState ? grabbableState->grabbableCellMask
                       : DM1_VIEWPORT_GRABBABLE_NO_CELLS);
    if (result.objectGrabbed) {
        result.pileTopObjectId =
            DM1_V1_ViewportGrabbable_PileTopPc34Compat(grabbableState, result.viewCell);
    }
    return result;
}

const char *DM1_V1_ViewportClick_SourceEvidencePc34Compat(void)
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
        "DUNVIEW.C:5113-5178: F0115 creates/extends grabbable object zones\n"
        "  while rendering visible object piles, then records pile-top object.\n"
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

