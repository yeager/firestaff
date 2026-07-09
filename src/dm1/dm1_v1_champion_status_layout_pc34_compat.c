#include "dm1_v1_champion_status_layout_pc34_compat.h"

#include "champion_status_slotbox_pc34_compat.h"
#include "dm1_v1_champion_panel_hud_pc34_compat.h"

static int copy_rect(const ChampionStatusRectCompat* src,
                     DM1_V1_ChampionStatusRectPc34* out) {
    if (!src || !out) return 0;
    out->x = src->x;
    out->y = src->y;
    out->w = src->w;
    out->h = src->h;
    return 1;
}

const char* dm1_v1_champion_status_layout_source_evidence_pc34(void) {
    return "ReDMCSB CHAMDRAW.C F0287/F0291/F0292/F0320, "
           "INVNTORY.C F0354/F0355, DEFS.H C151..C218/C167..C182";
}

int dm1_v1_champion_status_box_zone_id_pc34(int championSlot) {
    return CHAMPION_Compat_StatusBoxZoneId(championSlot);
}

int dm1_v1_champion_status_box_rect_pc34(
    int championSlot,
    DM1_V1_ChampionStatusRectPc34* out) {
    ChampionStatusRectCompat r;
    return CHAMPION_Compat_StatusBoxZone(championSlot, &r) &&
           copy_rect(&r, out);
}

int dm1_v1_champion_status_bar_graph_zone_id_pc34(int championSlot) {
    return CHAMPION_Compat_StatusBarGraphZoneId(championSlot);
}

int dm1_v1_champion_status_bar_zone_id_pc34(int statIndex) {
    return CHAMPION_Compat_StatusBarZoneId(statIndex);
}

int dm1_v1_champion_status_bar_value_zone_id_pc34(
    int championSlot,
    int statIndex) {
    return CHAMPION_Compat_StatusBarValueZoneId(championSlot, statIndex);
}

int dm1_v1_champion_status_bar_rect_pc34(
    int championSlot,
    int statIndex,
    DM1_V1_ChampionStatusRectPc34* out) {
    ChampionStatusRectCompat r;
    return CHAMPION_Compat_StatusBarZone(championSlot, statIndex, &r) &&
           copy_rect(&r, out);
}

int dm1_v1_champion_status_hand_parent_zone_id_pc34(int championSlot) {
    return CHAMPION_Compat_StatusHandParentZoneId(championSlot);
}

int dm1_v1_champion_status_hand_zone_id_pc34(
    int championSlot,
    int handIndex) {
    return CHAMPION_Compat_StatusHandZoneId(championSlot, handIndex);
}

int dm1_v1_champion_status_hand_rect_pc34(
    int championSlot,
    int handIndex,
    DM1_V1_ChampionStatusRectPc34* out) {
    ChampionStatusRectCompat r;
    return CHAMPION_Compat_StatusHandZone(championSlot, handIndex, &r) &&
           copy_rect(&r, out);
}

int dm1_v1_champion_status_hand_icon_rect_pc34(
    int championSlot,
    int handIndex,
    DM1_V1_ChampionStatusRectPc34* out) {
    ChampionStatusRectCompat r;
    return CHAMPION_Compat_StatusHandIconZone(championSlot, handIndex, &r) &&
           copy_rect(&r, out);
}

int dm1_v1_champion_status_hand_slot_box_rect_pc34(
    int championSlot,
    int handIndex,
    DM1_V1_ChampionStatusRectPc34* out) {
    ChampionStatusRectCompat r;
    return CHAMPION_Compat_StatusHandSlotBoxZone(championSlot, handIndex, &r) &&
           copy_rect(&r, out);
}

int dm1_v1_champion_status_hand_slot_graphic_pc34(
    int handIndex,
    uint16_t wounds,
    int isActingChampion) {
    return DM1_ChampionPanel_SlotBoxGraphic(handIndex,
                                            wounds,
                                            isActingChampion);
}

int dm1_v1_champion_status_name_clear_zone_id_pc34(int championSlot) {
    return CHAMPION_Compat_StatusNameClearZoneId(championSlot);
}

int dm1_v1_champion_status_name_text_zone_id_pc34(int championSlot) {
    return CHAMPION_Compat_StatusNameTextZoneId(championSlot);
}

int dm1_v1_champion_status_name_rect_pc34(
    int championSlot,
    DM1_V1_ChampionStatusRectPc34* out) {
    ChampionStatusRectCompat r;
    return CHAMPION_Compat_StatusNameZone(championSlot, &r) &&
           copy_rect(&r, out);
}

int dm1_v1_champion_status_name_text_rect_pc34(
    int championSlot,
    DM1_V1_ChampionStatusRectPc34* out) {
    ChampionStatusRectCompat r;
    return CHAMPION_Compat_StatusNameTextZone(championSlot, &r) &&
           copy_rect(&r, out);
}

int dm1_v1_champion_status_name_color_pc34(
    int present,
    int currentHealth,
    int leader) {
    return CHAMPION_Compat_StatusNameColor(present, currentHealth, leader);
}

int dm1_v1_champion_status_name_clear_color_pc34(void) {
    return CHAMPION_Compat_StatusNameClearColor();
}

int dm1_v1_champion_status_box_fill_color_pc34(void) {
    return CHAMPION_Compat_StatusBoxFillColor();
}

int dm1_v1_champion_status_box_graphic_pc34(void) {
    return CHAMPION_Compat_StatusBoxGraphicId();
}

int dm1_v1_champion_dead_status_box_graphic_pc34(void) {
    return CHAMPION_Compat_DeadStatusBoxGraphicId();
}

int dm1_v1_champion_status_box_base_graphic_pc34(
    int present,
    int currentHealth) {
    return CHAMPION_Compat_StatusBoxBaseGraphic(present, currentHealth);
}

int dm1_v1_champion_status_shield_border_graphics_pc34(
    int fireShieldDefense,
    int spellShieldDefense,
    int partyShieldDefense,
    int outGraphics[3]) {
    return CHAMPION_Compat_StatusShieldBorderGraphics(fireShieldDefense,
                                                      spellShieldDefense,
                                                      partyShieldDefense,
                                                      outGraphics);
}

int dm1_v1_champion_status_shield_border_rect_pc34(
    int championSlot,
    DM1_V1_ChampionStatusRectPc34* out) {
    return dm1_v1_champion_status_box_rect_pc34(championSlot, out);
}

int dm1_v1_champion_poison_label_rect_pc34(
    int championSlot,
    int labelW,
    int labelH,
    DM1_V1_ChampionStatusRectPc34* out) {
    DM1_V1_ChampionStatusRectPc34 box;
    if (labelW <= 0 || labelH <= 0 ||
        !dm1_v1_champion_status_box_rect_pc34(championSlot, &box) || !out) {
        return 0;
    }
    out->x = box.x + (box.w - labelW) / 2;
    out->y = box.y + box.h;
    out->w = labelW;
    out->h = labelH;
    return 1;
}

int dm1_v1_champion_damage_indicator_zone_id_pc34(int championSlot) {
    return CHAMPION_Compat_DamageIndicatorZoneId(championSlot);
}

int dm1_v1_champion_damage_indicator_rect_pc34(
    int championSlot,
    int indicatorW,
    int indicatorH,
    DM1_V1_ChampionStatusRectPc34* out) {
    ChampionStatusRectCompat r;
    return CHAMPION_Compat_DamageIndicatorZone(championSlot,
                                               indicatorW,
                                               indicatorH,
                                               &r) &&
           copy_rect(&r, out);
}

int dm1_v1_champion_inventory_damage_indicator_zone_id_pc34(int championSlot) {
    return CHAMPION_Compat_InventoryDamageIndicatorZoneId(championSlot);
}

int dm1_v1_champion_inventory_damage_indicator_rect_pc34(
    int championSlot,
    int indicatorW,
    int indicatorH,
    DM1_V1_ChampionStatusRectPc34* out) {
    ChampionStatusRectCompat r;
    return CHAMPION_Compat_InventoryDamageIndicatorZone(championSlot,
                                                        indicatorW,
                                                        indicatorH,
                                                        &r) &&
           copy_rect(&r, out);
}

int dm1_v1_champion_damage_number_origin_pc34(
    int championSlot,
    DM1_V1_ChampionStatusRectPc34* out) {
    ChampionStatusRectCompat r;
    return CHAMPION_Compat_DamageNumberOrigin(championSlot, &r) &&
           copy_rect(&r, out);
}

int dm1_v1_champion_damage_number_origin_variant_pc34(
    int championSlot,
    int damageAmount,
    int inventoryChampion,
    DM1_V1_ChampionStatusRectPc34* out) {
    ChampionStatusRectCompat r;
    return CHAMPION_Compat_DamageNumberOriginPc34(championSlot,
                                                  damageAmount,
                                                  inventoryChampion,
                                                  &r) &&
           copy_rect(&r, out);
}
