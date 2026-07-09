#ifndef FIRESTAFF_DM1_V1_CHAMPION_STATUS_LAYOUT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_STATUS_LAYOUT_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_ChampionStatusRectPc34 {
    int x;
    int y;
    int w;
    int h;
} DM1_V1_ChampionStatusRectPc34;

const char* dm1_v1_champion_status_layout_source_evidence_pc34(void);

int dm1_v1_champion_status_box_zone_id_pc34(int championSlot);
int dm1_v1_champion_status_box_rect_pc34(
    int championSlot,
    DM1_V1_ChampionStatusRectPc34* out);
int dm1_v1_champion_status_bar_graph_zone_id_pc34(int championSlot);
int dm1_v1_champion_status_bar_zone_id_pc34(int statIndex);
int dm1_v1_champion_status_bar_value_zone_id_pc34(
    int championSlot,
    int statIndex);
int dm1_v1_champion_status_bar_rect_pc34(
    int championSlot,
    int statIndex,
    DM1_V1_ChampionStatusRectPc34* out);

int dm1_v1_champion_status_hand_parent_zone_id_pc34(int championSlot);
int dm1_v1_champion_status_hand_zone_id_pc34(
    int championSlot,
    int handIndex);
int dm1_v1_champion_status_hand_rect_pc34(
    int championSlot,
    int handIndex,
    DM1_V1_ChampionStatusRectPc34* out);
int dm1_v1_champion_status_hand_icon_rect_pc34(
    int championSlot,
    int handIndex,
    DM1_V1_ChampionStatusRectPc34* out);
int dm1_v1_champion_status_hand_slot_box_rect_pc34(
    int championSlot,
    int handIndex,
    DM1_V1_ChampionStatusRectPc34* out);
int dm1_v1_champion_status_hand_slot_graphic_pc34(
    int handIndex,
    uint16_t wounds,
    int isActingChampion);

int dm1_v1_champion_status_name_clear_zone_id_pc34(int championSlot);
int dm1_v1_champion_status_name_text_zone_id_pc34(int championSlot);
int dm1_v1_champion_status_name_rect_pc34(
    int championSlot,
    DM1_V1_ChampionStatusRectPc34* out);
int dm1_v1_champion_status_name_text_rect_pc34(
    int championSlot,
    DM1_V1_ChampionStatusRectPc34* out);
int dm1_v1_champion_status_name_color_pc34(
    int present,
    int currentHealth,
    int leader);
int dm1_v1_champion_status_name_clear_color_pc34(void);
int dm1_v1_champion_status_box_fill_color_pc34(void);

int dm1_v1_champion_status_box_graphic_pc34(void);
int dm1_v1_champion_dead_status_box_graphic_pc34(void);
int dm1_v1_champion_status_box_base_graphic_pc34(
    int present,
    int currentHealth);
int dm1_v1_champion_status_shield_border_graphics_pc34(
    int fireShieldDefense,
    int spellShieldDefense,
    int partyShieldDefense,
    int outGraphics[3]);
int dm1_v1_champion_status_shield_border_rect_pc34(
    int championSlot,
    DM1_V1_ChampionStatusRectPc34* out);

int dm1_v1_champion_poison_label_rect_pc34(
    int championSlot,
    int labelW,
    int labelH,
    DM1_V1_ChampionStatusRectPc34* out);
int dm1_v1_champion_damage_indicator_zone_id_pc34(int championSlot);
int dm1_v1_champion_damage_indicator_rect_pc34(
    int championSlot,
    int indicatorW,
    int indicatorH,
    DM1_V1_ChampionStatusRectPc34* out);
int dm1_v1_champion_inventory_damage_indicator_zone_id_pc34(int championSlot);
int dm1_v1_champion_inventory_damage_indicator_rect_pc34(
    int championSlot,
    int indicatorW,
    int indicatorH,
    DM1_V1_ChampionStatusRectPc34* out);
int dm1_v1_champion_damage_number_origin_pc34(
    int championSlot,
    DM1_V1_ChampionStatusRectPc34* out);
int dm1_v1_champion_damage_number_origin_variant_pc34(
    int championSlot,
    int damageAmount,
    int inventoryChampion,
    DM1_V1_ChampionStatusRectPc34* out);

#ifdef __cplusplus
}
#endif

#endif
