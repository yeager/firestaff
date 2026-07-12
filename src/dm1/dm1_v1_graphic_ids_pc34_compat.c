#include "dm1_v1_graphic_ids_pc34_compat.h"

const char* dm1_v1_graphic_ids_source_evidence_pc34(void) {
    return "ReDMCSB DEFS.H C006/C007/C008/C014..C020/C023/C026/C028..C039/"
           "C093..C107/C346; PANEL.C F0339/F0341/F0342; CHAMDRAW.C F0291/F0622; "
           "DUNVIEW.C F0104/F0105/F0128 wall panels; ENDGAME.C F0445; "
           "OBJECT.C F0034/F0036/F0068 object icon indices";
}

int dm1_v1_graphic_dialog_box_pc34(void) { return 17; }
int dm1_v1_graphic_the_end_pc34(void) { return 6; }
int dm1_v1_graphic_endgame_champion_mirror_pc34(void) { return 346; }
int dm1_v1_graphic_champion_portraits_pc34(void) {
    return DM1_V1_CHAMPION_PORTRAIT_GRAPHIC_PC34;
}

int dm1_v1_graphic_champion_portrait_source_zone_pc34(
    int portrait_ordinal,
    DM1_V1_ObjectIconSourceZonePc34* outZone)
{
    if (!outZone || portrait_ordinal < 0 ||
        portrait_ordinal >= DM1_V1_CHAMPION_PORTRAIT_COUNT_PC34) {
        return 0;
    }
    /* ReDMCSB DUNVIEW.C:3913-3928: C026 is 256x87, arranged as eight
     * 32x29 champion portraits per row. The sensor/mirror ordinal chooses
     * the source cell directly; an invalid ordinal draws nothing. */
    outZone->graphic_index = DM1_V1_CHAMPION_PORTRAIT_GRAPHIC_PC34;
    outZone->x = (portrait_ordinal % DM1_V1_CHAMPION_PORTRAIT_COLUMNS_PC34) *
                 DM1_V1_CHAMPION_PORTRAIT_WIDTH_PC34;
    outZone->y = (portrait_ordinal / DM1_V1_CHAMPION_PORTRAIT_COLUMNS_PC34) *
                 DM1_V1_CHAMPION_PORTRAIT_HEIGHT_PC34;
    outZone->w = DM1_V1_CHAMPION_PORTRAIT_WIDTH_PC34;
    outZone->h = DM1_V1_CHAMPION_PORTRAIT_HEIGHT_PC34;
    return 1;
}

int dm1_v1_graphic_validate_champion_portrait_atlas_pc34(
    int atlas_width,
    int atlas_height)
{
    return atlas_width ==
               DM1_V1_CHAMPION_PORTRAIT_COLUMNS_PC34 *
               DM1_V1_CHAMPION_PORTRAIT_WIDTH_PC34 &&
           atlas_height ==
               DM1_V1_CHAMPION_PORTRAIT_ROWS_PC34 *
               DM1_V1_CHAMPION_PORTRAIT_HEIGHT_PC34;
}
int dm1_v1_graphic_champion_icons_pc34(void) { return 28; }
int dm1_v1_graphic_inventory_backdrop_pc34(void) { return 17; }
int dm1_v1_graphic_panel_empty_pc34(void) { return 20; }
int dm1_v1_graphic_panel_open_scroll_pc34(void) { return 23; }
int dm1_v1_graphic_object_description_circle_pc34(void) { return 29; }

int dm1_v1_graphic_arrow_or_eye_pc34(int pressingEye) {
    return pressingEye ? 19 : 18;
}

int dm1_v1_graphic_food_label_pc34(void) { return 30; }
int dm1_v1_graphic_water_label_pc34(void) { return 31; }
int dm1_v1_graphic_poisoned_label_pc34(void) { return 32; }
int dm1_v1_graphic_slot_box_normal_pc34(void) { return 33; }
int dm1_v1_graphic_slot_box_wounded_pc34(void) { return 34; }
int dm1_v1_graphic_slot_box_acting_hand_pc34(void) { return 35; }
int dm1_v1_graphic_party_shield_border_pc34(void) { return 37; }
int dm1_v1_graphic_fire_shield_border_pc34(void) { return 38; }
int dm1_v1_graphic_spell_shield_border_pc34(void) { return 39; }
int dm1_v1_graphic_champion_damage_small_pc34(void) { return 15; }
int dm1_v1_graphic_champion_damage_big_pc34(void) { return 16; }
int dm1_v1_graphic_creature_damage_pc34(void) { return 14; }

int dm1_v1_graphic_wallset0_index_pc34(int wallSetIndex) {
    if (wallSetIndex < 0 || wallSetIndex >= 15) {
        return -1;
    }
    /* ReDMCSB DEFS.H C093..C107: wall set 0 source graphics are contiguous
     * in DM1_WALL_* order used by DUNVIEW.C side/front wall draw helpers. */
    return 93 + wallSetIndex;
}

int dm1_v1_graphic_materialized_wallset_index_pc34(
    int mapWallSet,
    int wallSet0GraphicIndex) {
    enum {
        DM1_V1_GRAPHIC_WALLSET_MATERIALIZED_FIRST_PC34 = 86,
        DM1_V1_GRAPHIC_WALLSET_MATERIALIZED_COUNT_PC34 = 40
    };
    if (wallSet0GraphicIndex < DM1_V1_GRAPHIC_WALLSET_MATERIALIZED_FIRST_PC34 ||
        wallSet0GraphicIndex >=
            DM1_V1_GRAPHIC_WALLSET_MATERIALIZED_FIRST_PC34 +
            DM1_V1_GRAPHIC_WALLSET_MATERIALIZED_COUNT_PC34) {
        return wallSet0GraphicIndex;
    }
    if (mapWallSet < 0) {
        mapWallSet = 0;
    }
    /* ReDMCSB DUNVIEW.C F0096/F0128: current-map wall-set material is a
     * fixed-width wall-set block; callers pass a wall-set-0 graphic id. */
    return DM1_V1_GRAPHIC_WALLSET_MATERIALIZED_FIRST_PC34 +
           mapWallSet * DM1_V1_GRAPHIC_WALLSET_MATERIALIZED_COUNT_PC34 +
           (wallSet0GraphicIndex - DM1_V1_GRAPHIC_WALLSET_MATERIALIZED_FIRST_PC34);
}

int dm1_v1_object_icon_source_zone_pc34(
    int iconIndex,
    DM1_V1_ObjectIconSourceZonePc34* outZone) {
    int localIndex;
    if (!outZone || iconIndex < 0) return 0;
    localIndex = iconIndex % 32;
    outZone->graphic_index = 42 + (iconIndex / 32);
    outZone->x = (localIndex & 0x0F) * 16;
    outZone->y = (localIndex >> 4) * 16;
    outZone->w = 16;
    outZone->h = 16;
    return 1;
}
