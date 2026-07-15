/* F0172-selected G0290 must be the only F0107/M648 source in M11. */

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

int main(void)
{
    FILE *f = fopen(FIRESTAFF_ROOT_PATH "/src/engine/m11_game_view.c", "rb");
    char *source;
    char *material;
    char *material_end;
    long size;
    int ok;

    if (!f || fseek(f, 0, SEEK_END) || (size = ftell(f)) < 0 ||
        fseek(f, 0, SEEK_SET) ||
        !(source = malloc((size_t)size + 1U)) ||
        fread(source, 1, (size_t)size, f) != (size_t)size) {
        return 1;
    }
    source[size] = '\0';
    fclose(f);

    material = strstr(source, "static int m11_dm1_visible_wall_inscription_material(");
    material_end = material ? strstr(material,
        "static int m11_dm1_visible_wall_inscription_presentation(") : NULL;
    ok = material && material_end && material_end > material &&
         contains_between(material, material_end,
            "dm1_v1_inscription_host_material_from_selected_wall_pc34(") &&
         contains_between(material, material_end,
            "state->world.things, cell->inscriptionTextIndex, outMaterial") &&
         !contains_between(material, material_end,
            "dm1_v1_inscription_host_material_from_world_pc34(");
    free(source);
    return ok ? 0 : 1;
}
