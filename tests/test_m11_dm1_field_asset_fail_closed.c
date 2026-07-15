#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_ROOT_PATH
#error "FIRESTAFF_ROOT_PATH required"
#endif

static char *read_source(void)
{
    FILE *file = fopen(FIRESTAFF_ROOT_PATH "/src/engine/m11_game_view.c", "rb");
    long length;
    char *text;

    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (length = ftell(file)) < 0 || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    text = (char *)malloc((size_t)length + 1U);
    if (!text || fread(text, 1U, (size_t)length, file) != (size_t)length) {
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
    char *source = read_source();
    char *field_route;
    char *field_route_end;
    int ok;

    if (!source) return 1;
    field_route = strstr(source, "static int m11_draw_dm1_field_zone(");
    field_route_end = field_route
        ? strstr(field_route, "static void m11_blit_scaled_palette_map")
        : NULL;
    ok = field_route && field_route_end && field_route_end > field_route &&
         strstr(field_route, "F0113's C070..C075 mask owns") &&
         strstr(field_route,
                "degraded form of the original one.");
    free(source);
    return ok ? 0 : 1;
}
