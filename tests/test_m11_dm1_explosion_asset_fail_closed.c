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
    char *f0114;
    char *d0c;
    int ok;

    if (!source) return 1;
    f0114 = strstr(source, "static int m11_draw_explosion_sprite_bound_ex(");
    d0c = strstr(source, "static int m11_draw_d0c_explosion_pattern(");
    ok = f0114 && d0c &&
         strstr(f0114, "F0114 may scale only a decoded PC34 explosion surface.") &&
         strstr(f0114, "!slot->loaded || !slot->pixels") &&
         strstr(d0c, "D0C M636 route is distinct from F0114") &&
         strstr(d0c, "!slot->loaded || !slot->pixels");
    free(source);
    return ok ? 0 : 1;
}
