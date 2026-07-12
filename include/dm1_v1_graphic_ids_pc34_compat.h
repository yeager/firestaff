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

#define DM1_V1_CHAMPION_PORTRAIT_GRAPHIC_PC34 26
#define DM1_V1_CHAMPION_PORTRAIT_COLUMNS_PC34 8
#define DM1_V1_CHAMPION_PORTRAIT_ROWS_PC34 3
#define DM1_V1_CHAMPION_PORTRAIT_COUNT_PC34 \
    (DM1_V1_CHAMPION_PORTRAIT_COLUMNS_PC34 * \
     DM1_V1_CHAMPION_PORTRAIT_ROWS_PC34)
#define DM1_V1_CHAMPION_PORTRAIT_WIDTH_PC34 32
#define DM1_V1_CHAMPION_PORTRAIT_HEIGHT_PC34 29

int dm1_v1_graphic_dialog_box_pc34(void);
int dm1_v1_graphic_the_end_pc34(void);
int dm1_v1_graphic_endgame_champion_mirror_pc34(void);
int dm1_v1_graphic_champion_portraits_pc34(void);
/* ReDMCSB DUNVIEW.C:3913-3928 selects C026 with an explicit 0..23
 * ordinal. No modulo or fallback cell is valid for an absent portrait. */
int dm1_v1_graphic_champion_portrait_source_zone_pc34(
    int portrait_ordinal,
    DM1_V1_ObjectIconSourceZonePc34* outZone);
/* Accept only the native PC34 C026 8x3 atlas dimensions. */
int dm1_v1_graphic_validate_champion_portrait_atlas_pc34(
    int atlas_width,
    int atlas_height);
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
