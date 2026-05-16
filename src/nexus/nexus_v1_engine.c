
#include "nexus_v1_engine.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#define strcasecmp _stricmp
#else
#include <dirent.h>
#endif

/* Try to find ISO (CUE/BIN) in data directory — cross-platform */
#ifdef _WIN32
#include <windows.h>
static int find_iso(const char *dir, char *cue_path, int max_len) {
    WIN32_FIND_DATAA fd;
    HANDLE h;
    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s\\*.cue", dir);
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    snprintf(cue_path, max_len, "%s\\%s", dir, fd.cFileName);
    FindClose(h);
    return 1;
}
#else
static int find_iso(const char *dir, char *cue_path, int max_len) {
    DIR *d = opendir(dir);
    struct dirent *ent;
    if (!d) return 0;
    while ((ent = readdir(d)) != NULL) {
        int len = (int)strlen(ent->d_name);
        if (len > 4 && (strcasecmp(ent->d_name + len - 4, ".cue") == 0)) {
            snprintf(cue_path, max_len, "%s/%s", dir, ent->d_name);
            closedir(d);
            return 1;
        }
    }
    closedir(d);
    return 0;
}
#endif

/* Check if extracted files exist */
static int has_extracted(const char *dir) {
    char path[512];
    struct stat st;
    snprintf(path, sizeof(path), "%s/DM.BIN", dir);
    if (stat(path, &st) == 0) return 1;
    snprintf(path, sizeof(path), "%s/LEV00.DGN", dir);
    return (stat(path, &st) == 0);
}

int nexus_v1_init(Nexus_V1_Engine *engine, const char *data_dir) {
    char cue_path[512];
    if (!engine || !data_dir) return -1;
    memset(engine, 0, sizeof(*engine));
    strncpy(engine->data_dir, data_dir, sizeof(engine->data_dir) - 1);

    /* Priority: ISO first, extracted files second */
    if (find_iso(data_dir, cue_path, sizeof(cue_path))) {
        int n = nexus_iso_open_cue(&engine->iso, cue_path);
        if (n > 0 && nexus_iso_is_nexus(&engine->iso)) {
            engine->source = NEXUS_SRC_ISO;
            printf("Nexus: opened ISO with %d files\n", n);
        }
    }

    if (engine->source == NEXUS_SRC_NONE && has_extracted(data_dir)) {
        engine->source = NEXUS_SRC_EXTRACTED;
        printf("Nexus: using extracted files from %s\n", data_dir);
    }

    if (engine->source == NEXUS_SRC_NONE) {
        printf("Nexus: no game data found in %s\n", data_dir);
        return -1;
    }

    /* Init game state */
    nexus_v1_game_init(&engine->game, data_dir);
    engine->audio_enabled = 1;

    /* Load font */
    {
        int font_size = 0;
        uint8_t *font_data = nexus_v1_read_file(engine, "FONT256.S2D", &font_size);
        if (font_data) {
            engine->font_loaded = (nexus_v1_font_load(&engine->font, font_data, font_size) > 0);
            free(font_data);
        }
    }

    engine->initialized = 1;
    printf("Nexus V1 engine initialized (source: %s)\n",
        engine->source == NEXUS_SRC_ISO ? "ISO" : "extracted");
    return 0;
}

uint8_t *nexus_v1_read_file(Nexus_V1_Engine *engine, const char *name, int *out_size) {
    uint8_t *buf = NULL;
    if (!engine || !name) return NULL;

    if (engine->source == NEXUS_SRC_ISO) {
        const Nexus_ISOFile *f = nexus_iso_find(&engine->iso, name);
        if (!f) return NULL;
        buf = (uint8_t *)malloc(f->size);
        if (buf) {
            int n = nexus_iso_read_file(&engine->iso, f, buf, (int)f->size);
            if (n < 0) { free(buf); return NULL; }
            if (out_size) *out_size = (int)f->size;
        }
    } else if (engine->source == NEXUS_SRC_EXTRACTED) {
        char path[512];
        FILE *fp;
        long fsize;
        snprintf(path, sizeof(path), "%s/%s", engine->data_dir, name);
        fp = fopen(path, "rb");
        if (!fp) return NULL;
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buf = (uint8_t *)malloc(fsize);
        if (buf) {
            fread(buf, 1, fsize, fp);
            if (out_size) *out_size = (int)fsize;
        }
        fclose(fp);
    }
    return buf;
}

int nexus_v1_load_level(Nexus_V1_Engine *engine, int level) {
    char name[32];
    int size = 0;
    uint8_t *data;

    if (!engine || level < 0 || level > 15) return -1;
    snprintf(name, sizeof(name), "LEV%02d.DGN", level);

    data = nexus_v1_read_file(engine, name, &size);
    if (!data) {
        printf("Nexus: failed to load %s\n", name);
        return -1;
    }

    int r = nexus_v1_level_load(&engine->current_level, data, size, level);
    free(data);
    if (r < 0) return -1;

    engine->level_loaded = 1;
    engine->game.current_level = level;

    /* Update CD audio track */
    int new_track = nexus_v1_cd_track_for_level(level);
    if (new_track != engine->current_cd_track && engine->audio_enabled) {
        engine->current_cd_track = new_track;
        printf("Nexus: CD track %d for level %d\n", new_track, level);
        /* TODO: actual CD audio playback via SDL_mixer */
    }

    return 0;
}

int nexus_v1_load_model(Nexus_V1_Engine *engine, const char *name) {
    int size = 0;
    uint8_t *data;

    if (!engine || !name || engine->model_count >= NEXUS_MAX_MODELS) return -1;

    data = nexus_v1_read_file(engine, name, &size);
    if (!data) return -1;

    if (!nexus_v1_dmdf_is_valid(data, size)) {
        free(data);
        return -1;
    }

    int idx = engine->model_count;
    int r = nexus_v1_dmdf_load(&engine->models[idx], data, size, name);
    free(data);
    if (r < 0) return -1;

    engine->model_count++;
    return idx;
}

void nexus_v1_tick(Nexus_V1_Engine *engine) {
    if (!engine || !engine->initialized) return;
    /* Nexus uses same V1 tick rate as DM1 (55ms / 18.2 Hz).
     * Game logic: movement, combat, creature AI, timer events.
     * TODO: full game logic integration */
}

void nexus_v1_shutdown(Nexus_V1_Engine *engine) {
    int i;
    if (!engine) return;
    for (i = 0; i < engine->model_count; i++)
        nexus_v1_dmdf_free(&engine->models[i]);
    nexus_v1_font_free(&engine->font);
    if (engine->source == NEXUS_SRC_ISO)
        nexus_iso_close(&engine->iso);
    memset(engine, 0, sizeof(*engine));
    printf("Nexus V1 engine shut down\n");
}

