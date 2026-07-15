#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <stdio.h>

static int g_failed = 0;

static void expect_int(const char* label, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL %s: got %d expected %d\n",
                label, actual, expected);
        g_failed = 1;
    }
}

static void expect_true(const char* label, int condition)
{
    if (!condition) {
        fprintf(stderr, "FAIL %s\n", label);
        g_failed = 1;
    }
}

int main(void)
{
    int potionAspect;
    DM1_ProjectileMaterialResolutionPc34 resolution;

    expect_int("F0142 fireball ordinal",
               dm1_v1_f0142_get_projectile_aspect_pc34(
                   PROJECTILE_SUBTYPE_FIREBALL, -1, -1, 0),
               -(DM1_PROJ_ASPECT_FIREBALL + 1));
    expect_int("F0142 lightning ordinal",
               dm1_v1_f0142_get_projectile_aspect_pc34(
                   PROJECTILE_SUBTYPE_LIGHTNING_BOLT, -1, -1, 0),
               -(DM1_PROJ_ASPECT_LIGHTNING_BOLT + 1));
    expect_int("F0142 poison cloud ordinal",
               dm1_v1_f0142_get_projectile_aspect_pc34(
                   PROJECTILE_SUBTYPE_POISON_CLOUD, -1, -1, 0),
               -(DM1_PROJ_ASPECT_POISON + 1));
    expect_int("F0142 default spell ordinal",
               dm1_v1_f0142_get_projectile_aspect_pc34(
                   PROJECTILE_SUBTYPE_OPEN_DOOR, -1, -1, 0),
               -(DM1_PROJ_ASPECT_DEFAULT + 1));
    expect_int("F0142 kinetic arrow ordinal",
               dm1_v1_f0142_get_projectile_aspect_pc34(
                   PROJECTILE_SUBTYPE_KINETIC_ARROW, -1, -1, 0),
               -1);

    expect_int("F0142 weapon projectile ordinal",
               dm1_v1_f0142_get_projectile_aspect_pc34(
                   PROJECTILE_SUBTYPE_KINETIC_ARROW,
                   THING_TYPE_WEAPON, 8, 2),
               -2);

    potionAspect = dm1_item_aspect_index(THING_TYPE_POTION, 0);
    expect_true("potion object aspect exists", potionAspect >= 0);
    expect_int("F0142 potion object aspect",
               dm1_v1_f0142_get_projectile_aspect_pc34(
                   PROJECTILE_SUBTYPE_KINETIC_ARROW,
                   THING_TYPE_POTION, 0, 0),
               potionAspect);

    expect_int("F0142 invalid weapon ordinal",
               dm1_v1_f0142_get_projectile_aspect_pc34(
                   PROJECTILE_SUBTYPE_KINETIC_ARROW,
                   THING_TYPE_WEAPON, 8,
                   DM1_PROJECTILE_ASPECT_COUNT + 1),
               DM1_F0142_INVALID_PROJECTILE_ASPECT_PC34);
    expect_int("F0142 unresolved object aspect",
               dm1_v1_f0142_get_projectile_aspect_pc34(
                   PROJECTILE_SUBTYPE_KINETIC_ARROW,
                   THING_TYPE_POTION, 99, 0),
               DM1_F0142_INVALID_PROJECTILE_ASPECT_PC34);

    expect_true("weapon material resolution uses M613 route",
                dm1_v1_projectile_material_resolve_pc34(
                    PROJECTILE_SUBTYPE_KINETIC_ARROW,
                    THING_TYPE_WEAPON, 8, 2, &resolution));
    expect_int("weapon resolution projectile aspect",
               resolution.aspect_index, 1);
    expect_int("weapon resolution graphic",
               resolution.graphic_index,
               dm1_v1_projectile_graphic_index(1, 0));

    expect_true("potion material resolution uses object route",
                dm1_v1_projectile_material_resolve_pc34(
                    PROJECTILE_SUBTYPE_KINETIC_ARROW,
                    THING_TYPE_POTION, 0, 0, &resolution));
    expect_int("potion resolution object aspect",
               resolution.aspect_index, potionAspect);
    expect_int("potion resolution object flag",
               resolution.uses_object_aspect, 1);

    if (g_failed) {
        return 1;
    }
    printf("DM1 F0142 projectile aspect contract PASS\n");
    return 0;
}
