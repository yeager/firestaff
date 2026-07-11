#ifndef FIRESTAFF_DM1_V1_GRAPHIC_IDS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GRAPHIC_IDS_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

const char* dm1_v1_graphic_ids_source_evidence_pc34(void);

typedef struct DM1_V1_ObjectIconSourceZonePc34 {
    int graphic_index;
    int x;
    int y;
    int w;
    int h;
} DM1_V1_ObjectIconSourceZonePc34;

int dm1_v1_graphic_dialog_box_pc34(void);
int dm1_v1_graphic_the_end_pc34(void);
int dm1_v1_graphic_endgame_champion_mirror_pc34(void);
int dm1_v1_graphic_champion_portraits_pc34(void);
int dm1_v1_graphic_champion_icons_pc34(void);
int dm1_v1_graphic_inventory_backdrop_pc34(void);
int dm1_v1_graphic_panel_empty_pc34(void);
int dm1_v1_graphic_panel_open_scroll_pc34(void);
int dm1_v1_graphic_object_description_circle_pc34(void);
int dm1_v1_graphic_arrow_or_eye_pc34(int pressingEye);
int dm1_v1_graphic_food_label_pc34(void);
int dm1_v1_graphic_water_label_pc34(void);
int dm1_v1_graphic_poisoned_label_pc34(void);
int dm1_v1_graphic_slot_box_normal_pc34(void);
int dm1_v1_graphic_slot_box_wounded_pc34(void);
int dm1_v1_graphic_slot_box_acting_hand_pc34(void);
int dm1_v1_graphic_party_shield_border_pc34(void);
int dm1_v1_graphic_fire_shield_border_pc34(void);
int dm1_v1_graphic_spell_shield_border_pc34(void);
int dm1_v1_graphic_champion_damage_small_pc34(void);
int dm1_v1_graphic_champion_damage_big_pc34(void);
int dm1_v1_graphic_creature_damage_pc34(void);
int dm1_v1_graphic_wallset0_index_pc34(int wallSetIndex);
int dm1_v1_graphic_materialized_wallset_index_pc34(
    int mapWallSet,
    int wallSet0GraphicIndex);
int dm1_v1_object_icon_source_zone_pc34(
    int iconIndex,
    DM1_V1_ObjectIconSourceZonePc34* outZone);

#ifdef __cplusplus
}
#endif

#endif
