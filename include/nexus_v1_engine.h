
#ifndef NEXUS_V1_ENGINE_H
#define NEXUS_V1_ENGINE_H

#include "nexus_v1_iso_reader.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_game.h"
#include "nexus_v1_dmdf_model.h"
#include "nexus_v1_saturn_font.h"
#include "nexus_v1_text.h"
#include <stdint.h>

/* Maximum creatures with 3D models */
#define NEXUS_MAX_MODELS 64

typedef enum {
    NEXUS_SRC_NONE = 0,
    NEXUS_SRC_ISO,         /* Reading from .cue/.bin disc image */
    NEXUS_SRC_EXTRACTED    /* Reading from extracted files on disk */
} Nexus_DataSource;

typedef struct {
    /* Data source */
    Nexus_DataSource source;
    Nexus_ISOReader iso;
    char data_dir[512];

    /* Game state */
    Nexus_V1_GameState game;

    /* Current level */
    Nexus_V1_Level current_level;
    int level_loaded;

    /* 3D models (loaded on demand) */
    Nexus_V1_Model models[NEXUS_MAX_MODELS];
    int model_count;

    /* Font */
    Nexus_V1_Font font;
    int font_loaded;

    /* Audio */
    int current_cd_track;
    int audio_enabled;

    /* Initialized flag */
    int initialized;
} Nexus_V1_Engine;

/* Initialize from data directory (auto-detects ISO vs extracted) */
int nexus_v1_init(Nexus_V1_Engine *engine, const char *data_dir);

/* Load a dungeon level (0-15) */
int nexus_v1_load_level(Nexus_V1_Engine *engine, int level);

/* Load a 3D creature model by filename (e.g. "SCORPION.MNS") */
int nexus_v1_load_model(Nexus_V1_Engine *engine, const char *name);

/* Read any file from the disc image or extracted directory */
uint8_t *nexus_v1_read_file(Nexus_V1_Engine *engine, const char *name, int *out_size);

/* Game tick (call at V1 rate: 55ms) */
void nexus_v1_tick(Nexus_V1_Engine *engine);

/* Shutdown */
void nexus_v1_shutdown(Nexus_V1_Engine *engine);

#endif

