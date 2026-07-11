#ifndef FIRESTAFF_DM1_V1_LAYOUT_ZONES_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_LAYOUT_ZONES_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int x;
    int y;
    int w;
    int h;
} DM1_V1_LayoutZoneRectPc34;

const char* dm1_v1_layout_zones_source_evidence_pc34(void);

int dm1_v1_screen_zone_id_pc34(void);
DM1_V1_LayoutZoneRectPc34 dm1_v1_screen_rect_pc34(void);
int dm1_v1_screen_centered_dialog_zone_id_pc34(void);
DM1_V1_LayoutZoneRectPc34 dm1_v1_screen_centered_dialog_rect_pc34(void);
int dm1_v1_explosion_pattern_d0c_zone_id_pc34(void);
DM1_V1_LayoutZoneRectPc34 dm1_v1_explosion_pattern_d0c_rect_pc34(void);
int dm1_v1_viewport_centered_text_zone_id_pc34(void);
int dm1_v1_viewport_centered_text_rect_pc34(int contentW,
                                            int contentH,
                                            DM1_V1_LayoutZoneRectPc34* outRect);
int dm1_v1_message_area_zone_id_pc34(void);
DM1_V1_LayoutZoneRectPc34 dm1_v1_message_area_rect_pc34(void);
int dm1_v1_viewport_zone_id_pc34(void);
DM1_V1_LayoutZoneRectPc34 dm1_v1_viewport_rect_pc34(void);
int dm1_v1_leader_hand_object_name_zone_id_pc34(void);
DM1_V1_LayoutZoneRectPc34 dm1_v1_leader_hand_object_name_rect_pc34(void);
int dm1_v1_champion_icon_zone_id_pc34(int championSlot);
int dm1_v1_champion_icon_rect_pc34(int championSlot,
                                   DM1_V1_LayoutZoneRectPc34* outRect);
DM1_V1_LayoutZoneRectPc34 dm1_v1_inventory_backdrop_rect_pc34(void);
int dm1_v1_inventory_backdrop_zone_xywh_pc34(
    int* outX, int* outY, int* outW, int* outH);
int dm1_v1_inventory_panel_zone_id_pc34(void);
DM1_V1_LayoutZoneRectPc34 dm1_v1_inventory_panel_rect_pc34(void);
int dm1_v1_inventory_panel_zone_xywh_pc34(
    int* outX, int* outY, int* outW, int* outH);
int dm1_v1_object_description_circle_zone_id_pc34(void);
DM1_V1_LayoutZoneRectPc34 dm1_v1_object_description_circle_rect_pc34(void);
int dm1_v1_object_description_circle_zone_xywh_pc34(
    int* outX, int* outY, int* outW, int* outH);
int dm1_v1_object_description_icon_zone_id_pc34(void);
DM1_V1_LayoutZoneRectPc34 dm1_v1_object_description_icon_rect_pc34(void);
int dm1_v1_object_description_icon_zone_xywh_pc34(
    int* outX, int* outY, int* outW, int* outH);
int dm1_v1_arrow_or_eye_zone_id_pc34(void);
DM1_V1_LayoutZoneRectPc34 dm1_v1_arrow_or_eye_rect_pc34(void);
int dm1_v1_arrow_or_eye_zone_xywh_pc34(
    int* outX, int* outY, int* outW, int* outH);
int dm1_v1_object_description_name_zone_id_pc34(void);
int dm1_v1_object_description_name_rect_for_text_pc34(
    int textPixelWidth,
    int textPixelHeight,
    DM1_V1_LayoutZoneRectPc34* outRect);
int dm1_v1_object_description_continuation_origin_pc34(int* outX, int* outY);

#ifdef __cplusplus
}
#endif

#endif
