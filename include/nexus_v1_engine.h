#ifndef NEXUS_V1_ENGINE_H
#define NEXUS_V1_ENGINE_H

/* ── Forward declaration for mechanics.h ─────────────────────────────
 * Define both struct tag and typedef alias so all translation units
 * that include this header (directly or transitively through
 * mechanics.h) can use Nexus_V1_Engine by name as either a typedef
 * or struct tag. Set a guard so mechanics headers skip their own
 * re-declaration. */
#ifndef NEXUS_ENGINE_FWD_FROM_HEADERS
#define NEXUS_ENGINE_FWD_FROM_HEADERS
struct Nexus_V1_Engine;
typedef struct Nexus_V1_Engine Nexus_V1_Engine;
#endif

/* ── Data source headers (order matters: low-level first) ──────────── */
#include "nexus_v1_iso_reader.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_game.h"
#include "nexus_v1_dmdf_model.h"
#include "nexus_v1_saturn_font.h"
#include "nexus_v1_text.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_creatures.h"
#include "nexus_v1_sound.h"
#include <stdint.h>

/* ── Constants ─────────────────────────────────────────────────────── */
#define NEXUS_MAX_MODELS 64

/* ── Enumerations ──────────────────────────────────────────────────── */
typedef enum {
    NEXUS_SRC_NONE = 0,
    NEXUS_SRC_ISO,         /* Reading from .cue/.bin disc image */
    NEXUS_SRC_EXTRACTED    /* Reading from extracted files on disk */
} Nexus_DataSource;

/* ── Main engine struct ─────────────────────────────────────────────── */
struct Nexus_V1_Engine {
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

    /* Champion pool — full champion roster + party management */
    Nexus_V1_ChampionPool champions;

    /* Creature manager */
    Nexus_V1_CreatureManager creatures;

    /* Mechanics state — opaque pointer, allocated in nexus_v1_init().
     * Defined in nexus_v1_mechanics.c (source-locked DM1 game loop).
     * Source: DM1 CLIKMENU.C F0366, MOVESENS.C F0267. */
    struct Nexus_MechanicsState *mechanics;

    /* Audio */
    Nexus_SoundEngine audio;
    int current_cd_track;
    int audio_enabled;

    /* Initialized flag */
    int initialized;
};

/* ── Engine API ─────────────────────────────────────────────────────── */

/* Initialize from data directory (auto-detects ISO vs extracted).
 * Returns 0 on success, -1 on no data found. */
int nexus_v1_init(Nexus_V1_Engine *engine, const char *data_dir);

/* Load a dungeon level (0-15). Calls nexus_v1_level_load().
 * Returns 0 on success, -1 on failure. */
int nexus_v1_load_level(Nexus_V1_Engine *engine, int level);

/* Load a 3D creature model by filename (e.g. "SCORPION.MNS").
 * Returns model index (>=0) on success, -1 on failure. */
int nexus_v1_load_model(Nexus_V1_Engine *engine, const char *name);

/* Read any file from the disc image or extracted directory.
 * Caller owns returned buffer; free with free(). */
uint8_t *nexus_v1_read_file(Nexus_V1_Engine *engine, const char *name, int *out_size);

/* Handle pending level transition (called after mechanics signals).
 * Source: DM1 CLIKMENU.C F0364. */
int nexus_v1_engine_level_change(Nexus_V1_Engine *engine, int *out_new_level);

/* Game tick — call each 55ms (18.2 Hz).
 * Source: DM1 CLIKMENU.C:269-323, F0366. */
void nexus_v1_tick(Nexus_V1_Engine *engine);

/* Shutdown and free all resources */
void nexus_v1_shutdown(Nexus_V1_Engine *engine);

#endif /* NEXUS_V1_ENGINE_H */
