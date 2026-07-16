#include "dm1_v1_graphic_ids_pc34_compat.h"

const char* dm1_v1_graphic_ids_source_evidence_pc34(void) {
    return "ReDMCSB DEFS.H C006/C007/C008/C014..C020/C023/C026/C028..C039/"
           "C346; PANEL.C F0339/F0341/F0342; CHAMDRAW.C F0291/F0622; "
           "ENDGAME.C F0445";
}

int dm1_v1_graphic_dialog_box_pc34(void) { return 17; }
int dm1_v1_graphic_the_end_pc34(void) { return 6; }
int dm1_v1_graphic_endgame_champion_mirror_pc34(void) { return 346; }
int dm1_v1_graphic_champion_portraits_pc34(void) { return 26; }
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

int dm1_v1_object_icon_source_zone_pc34(
    int iconIndex,
    DM1_V1_ObjectIconSourceZonePc34* outZone) {
    if (!outZone || iconIndex < 0) {
        return 0;
    }
    outZone->graphic_index = 42 + (iconIndex / 16);
    outZone->x = (iconIndex % 16) * 16;
    outZone->y = 0;
    outZone->w = 16;
    outZone->h = 16;
    return 1;
}

int dm1_v1_graphic_champion_portrait_source_zone_pc34(
    int portraitIndex,
    DM1_V1_ObjectIconSourceZonePc34* outZone) {
    if (!outZone || portraitIndex < 0 || portraitIndex >= 24) {
        return 0;
    }
    outZone->graphic_index = dm1_v1_graphic_champion_portraits_pc34();
    outZone->x = (portraitIndex % 8) * 32;
    outZone->y = (portraitIndex / 8) * 29;
    outZone->w = 32;
    outZone->h = 29;
    return 1;
}

int dm1_v1_graphic_validate_champion_portrait_atlas_pc34(
    int width,
    int height) {
    return width >= 256 && height >= 87;
}
