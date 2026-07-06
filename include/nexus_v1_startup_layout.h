#ifndef NEXUS_V1_STARTUP_LAYOUT_H
#define NEXUS_V1_STARTUP_LAYOUT_H

typedef enum {
    NEXUS_V1_STARTUP_HIT_NONE = 0,
    NEXUS_V1_STARTUP_HIT_SAVE_PANEL = 1,
    NEXUS_V1_STARTUP_HIT_SAVE_ROW = 2,
    NEXUS_V1_STARTUP_HIT_CHAMPION_PANEL = 3,
    NEXUS_V1_STARTUP_HIT_CHAMPION_ROW = 4,
    NEXUS_V1_STARTUP_HIT_CHAMPION_FOOTER = 5
} Nexus_V1_StartupHitKind;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} Nexus_V1_StartupRect;

typedef struct {
    Nexus_V1_StartupHitKind kind;
    int row;
} Nexus_V1_StartupHit;

enum {
    NEXUS_V1_STARTUP_TITLE_X = 18,
    NEXUS_V1_STARTUP_TITLE_Y = 14,
    NEXUS_V1_STARTUP_SUBTITLE_Y = 28,
    NEXUS_V1_STARTUP_SAVE_ROW_TEXT_X = 22,
    NEXUS_V1_STARTUP_CHAMPION_ROW_TEXT_X = 36,
    NEXUS_V1_STARTUP_CHAMPION_PORTRAIT_X = 22,
    NEXUS_V1_STARTUP_FOOTER_X = 18,
    NEXUS_V1_STARTUP_FOOTER_Y = 184
};

int nexus_v1_startup_save_row_rect(int row, Nexus_V1_StartupRect *out_rect);
int nexus_v1_startup_save_panel_rect(int row_count,
                                     Nexus_V1_StartupRect *out_rect);
int nexus_v1_startup_save_hit(int row_count,
                              int x,
                              int y,
                              Nexus_V1_StartupHit *out_hit);
int nexus_v1_startup_champion_row_rect(int row,
                                       Nexus_V1_StartupRect *out_rect);
int nexus_v1_startup_champion_footer_rect(Nexus_V1_StartupRect *out_rect);
int nexus_v1_startup_champion_panel_rect(Nexus_V1_StartupRect *out_rect);
int nexus_v1_startup_champion_hit(int champion_count,
                                  int x,
                                  int y,
                                  Nexus_V1_StartupHit *out_hit);

#endif
