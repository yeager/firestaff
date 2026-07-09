#ifndef FIRESTAFF_DM1_V1_ENDGAME_LAYOUT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ENDGAME_LAYOUT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_EndgameRectPc34 {
    int x;
    int y;
    int w;
    int h;
} DM1_V1_EndgameRectPc34;

const char* dm1_v1_endgame_layout_source_evidence_pc34(void);

int dm1_v1_endgame_the_end_rect_pc34(DM1_V1_EndgameRectPc34* out);

int dm1_v1_endgame_champion_mirror_zone_id_pc34(int championSlot);
int dm1_v1_endgame_champion_mirror_rect_pc34(
    int championSlot,
    DM1_V1_EndgameRectPc34* out);

int dm1_v1_endgame_champion_portrait_zone_id_pc34(int championSlot);
int dm1_v1_endgame_champion_portrait_rect_pc34(
    int championSlot,
    DM1_V1_EndgameRectPc34* out);

int dm1_v1_endgame_champion_name_origin_pc34(
    int championSlot,
    int* outX,
    int* outY);
int dm1_v1_endgame_champion_skill_origin_pc34(
    int championSlot,
    int skillLineIndex,
    int* outX,
    int* outY);

int dm1_v1_endgame_restart_box_pc34(
    int inner,
    DM1_V1_EndgameRectPc34* out);
int dm1_v1_endgame_quit_box_pc34(
    int inner,
    DM1_V1_EndgameRectPc34* out);

#ifdef __cplusplus
}
#endif

#endif
