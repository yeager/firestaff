#ifndef NEXUS_V1_TITLE_H
#define NEXUS_V1_TITLE_H

#include "nexus_v1_engine.h"
#include "nexus_v1_rasterizer.h"
#include "nexus_v1_title_sequence.h"
#include <stdint.h>

enum {
    NEXUS_V1_TITLE_RENDER_MAX_RECTS = 40,
    NEXUS_V1_TITLE_MAP_COUNT = 5,
    NEXUS_V1_TITLE_MAP_WIDTH = 512,
    NEXUS_V1_TITLE_MAP_HEIGHT = 224
};

typedef enum {
    NEXUS_V1_TITLE_RENDER_PLAN_TITLE_ART = 0,
    NEXUS_V1_TITLE_RENDER_PLAN_WARNING_ART = 1
} Nexus_V1_TitleRenderPlanKind;

typedef struct {
    int x;
    int y;
    int w;
    int h;
    uint8_t color;
} Nexus_V1_TitleRenderRect;

typedef struct {
    Nexus_V1_TitleRenderPlanKind kind;
    Nexus_V1_BootPhase boot_phase;
    int input_frame;
    int frame_in_phase;
    int title_frame;
    int reveal_y0;
    int reveal_y1;
    uint8_t edge_color;
    int copy_width;
    int copy_height;
    int pulse;
    int prompt_visible;
    uint8_t prompt_base_color;
    Nexus_V1_TitleRenderRect rects[NEXUS_V1_TITLE_RENDER_MAX_RECTS];
    int rect_count;
} Nexus_V1_TitleRenderPlan;

typedef struct {
    uint8_t *pixels;
    int width;
    int height;
    int loaded;
    uint8_t *warning_pixels;
    int warning_width;
    int warning_height;
    int warning_loaded;
    uint32_t warning_palette_rgba[256];
    int warning_palette_loaded;
    uint8_t *decoded_map_pixels[NEXUS_V1_TITLE_MAP_COUNT];
    uint16_t decoded_map_palette[16];
    int decoded_map_count;
    int decoded_map_source_bound;
} Nexus_TitleScreen;

int nexus_title_load(Nexus_TitleScreen *title, Nexus_V1_Engine *engine);
void nexus_title_free(Nexus_TitleScreen *title);
int nexus_title_min_boot_frames(void);
int nexus_title_start_ready_frames(void);
int nexus_title_boot_warning_frames(void);
int nexus_title_boot_start_ready_frames(void);
int nexus_title_boot_reveal_complete(int frame);
int nexus_title_start_ready(int frame);
int nexus_title_full_boot_start_ready(int frame);
int nexus_v1_title_build_render_plan(const Nexus_TitleScreen *title,
                                     int frame,
                                     Nexus_V1_TitleRenderPlan *out_plan);
void nexus_render_title(const Nexus_TitleScreen *title,
                        Nexus_Framebuffer *fb,
                        int frame);

#endif
