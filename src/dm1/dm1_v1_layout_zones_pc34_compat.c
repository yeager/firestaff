#include "dm1_v1_layout_zones_pc34_compat.h"

const char* dm1_v1_layout_zones_source_evidence_pc34(void) {
    return "ReDMCSB source lock: PANEL.C:172-223 F0335 form-feed resets "
           "text origin through C556_ZONE_OBJECT_DESCRIPTION; "
           "PANEL.C:1136-1145 F0342 enters C03_PANEL_OBJECT_DESCRIPTION, "
           "blits C020 panel to C101 and C029 circle to C504; "
           "PANEL.C:1198-1200 prints object name in C506 and object icon "
           "in C505; TEXT.C:1937-1950 F0648 measures text and resolves "
           "C506 through COORD.C F0635; COORD.C:2052-2412 F0635 resolves "
           "layout records; COORD.C:2434-2448 F0636 adds the 1-pixel text "
           "margin; DEFS.H C002/C004/C005/C006/C007/C015/C017/C101/"
           "C113..C116/C503..C506/C556; DATA.C:316 and layout-696 provide "
           "the panel/circle/icon/text geometry.";
}

int dm1_v1_screen_zone_id_pc34(void) {
    return 2;
}

DM1_V1_LayoutZoneRectPc34 dm1_v1_screen_rect_pc34(void) {
    DM1_V1_LayoutZoneRectPc34 rect = { 0, 0, 320, 200 };
    return rect;
}

int dm1_v1_screen_centered_dialog_zone_id_pc34(void) {
    return 5;
}

DM1_V1_LayoutZoneRectPc34 dm1_v1_screen_centered_dialog_rect_pc34(void) {
    DM1_V1_LayoutZoneRectPc34 rect = { 48, 32, 224, 136 };
    return rect;
}

int dm1_v1_explosion_pattern_d0c_zone_id_pc34(void) {
    return 4;
}

DM1_V1_LayoutZoneRectPc34 dm1_v1_explosion_pattern_d0c_rect_pc34(void) {
    DM1_V1_LayoutZoneRectPc34 rect = { 0, 0, 32, 29 };
    return rect;
}

int dm1_v1_viewport_centered_text_zone_id_pc34(void) {
    return 6;
}

int dm1_v1_viewport_centered_text_rect_pc34(
    int contentW,
    int contentH,
    DM1_V1_LayoutZoneRectPc34* outRect) {
    if (!outRect || contentW <= 0 || contentH <= 0) {
        return 0;
    }
    outRect->x = (224 - contentW) / 2;
    outRect->y = (136 - contentH) / 2;
    outRect->w = contentW;
    outRect->h = contentH;
    return 1;
}

int dm1_v1_message_area_zone_id_pc34(void) {
    return 15;
}

DM1_V1_LayoutZoneRectPc34 dm1_v1_message_area_rect_pc34(void) {
    DM1_V1_LayoutZoneRectPc34 rect = { 0, 173, 320, 27 };
    return rect;
}

int dm1_v1_viewport_zone_id_pc34(void) {
    return 7;
}

DM1_V1_LayoutZoneRectPc34 dm1_v1_viewport_rect_pc34(void) {
    DM1_V1_LayoutZoneRectPc34 rect = { 0, 33, 224, 136 };
    return rect;
}

int dm1_v1_leader_hand_object_name_zone_id_pc34(void) {
    return 17;
}

DM1_V1_LayoutZoneRectPc34 dm1_v1_leader_hand_object_name_rect_pc34(void) {
    DM1_V1_LayoutZoneRectPc34 rect = { 233, 33, 87, 6 };
    return rect;
}

int dm1_v1_champion_icon_zone_id_pc34(int championSlot) {
    if (championSlot < 0 || championSlot >= 4) {
        return 0;
    }
    return 113 + championSlot;
}

int dm1_v1_champion_icon_rect_pc34(int championSlot,
                                   DM1_V1_LayoutZoneRectPc34* outRect) {
    static const int xs[4] = { 281, 301, 301, 281 };
    static const int ys[4] = { 0, 0, 15, 15 };
    if (!outRect || !dm1_v1_champion_icon_zone_id_pc34(championSlot)) {
        return 0;
    }
    outRect->x = xs[championSlot];
    outRect->y = ys[championSlot];
    outRect->w = 19;
    outRect->h = 14;
    return 1;
}

DM1_V1_LayoutZoneRectPc34 dm1_v1_inventory_backdrop_rect_pc34(void) {
    return dm1_v1_viewport_rect_pc34();
}

int dm1_v1_inventory_backdrop_zone_xywh_pc34(
    int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_LayoutZoneRectPc34 rect = dm1_v1_inventory_backdrop_rect_pc34();
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return 1;
}

int dm1_v1_inventory_panel_zone_id_pc34(void) {
    return 101;
}

DM1_V1_LayoutZoneRectPc34 dm1_v1_inventory_panel_rect_pc34(void) {
    DM1_V1_LayoutZoneRectPc34 rect = { 80, 52, 144, 73 };
    return rect;
}

int dm1_v1_inventory_panel_zone_xywh_pc34(
    int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_LayoutZoneRectPc34 rect = dm1_v1_inventory_panel_rect_pc34();
    if (!dm1_v1_inventory_panel_zone_id_pc34()) return 0;
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return 1;
}

int dm1_v1_object_description_circle_zone_id_pc34(void) {
    return 504;
}

DM1_V1_LayoutZoneRectPc34 dm1_v1_object_description_circle_rect_pc34(void) {
    DM1_V1_LayoutZoneRectPc34 rect = { 103, 53, 32, 27 };
    return rect;
}

int dm1_v1_object_description_circle_zone_xywh_pc34(
    int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_LayoutZoneRectPc34 rect =
        dm1_v1_object_description_circle_rect_pc34();
    if (!dm1_v1_object_description_circle_zone_id_pc34()) return 0;
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return 1;
}

int dm1_v1_object_description_icon_zone_id_pc34(void) {
    return 505;
}

DM1_V1_LayoutZoneRectPc34 dm1_v1_object_description_icon_rect_pc34(void) {
    DM1_V1_LayoutZoneRectPc34 rect = { 111, 59, 16, 16 };
    return rect;
}

int dm1_v1_object_description_icon_zone_xywh_pc34(
    int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_LayoutZoneRectPc34 rect =
        dm1_v1_object_description_icon_rect_pc34();
    if (!dm1_v1_object_description_icon_zone_id_pc34()) return 0;
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return 1;
}

int dm1_v1_arrow_or_eye_zone_id_pc34(void) {
    return 503;
}

DM1_V1_LayoutZoneRectPc34 dm1_v1_arrow_or_eye_rect_pc34(void) {
    DM1_V1_LayoutZoneRectPc34 rect = { 83, 57, 16, 9 };
    return rect;
}

int dm1_v1_arrow_or_eye_zone_xywh_pc34(
    int* outX, int* outY, int* outW, int* outH) {
    DM1_V1_LayoutZoneRectPc34 rect = dm1_v1_arrow_or_eye_rect_pc34();
    if (!dm1_v1_arrow_or_eye_zone_id_pc34()) return 0;
    if (outX) *outX = rect.x;
    if (outY) *outY = rect.y;
    if (outW) *outW = rect.w;
    if (outH) *outH = rect.h;
    return 1;
}

int dm1_v1_object_description_name_zone_id_pc34(void) {
    return 506;
}

int dm1_v1_object_description_name_rect_for_text_pc34(
    int textPixelWidth,
    int textPixelHeight,
    DM1_V1_LayoutZoneRectPc34* outRect) {
    if (!outRect || textPixelWidth <= 0 || textPixelHeight <= 0) {
        return 0;
    }
    outRect->x = 134;
    outRect->y = 68 - ((textPixelHeight + 1) / 2);
    outRect->w = textPixelWidth;
    outRect->h = textPixelHeight;
    return 1;
}

int dm1_v1_object_description_continuation_origin_pc34(int* outX, int* outY) {
    if (outX) *outX = 108;
    if (outY) *outY = 59;
    return 1;
}
