#ifndef FIRESTAFF_DM1_V1_VIEWPORT_HAND_OVERLAY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_HAND_OVERLAY_PC34_COMPAT_H

/*
 * DM1 V1 viewport hand/cursor carried-item overlay gate.
 *
 * Source reference: ReDMCSB IO.C
 *   - F0781_MouseHandler: when an object is carried, a HAND pointer screen
 *     region resolves to C2_POINTER_OBJECT_ICON instead of C1_POINTER_HAND
 *     (IO.C:1232-1255; same shape at IO.C:719-745).
 *   - S0073_MOUSE_DrawPointerToScreen: viewport/status/menus/message screen
 *     zones select arrow/hand/object priority, with carried object using a
 *     custom 18x18 overlay and hotspot (8,8) (IO.C:2765-2837).
 *   - Palette-zone split for carried object/champion icons starts after the
 *     object/champion hotspot Y offset and can mix original/modified colors
 *     across the middle-screen palette boundary (IO.C:2840-2877).
 */

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_POINTER_ARROW       0
#define DM1_V1_POINTER_HAND        1
#define DM1_V1_POINTER_OBJECT_ICON 2

#define DM1_V1_CURSOR_VIEWPORT_RIGHT_EXCLUSIVE 224
#define DM1_V1_CURSOR_TOP_PANEL_BOTTOM          28
#define DM1_V1_CURSOR_MENU_LEFT                224
#define DM1_V1_CURSOR_CHAMPION_ICON_LEFT       274
#define DM1_V1_CURSOR_MESSAGE_TOP              169
#define DM1_V1_CURSOR_STATUS_NAME_WIDTH         42
#define DM1_V1_CURSOR_STATUS_NAME_BOTTOM         6
#define DM1_V1_CURSOR_CHAMPION_STATUS_SPACING   69

#define DM1_V1_CURSOR_ARROW_VISIBLE_WIDTH       10
#define DM1_V1_CURSOR_ARROW_RIGHT_PADDING        6
#define DM1_V1_CURSOR_HAND_VISIBLE_WIDTH        16
#define DM1_V1_CURSOR_HAND_RIGHT_PADDING         0
#define DM1_V1_CURSOR_HAND_HOTSPOT_X             0
#define DM1_V1_CURSOR_HAND_HOTSPOT_Y             0
#define DM1_V1_CURSOR_HAND_LAST_LINE_INDEX      15
#define DM1_V1_CURSOR_OBJECT_WIDTH              18
#define DM1_V1_CURSOR_OBJECT_HEIGHT             18
#define DM1_V1_CURSOR_OBJECT_PIXEL_WIDTH_MINUS1 17
#define DM1_V1_CURSOR_OBJECT_HOTSPOT_X           8
#define DM1_V1_CURSOR_OBJECT_HOTSPOT_Y           8
#define DM1_V1_CURSOR_OBJECT_SCREEN_Y_OFFSET    19
#define DM1_V1_CURSOR_OBJECT_LAST_LINE_INDEX    17
#define DM1_V1_CURSOR_OBJECT_UNIT_WIDTH          2
#define DM1_V1_CURSOR_OBJECT_EDGE_UNIT_WIDTH     3

#define DM1_V1_CURSOR_PALETTE_MIDDLE_Y          19

typedef struct DM1_V1_ViewportHandOverlayDecisionPc34Compat {
    int pointerType;
    int hotspotX;
    int hotspotY;
    int width;
    int height;
    int visibleWidth;
    int rightPadding;
    int lastLineIndex;
    int unitWidth;
    int usesCustomBitmap;
    int usesObjectBitmap;
    int paletteSplitCandidate;
} DM1_V1_ViewportHandOverlayDecisionPc34Compat;

static inline int DM1_V1_ViewportHandOverlay_RegionPointerPc34Compat(
    int regionPointerType,
    int useObjectAsPointer)
{
    if (useObjectAsPointer && regionPointerType == DM1_V1_POINTER_HAND) {
        return DM1_V1_POINTER_OBJECT_ICON;
    }
    return regionPointerType;
}

static inline int DM1_V1_ViewportHandOverlay_IsViewportOrTopHandZonePc34Compat(
    int x,
    int y,
    int partyChampionCount,
    int inventoryChampionOrdinal)
{
    if (y >= DM1_V1_CURSOR_MESSAGE_TOP || x >= DM1_V1_CURSOR_CHAMPION_ICON_LEFT) {
        return 0;
    }
    if (y <= DM1_V1_CURSOR_TOP_PANEL_BOTTOM) {
        int championIndex = x / DM1_V1_CURSOR_CHAMPION_STATUS_SPACING;
        int statusRemainder = x % DM1_V1_CURSOR_CHAMPION_STATUS_SPACING;
        if (championIndex >= partyChampionCount) {
            return 1;
        }
        if (statusRemainder > DM1_V1_CURSOR_STATUS_NAME_WIDTH) {
            return 1;
        }
        if ((championIndex + 1) == inventoryChampionOrdinal) {
            return 0;
        }
        if (y <= DM1_V1_CURSOR_STATUS_NAME_BOTTOM) {
            return 0;
        }
        return 1;
    }
    if (x >= DM1_V1_CURSOR_MENU_LEFT) {
        return 0;
    }
    return 1;
}

static inline DM1_V1_ViewportHandOverlayDecisionPc34Compat
DM1_V1_ViewportHandOverlay_DecidePc34Compat(
    int x,
    int y,
    int useHandAsPointer,
    int useObjectAsPointer,
    int partyChampionCount,
    int inventoryChampionOrdinal)
{
    DM1_V1_ViewportHandOverlayDecisionPc34Compat out;
    out.pointerType = DM1_V1_POINTER_ARROW;
    out.hotspotX = DM1_V1_CURSOR_HAND_HOTSPOT_X;
    out.hotspotY = DM1_V1_CURSOR_HAND_HOTSPOT_Y;
    out.width = 16;
    out.height = 16;
    out.visibleWidth = DM1_V1_CURSOR_ARROW_VISIBLE_WIDTH;
    out.rightPadding = DM1_V1_CURSOR_ARROW_RIGHT_PADDING;
    out.lastLineIndex = DM1_V1_CURSOR_HAND_LAST_LINE_INDEX;
    out.unitWidth = 1;
    out.usesCustomBitmap = 0;
    out.usesObjectBitmap = 0;
    out.paletteSplitCandidate = 0;

    if ((useHandAsPointer || useObjectAsPointer) &&
        DM1_V1_ViewportHandOverlay_IsViewportOrTopHandZonePc34Compat(
            x, y, partyChampionCount, inventoryChampionOrdinal)) {
        if (useObjectAsPointer) {
            out.pointerType = DM1_V1_POINTER_OBJECT_ICON;
            out.hotspotX = DM1_V1_CURSOR_OBJECT_HOTSPOT_X;
            out.hotspotY = DM1_V1_CURSOR_OBJECT_HOTSPOT_Y;
            out.width = DM1_V1_CURSOR_OBJECT_WIDTH;
            out.height = DM1_V1_CURSOR_OBJECT_HEIGHT;
            out.visibleWidth = DM1_V1_CURSOR_OBJECT_WIDTH;
            out.rightPadding = 0;
            out.lastLineIndex = DM1_V1_CURSOR_OBJECT_LAST_LINE_INDEX;
            out.unitWidth = ((x - DM1_V1_CURSOR_OBJECT_HOTSPOT_X) & 0x0F) == 15
                ? DM1_V1_CURSOR_OBJECT_EDGE_UNIT_WIDTH
                : DM1_V1_CURSOR_OBJECT_UNIT_WIDTH;
            out.usesCustomBitmap = 1;
            out.usesObjectBitmap = 1;
            out.paletteSplitCandidate = (y - DM1_V1_CURSOR_OBJECT_SCREEN_Y_OFFSET) > 0;
        } else {
            out.pointerType = DM1_V1_POINTER_HAND;
            out.visibleWidth = DM1_V1_CURSOR_HAND_VISIBLE_WIDTH;
            out.rightPadding = DM1_V1_CURSOR_HAND_RIGHT_PADDING;
            out.lastLineIndex = DM1_V1_CURSOR_HAND_LAST_LINE_INDEX;
        }
    }

    return out;
}

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_HAND_OVERLAY_PC34_COMPAT_H */
