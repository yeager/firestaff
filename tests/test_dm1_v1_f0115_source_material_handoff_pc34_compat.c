#include "dm1_v1_f0115_source_material_handoff_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect(int condition, const char *message)
{
    if (!condition) { fprintf(stderr, "FAIL: %s\n", message); return 0; }
    return 1;
}

static DM1_V1_F0115SourcePixelsPc34 surface_for(unsigned int graphic)
{
    /* Test fixture only. Production admission requires decoded PC34 pixels. */
    static const unsigned char pixels[32 * 32] = { 1 };
    DM1_V1_F0115SourcePixelsPc34 surface = {
        pixels, sizeof(pixels), graphic, 32, 32, 1
    };
    return surface;
}

int main(void)
{
    DM1_V1_F0115SourceMaterialInputPc34 input;
    DM1_V1_F0115SourceMaterialHandoffPc34 handoff;
    DM1_V1_F0115SquareMaterialPc34 scheduled;
    DM1_V1_F0115SourcePixelsPc34 surface;
    DM1_ProjectileMaterialResolutionPc34 projectile;
    int ok = 1;

    memset(&input, 0, sizeof(input));
    input.kind = DM1_V1_F0115_SOURCE_MATERIAL_FLOOR_OBJECT_PC34;
    input.thingType = THING_TYPE_WEAPON; input.subtype = 0;
    input.relativeForward = 1; input.relativeCell = 2; input.sourceZoneRow = 1;
    input.pileIndex = 1; input.viewportW = 224; input.viewportH = 136;
    surface = surface_for(dm1_item_sprite_index(input.thingType, input.subtype));
    input.surface = &surface;
    ok &= expect(dm1_v1_f0115_source_material_handoff_pc34(&input, &handoff) &&
                 handoff.valid && !handoff.noDraw && handoff.usesF0791Blit &&
                 handoff.sourceZone == 2506 && handoff.pileIndex == 1 &&
                 handoff.materialFNV1a != 0u,
                 "floor object pile binds C2500/G0209 source pixels");
    ok &= expect(dm1_v1_f0115_source_material_to_square_pc34(
                     DM1_V1_F0128_VIEW_SQUARE_D1C,
                     DM1_V1_F0115_MATERIAL_NORMAL_OBJECT_PC34,
                     &handoff, &scheduled) && scheduled.pixels == handoff.pixels &&
                 scheduled.graphicIndex == handoff.graphicIndex,
                 "admitted floor handoff reaches F0128 scheduler input");
    surface.sourceOwned = 0;
    ok &= expect(dm1_v1_f0115_source_material_handoff_pc34(&input, &handoff) &&
                 !handoff.valid && handoff.noDraw,
                 "unowned pixels cannot become an F0115 fallback");
    ok &= expect(!dm1_v1_f0115_source_material_to_square_pc34(
                     DM1_V1_F0128_VIEW_SQUARE_D1C,
                     DM1_V1_F0115_MATERIAL_NORMAL_OBJECT_PC34,
                     &handoff, &scheduled),
                 "no-draw handoff cannot enter scheduler");

    memset(&input, 0, sizeof(input));
    input.kind = DM1_V1_F0115_SOURCE_MATERIAL_THROWN_OBJECT_PC34;
    input.thingType = THING_TYPE_ARMOUR; input.subtype = 0;
    input.projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    input.relativeForward = 1; input.relativeCell = 2; input.sourceZoneRow = 1;
    input.viewportW = 224; input.viewportH = 136;
    ok &= expect(dm1_v1_projectile_material_resolve_pc34(
                     input.projectileSubtype, input.thingType,
                     input.subtype, 0, &projectile) && projectile.uses_object_aspect,
                 "F0142 selects object aspect for thrown item");
    surface = surface_for((unsigned int)projectile.graphic_index); input.surface = &surface;
    ok &= expect(dm1_v1_f0115_source_material_handoff_pc34(&input, &handoff) &&
                 handoff.valid && handoff.sourceZone == 2906 && handoff.usesF0791Blit,
                 "thrown item binds F0142/G0209 material to C2900");

    memset(&input, 0, sizeof(input));
    input.kind = DM1_V1_F0115_SOURCE_MATERIAL_NATIVE_PROJECTILE_PC34;
    input.thingType = THING_TYPE_PROJECTILE; input.subtype = -1;
    input.projectileSubtype = PROJECTILE_SUBTYPE_FIREBALL;
    input.relativeForward = 1; input.relativeCell = 2; input.sourceZoneRow = 1;
    input.viewportW = 224; input.viewportH = 136;
    ok &= expect(dm1_v1_projectile_material_resolve_pc34(
                     input.projectileSubtype, input.thingType, input.subtype, 0, &projectile) &&
                 !projectile.uses_object_aspect,
                 "F0142 selects M613 material for spell projectile");
    surface = surface_for((unsigned int)projectile.graphic_index); input.surface = &surface;
    ok &= expect(dm1_v1_f0115_source_material_handoff_pc34(&input, &handoff) &&
                 handoff.valid && handoff.graphicIndex == projectile.graphic_index,
                 "native projectile binds M613 source pixels");
    surface.graphicIndex += 1;
    ok &= expect(dm1_v1_f0115_source_material_handoff_pc34(&input, &handoff) &&
                 !handoff.valid && handoff.noDraw,
                 "wrong GRAPHICS.DAT index is fail-closed");

    if (!ok) return 1;
    puts("ok: DM1 F0115 object, pile and projectile material handoffs are source-bound");
    return 0;
}
