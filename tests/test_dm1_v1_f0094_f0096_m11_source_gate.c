/* Source lock for the M11 endpoints of ReDMCSB DUNVIEW.C F0094/F0096.
 * These helpers must fail closed when there is no active DM1 map. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_source(const char *path)
{
    FILE *file = fopen(path, "rb");
    long length;
    char *text;

    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) < 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    text = (char *)malloc((size_t)length + 1u);
    if (!text) {
        fclose(file);
        return NULL;
    }
    if (fread(text, 1u, (size_t)length, file) != (size_t)length) {
        free(text);
        fclose(file);
        return NULL;
    }
    text[length] = '\0';
    fclose(file);
    return text;
}

static int require_text(const char *text, const char *needle)
{
    if (strstr(text, needle)) return 1;
    fprintf(stderr, "FAIL: missing M11 source gate: %s\n", needle);
    return 0;
}

int main(void)
{
    char *text = read_source("src/engine/m11_game_view.c");
    int ok;

    if (!text) {
        fprintf(stderr, "FAIL: could not read M11 source\n");
        return 1;
    }
    ok = require_text(text, "M11_GFX_UNAVAILABLE") &&
         require_text(text,
                      "static int m11_current_map_floor_set(const M11_GameViewState* state,") &&
         require_text(text, "if (!m11_current_map_floor_set(state, &floor_set))") &&
         require_text(text,
                      "static int m11_current_map_wall_set(const M11_GameViewState* state,") &&
         require_text(text, "if (!m11_current_map_wall_set(state, &map_wall_set))") &&
         require_text(text, "if (!m11_current_map_wall_set(state, &mapWallSet))") &&
         !strstr(text, ") ? (int)state->world.dungeon->maps[state->world.party.mapIndex].wallSet\n                : 0,");
    if (!ok) {
        free(text);
        return 1;
    }
    free(text);
    puts("PASS dm1 F0094/F0096 M11 active-map source gate");
    return 0;
}
