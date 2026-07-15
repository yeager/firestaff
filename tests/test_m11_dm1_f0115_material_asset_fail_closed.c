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
    char *thrown;
    char *item;
    char *creature;
    char *creatureEnd;
    int ok;

    if (!source) return 1;
    thrown = strstr(source, "static int m11_draw_thrown_object_projectile_sprite(");
    item = strstr(source, "static int m11_draw_item_sprite_material(");
    creature = strstr(source, "static int m11_draw_creature_sprite_ex_material(");
    creatureEnd = creature ? strstr(creature, "\n}\n\n/* Draw the outer UI frame") : NULL;
    if (creatureEnd) *creatureEnd = '\0';
    ok = thrown && item && creature && creatureEnd &&
         strstr(thrown, "!slot || !slot->loaded || !slot->pixels") &&
         strstr(item, "F0115 floor-object material must be a decoded PC34") &&
         strstr(item, "!slot || !slot->loaded || !slot->pixels") &&
         strstr(creature, "F0115 draws the C584+ bitmap selected by G0221/G0222") &&
         strstr(creature, "!slot || !slot->loaded || !slot->pixels") &&
         strstr(creature, "sideHint != 0") &&
         !strstr(creature, "maxW = maxW * 70 / 100");
    free(source);
    return ok ? 0 : 1;
}
