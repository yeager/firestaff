/*
 * firestaff_accessibility.c — Accessibility manifest writer for Firestaff
 *
 * Writes a JSON file describing all interactive UI zones in the current frame.
 * External tools (Peekaboo, macOS Accessibility, test harnesses) can read this
 * to identify clickable elements without parsing the framebuffer.
 *
 * Protocol: game writes ~/.firestaff/accessibility.json atomically each frame.
 * Format follows Peekaboo DetectedElement schema for direct integration.
 */

#include "firestaff_accessibility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define MAX_ELEMENTS 128
#define MAX_STRING_LEN 256
#define HOME_DIR_NAME ".firestaff"
#define JSON_FILENAME "accessibility.json"
#define TMP_SUFFIX ".tmp"

/* ── Internal state ───────────────────────────────────────────────── */

static int g_ax_enabled = 0;
static char g_home_dir[MAX_STRING_LEN];
static char g_json_path[MAX_STRING_LEN];
static char g_tmp_path[MAX_STRING_LEN];

static int g_frame_w = 0;
static int g_frame_h = 0;
static char g_game_state[MAX_STRING_LEN];

static FS_AX_Element g_elements[MAX_ELEMENTS];
static int g_element_count = 0;

/* ── JSON string escaping ─────────────────────────────────────────── */

const char* fs_ax_json_escape(const char* str)
{
    static char buf[MAX_STRING_LEN * 2];
    if (!str) return "";

    size_t len = strlen(str);
    size_t j = 0;

    for (size_t i = 0; i < len && j < sizeof(buf) - 2; i++) {
        char c = str[i];
        switch (c) {
        case '"':
            buf[j++] = '\\'; buf[j++] = '"';
            break;
        case '\\':
            buf[j++] = '\\'; buf[j++] = '\\';
            break;
        case '\n':
            buf[j++] = '\\'; buf[j++] = 'n';
            break;
        case '\r':
            buf[j++] = '\\'; buf[j++] = 'r';
            break;
        case '\t':
            buf[j++] = '\\'; buf[j++] = 't';
            break;
        case '\b':
            buf[j++] = '\\'; buf[j++] = 'b';
            break;
        case '\f':
            buf[j++] = '\\'; buf[j++] = 'f';
            break;
        default:
            if ((unsigned char)c < 0x20) {
                /* Control character — encode as \u00XX */
                j += (size_t)snprintf(buf + j, sizeof(buf) - j,
                                      "\\u%04x", (unsigned char)c);
            } else {
                buf[j++] = c;
            }
            break;
        }
    }
    buf[j] = '\0';
    return buf;
}

/* ── Element type → string ────────────────────────────────────────── */

static const char* type_to_string(FS_AX_ElementType type)
{
    switch (type) {
    case FS_AX_BUTTON:        return "button";
    case FS_AX_REGION:        return "region";
    case FS_AX_TEXT:          return "text";
    case FS_AX_SLOT:          return "slot";
    case FS_AX_PORTRAIT:      return "portrait";
    case FS_AX_MOVEMENT:      return "movement";
    case FS_AX_DIALOG_CHOICE: return "dialog_choice";
    case FS_AX_CHAMPION_MIRROR: return "champion_mirror";
    default:                  return "unknown";
    }
}

/* ── Directory creation ───────────────────────────────────────────── */

static int ensure_directory(const char* path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode) ? 0 : -1;
    }
    /* Create directory (just the leaf — home dir should exist) */
    return mkdir(path, 0755);
}

/* ── Public API ───────────────────────────────────────────────────── */

void fs_ax_set_enabled(int enabled)
{
    g_ax_enabled = (enabled != 0) ? 1 : 0;

    /* Populate path components once on enable */
    if (g_ax_enabled) {
        const char* home = getenv("HOME");
        if (!home) home = "";

        /* Build home dir path: ~/.firestaff */
        int dlen = (int)strlen(home);
        int nlen = (int)strlen(HOME_DIR_NAME);
        int tlen = dlen + 1 + nlen;

        if (dlen + 1 + nlen < MAX_STRING_LEN) {
            snprintf(g_home_dir, sizeof(g_home_dir),
                     "%s/%s", home, HOME_DIR_NAME);
            snprintf(g_json_path, sizeof(g_json_path),
                     "%s/%s", g_home_dir, JSON_FILENAME);
            snprintf(g_tmp_path, sizeof(g_tmp_path),
                     "%s%s", g_json_path, TMP_SUFFIX);
        }
    }
}

int fs_ax_is_enabled(void)
{
    return g_ax_enabled;
}

void fs_ax_begin_frame(int framebuffer_width, int framebuffer_height,
                       const char* game_state)
{
    g_frame_w = framebuffer_width;
    g_frame_h = framebuffer_height;

    if (game_state) {
        strncpy(g_game_state, game_state, MAX_STRING_LEN - 1);
        g_game_state[MAX_STRING_LEN - 1] = '\0';
    } else {
        g_game_state[0] = '\0';
    }

    g_element_count = 0;
}

void fs_ax_add_element(const FS_AX_Element* element)
{
    if (!g_ax_enabled) return;
    if (!element) return;
    if (g_element_count >= MAX_ELEMENTS) return;

    g_elements[g_element_count++] = *element;
}

void fs_ax_flush(void)
{
    FILE* fp;
    int dir_result;

    if (!g_ax_enabled) return;

    /* Ensure ~/.firestaff/ exists */
    dir_result = ensure_directory(g_home_dir);
    if (dir_result != 0 && errno != EEXIST) {
        return;
    }

    /* Open temp file for atomic write */
    fp = fopen(g_tmp_path, "w");
    if (!fp) return;

    /* Build JSON — Peekaboo DetectedElement schema */
    fprintf(fp, "{\"version\":1, \"app\":\"firestaff\", ");
    fprintf(fp, "\"gameState\":\"%s\", ",
            fs_ax_json_escape(g_game_state[0] ? g_game_state : ""));

    fprintf(fp, "\"framebuffer\":{\"width\":%d,\"height\":%d}, ",
            g_frame_w, g_frame_h);

    fprintf(fp, "\"elements\":[");

    for (int i = 0; i < g_element_count; i++) {
        FS_AX_Element* e = &g_elements[i];
        if (i > 0) fprintf(fp, ",");

        fprintf(fp, "{\"id\":\"%s\",\"type\":\"%s\",\"label\":\"%s\",",
                fs_ax_json_escape(e->id),
                type_to_string(e->type),
                fs_ax_json_escape(e->label));

        fprintf(fp, "\"bounds\":{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d},",
                e->x, e->y, e->w, e->h);

        fprintf(fp, "\"enabled\":%s, ", e->enabled ? "true" : "false");

        if (e->value) {
            fprintf(fp, "\"value\":\"%s\"}",
                    fs_ax_json_escape(e->value));
        } else {
            fprintf(fp, "\"value\":null}");
        }
    }

    fprintf(fp, "]}\n");

    fclose(fp);

    /* Atomic rename: tmp → target */
    rename(g_tmp_path, g_json_path);
}

void fs_ax_shutdown(void)
{
    /* Delete the JSON file on cleanup */
    if (g_ax_enabled && g_json_path[0]) {
        remove(g_json_path);
    }

    g_ax_enabled = 0;
    g_element_count = 0;
    g_game_state[0] = '\0';
}
