
#ifndef NEXUS_V2_CONFIG_H
#define NEXUS_V2_CONFIG_H

/* Nexus V2 configuration — mirrors DM1 V2 approach.
 *
 * V2.1 Upscaled: V1 320x200 → EPX 2x → 640x400, same game logic
 * V2.2 Enhanced: full feature set on top of V2.1
 *
 * V1 tick rate (55ms) preserved exactly — V2 only changes rendering. */

typedef enum {
    NEXUS_V2_OFF = 0,       /* V1 Original */
    NEXUS_V2_UPSCALED,      /* V2.1: EPX + bilinear filter */
    NEXUS_V2_ENHANCED        /* V2.2: full enhanced features */
} Nexus_V2_Mode;

typedef struct {
    Nexus_V2_Mode mode;

    /* V2.1 Upscaled settings */
    int upscale_factor;      /* 2 (EPX) or 3 (EPX3) */
    int bilinear_filter;     /* smooth after EPX */
    int widescreen;          /* 16:9 viewport expansion */
    int render_width;        /* output width (640, 960, 1280) */
    int render_height;       /* output height (400, 600, 800) */

    /* V2.2 Enhanced settings */
    int smooth_movement;     /* interpolated party movement */
    int dynamic_lighting;    /* per-vertex torch/spell lighting */
    int texture_filtering;   /* bilinear on wall textures */
    int ambient_occlusion;   /* soft shadows in corners */
    int particles;           /* dust, sparks, magic effects */
    int fog;                 /* distance fog (depth-based) */
    int reflective_floors;   /* water/polished stone reflections */
    int enhanced_creatures;  /* higher-poly DMDF + smooth animation */
    int camera_bob;          /* subtle head-bob while walking */
    int screen_shake;        /* on damage, door slam, explosions */
    int weather_fx;          /* dripping water, dust, cold breath */
    int enhanced_audio;      /* CD audio + positional SFX */
    int minimap;             /* translucent corner minimap */
    int damage_numbers;      /* floating combat numbers */
    int journal;             /* auto-journal of events */
    int achievements;        /* Saturn-era milestones */
    int footstep_audio;      /* surface-aware footstep sounds */
    int torch_flicker;       /* dynamic light wobble */
    int portrait_animations; /* champion face reactions */
    int inventory_sort;      /* auto-sort by type/weight */
} Nexus_V2_Config;

void nexus_v2_config_init(Nexus_V2_Config *cfg, Nexus_V2_Mode mode);

#endif

