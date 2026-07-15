#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_ROOT_PATH
#error "FIRESTAFF_ROOT_PATH required"
#endif

static int contains_between(const char *begin, const char *end,
                            const char *needle)
{
    size_t length;
    const char *cursor;

    if (!begin || !end || end < begin || !needle) return 0;
    length = strlen(needle);
    for (cursor = begin; cursor + length <= end; ++cursor) {
        if (memcmp(cursor, needle, length) == 0) return 1;
    }
    return 0;
}

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
    char *f0114;
    char *f0114_end;
    char *d0c;
    char *d0c_end;
    int ok;

    if (!source) return 1;
    f0114 = strstr(source, "static int m11_draw_explosion_sprite_bound_ex(");
    f0114_end = f0114
        ? strstr(f0114, "static int m11_draw_explosion_sprite(")
        : NULL;
    d0c = strstr(source, "static int m11_draw_d0c_explosion_pattern(");
    d0c_end = d0c
        ? strstr(d0c, "static int m11_draw_explosion_material(")
        : NULL;
    ok = f0114 && f0114_end && d0c && d0c_end &&
         contains_between(f0114, f0114_end,
                          "F0114 may scale only a decoded PC34 explosion surface.") &&
         contains_between(f0114, f0114_end,
                          "!slot->loaded || !slot->pixels") &&
         contains_between(d0c, d0c_end,
                          "D0C M636 route is distinct from F0114") &&
         contains_between(d0c, d0c_end,
                          "!slot->loaded || !slot->pixels");
    free(source);
    return ok ? 0 : 1;
}
