/* Source gate for ReDMCSB ENTRANCE.C F0797's FM Towns presentation route.
 *
 * F0797 constructs C255, then calls F0128 from (2,0) facing south.  A prior
 * M11 implementation constructed the right five-by-five map but rendered it
 * from (2,2) with direction 0, so the opened C002/C003 doors revealed C004's
 * red placeholder instead of the source-owned micro-dungeon viewport.
 */

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
    fprintf(stderr, "FAIL: missing F0797 M11 source gate: %s\n", needle);
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
    ok = require_text(
             text,
             "#include \"csb_v1_f0797_startend_entrance_micro_dungeon_pc34_compat.h\"") &&
         require_text(text, "static int m11_render_csb_fmtowns_entrance_micro_viewport(") &&
         require_text(text, "CSB_V1_F0797_VIEW_DIRECTION_SOUTH_PC34,") &&
         require_text(text, "CSB_V1_F0797_VIEW_X_PC34,") &&
         require_text(text, "CSB_V1_F0797_VIEW_Y_PC34);") &&
         require_text(text, "state->csbFmtownsStartupWallSetActive = 1;") &&
         require_text(text, "state->csbFmtownsStartupWallSetActive = 0;") &&
         require_text(text, "if (state->csbFmtownsStartupWallSetActive)") &&
         !strstr(text, "csb_v1_viewport_render_frame(&cfg, 2, 2, 0);");
    free(text);
    if (!ok) return 1;
    puts("PASS CSB FM Towns F0797 micro-dungeon M11 source gate");
    return 0;
}
