/* Source gate for the FM Towns (MEDIA508) compound-wall compositor.
 *
 * ReDMCSB DUNVIEW.C F0122/F0123 reuses the D1LCR backing bitmap for side
 * walls, but routes each through F0635 with C711/C712.  A full-compound
 * blit for a side zone paints the D1C area and makes ordinary corridors
 * look as if a wall is permanently in front of the party.
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

int main(void)
{
    char *text = read_source("src/engine/m11_game_view.c");
    int ok;

    if (!text) {
        fprintf(stderr, "FAIL: could not read M11 source\n");
        return 1;
    }
    /* The original check asserted an intermediate implementation which
     * rejected side-zone compound calls outright.  The renderer now keeps
     * those calls, but clips them to their F0635 destination lane.  That is
     * the required behaviour: side walls remain visible without painting a
     * false wall across D1C.  Keep this source gate coupled to the actual
     * clipping predicate rather than an obsolete centre-only shortcut. */
    ok = strstr(text, "Only a centre-zone call owns the complete compound.") != NULL &&
         strstr(text, "dstX != centreZoneX &&") != NULL &&
         strstr(text, "fbX < M11_VIEWPORT_X + dstX ||") != NULL &&
         strstr(text, "fbX >= M11_VIEWPORT_X + dstX + dstW") != NULL &&
         strstr(text, "fbY < M11_VIEWPORT_Y + dstY ||") != NULL &&
         strstr(text, "fbY >= M11_VIEWPORT_Y + dstY + dstH") != NULL;
    free(text);
    if (!ok) {
        fprintf(stderr, "FAIL: FM Towns compound wall side-zone clipping gate\n");
        return 1;
    }
    puts("PASS DM1 FM Towns compound walls are centre-zone only");
    return 0;
}
