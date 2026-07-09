#include "dm1_v1_endgame_layout_pc34_compat.h"

enum {
    kChampionSlots = 4,
    kChampionSkillLines = 4,
    kPortraitW = 32,
    kPortraitH = 29
};

static int set_rect(DM1_V1_EndgameRectPc34* out,
                    int x,
                    int y,
                    int w,
                    int h) {
    if (!out) return 0;
    out->x = x;
    out->y = y;
    out->w = w;
    out->h = h;
    return 1;
}

static int valid_champion_slot(int championSlot) {
    return championSlot >= 0 && championSlot < kChampionSlots;
}

const char* dm1_v1_endgame_layout_source_evidence_pc34(void) {
    return "ReDMCSB ENDGAME.C F0444/F0445; DATA.C "
           "G0012/G0013/G0014/G0015/G0016; DEFS.H C412..C419";
}

int dm1_v1_endgame_the_end_rect_pc34(DM1_V1_EndgameRectPc34* out) {
    /* ENDGAME.C blits G0012 {120,199,95,108}; runtime rect is x1,y1,w,h. */
    return set_rect(out, 120, 95, 80, 14);
}

int dm1_v1_endgame_champion_mirror_zone_id_pc34(int championSlot) {
    if (!valid_champion_slot(championSlot)) return 0;
    return 412 + championSlot;
}

int dm1_v1_endgame_champion_mirror_rect_pc34(
    int championSlot,
    DM1_V1_EndgameRectPc34* out) {
    if (!valid_champion_slot(championSlot)) return 0;
    return set_rect(out, 19, 7 + championSlot * 48, 48, 43);
}

int dm1_v1_endgame_champion_portrait_zone_id_pc34(int championSlot) {
    if (!valid_champion_slot(championSlot)) return 0;
    return 416 + championSlot;
}

int dm1_v1_endgame_champion_portrait_rect_pc34(
    int championSlot,
    DM1_V1_EndgameRectPc34* out) {
    if (!valid_champion_slot(championSlot)) return 0;
    return set_rect(out,
                    27,
                    13 + championSlot * 48,
                    kPortraitW,
                    kPortraitH);
}

int dm1_v1_endgame_champion_name_origin_pc34(
    int championSlot,
    int* outX,
    int* outY) {
    if (!valid_champion_slot(championSlot)) return 0;
    if (outX) *outX = 87;
    if (outY) *outY = 14 + championSlot * 48;
    return 1;
}

int dm1_v1_endgame_champion_skill_origin_pc34(
    int championSlot,
    int skillLineIndex,
    int* outX,
    int* outY) {
    if (!valid_champion_slot(championSlot) ||
        skillLineIndex < 0 ||
        skillLineIndex >= kChampionSkillLines) {
        return 0;
    }
    if (outX) *outX = 105;
    if (outY) *outY = 23 + championSlot * 48 + skillLineIndex * 8;
    return 1;
}

int dm1_v1_endgame_restart_box_pc34(
    int inner,
    DM1_V1_EndgameRectPc34* out) {
    return set_rect(out,
                    inner ? 105 : 103,
                    inner ? 142 : 140,
                    inner ? 111 : 115,
                    inner ? 11 : 15);
}

int dm1_v1_endgame_quit_box_pc34(
    int inner,
    DM1_V1_EndgameRectPc34* out) {
    return set_rect(out,
                    inner ? 129 : 127,
                    inner ? 167 : 165,
                    inner ? 63 : 67,
                    inner ? 11 : 15);
}
