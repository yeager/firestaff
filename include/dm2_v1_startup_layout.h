#ifndef DM2_V1_STARTUP_LAYOUT_H
#define DM2_V1_STARTUP_LAYOUT_H

typedef enum {
    DM2_V1_STARTUP_HIT_NONE = 0,
    DM2_V1_STARTUP_HIT_PANEL = 1,
    DM2_V1_STARTUP_HIT_ROW = 2
} DM2_V1_StartupHitKind;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} DM2_V1_StartupRect;

typedef struct {
    DM2_V1_StartupHitKind kind;
    int row;
} DM2_V1_StartupHit;

enum {
    DM2_V1_STARTUP_TITLE_X = 94,
    DM2_V1_STARTUP_TITLE_Y = 58,
    DM2_V1_STARTUP_SUBTITLE_X = 96,
    DM2_V1_STARTUP_SUBTITLE_Y = 68,
    DM2_V1_STARTUP_ROW_TEXT_X = 96,
    DM2_V1_STARTUP_FOOTER_X = 90,
    DM2_V1_STARTUP_FOOTER_Y = 158
};

int dm2_v1_startup_panel_rect(DM2_V1_StartupRect *out_rect);
int dm2_v1_startup_row_rect(int row, DM2_V1_StartupRect *out_rect);
int dm2_v1_startup_row_highlight_rect(int row,
                                      DM2_V1_StartupRect *out_rect);
int dm2_v1_startup_hit(int row_count,
                       int x,
                       int y,
                       DM2_V1_StartupHit *out_hit);

#endif
