#include "csb_v1_viewport_f0115_projectile_metadata_pc34_compat.h"

#include <string.h>

enum {
    CSB_M613_GRAPHIC_FIRST_PROJECTILE = 454, /* ReDMCSB DEFS.H:2388 */
    CSB_M719_GRAPHIC_FIRST_SOUND = 671,      /* ReDMCSB DEFS.H:2393 */
    CSB_C2900_PROJECTILE_ZONE_BASE = 2900,   /* ReDMCSB DEFS.H:4230 */
    CSB_C10_TRANSPARENT_COLOR = 10,          /* ReDMCSB DEFS.H:2088 */
    CSB_PROJECTILE_COORDINATE_STRIDE = 32,
    CSB_PROJECTILE_ZORDER_DISTANCE_STRIDE = 4,
    CSB_PROJECTILE_LOOKUP_RESULT_COUNT = 8
};

static const char s_source_evidence[] =
    "ReDMCSB DUNVIEW.C:F0115:4547-4581 establishes the object/creature/"
    "projectile/explosion pass order; F0115:5668-5671 maps a visible square "
    "through G2028 before the projectile pass; F0115:5681-5683 requires "
    "C14_THING_TYPE_PROJECTILE, a matching cell, and computes C2900_ZONE_ + "
    "row*4+ViewCell; F0115:5691-5694 converts a negative F0142 result from "
    "projectile ordinal to G0210_as_Graphic558_ProjectileAspects; "
    "F0115:5807-5816/5863-5866 applies bitmap-delta and derived-bitmap "
    "metadata; F0115:5881-5882 dispatches F0791 with C10 transparency. "
    "F0116:6361-6480 follows the D3L2/D3R2 door dispatch back into F0115. "
    "DEFS.H:2037-2055 defines PROJECTIL_ASPECT fields and aspect-type bits; "
    "DEFS.H:2088 defines C10; DEFS.H:2388/2393 bounds projectile native "
    "graphics between M613 and M719. CSB lineage Viewport.cpp:1903-1915 "
    "preserves the CSB F1 door room-object dispatch pair around the door.";

/* ReDMCSB: DUNVIEW.C G0210_as_Graphic558_ProjectileAspects:1511-1520 gives
 * the first six PC34 projectile aspects. F0115:5691-5694 indexes this table
 * from the projectile aspect ordinal returned by F0142_DUNGEON_GetProjectileAspect
 * (DUNGEON.C:1164-1222). This compact fixture binds those six ordinals to
 * the two side metadata records F0115 consults before F0791 dispatch. */
static const CSB_V1_ViewportF0115ProjectileMetadataPc34
    csb_v1_viewport_f0115_projectile_table[] = {
        { CSB_V1_F0115_PROJECTILE_ORDINAL_M715, CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
          -1, CSB_M613_GRAPHIC_FIRST_PROJECTILE + 0, CSB_C2900_PROJECTILE_ZONE_BASE,
          1, 1, 1, 1, 0, 0, "ReDMCSB DUNVIEW.C:1511/5691-5694" },
        { CSB_V1_F0115_PROJECTILE_ORDINAL_M715, CSB_V1_F0115_PROJECTILE_SIDE_RIGHT,
          -1, CSB_M613_GRAPHIC_FIRST_PROJECTILE + 1, CSB_C2900_PROJECTILE_ZONE_BASE + 1,
          1, 1, 1, 1, 0, 0, "ReDMCSB DUNVIEW.C:1511/5807" },
        { CSB_V1_F0115_PROJECTILE_ORDINAL_M716, CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
          -1, CSB_M613_GRAPHIC_FIRST_PROJECTILE + 3, CSB_C2900_PROJECTILE_ZONE_BASE,
          1, 1, 2, 1, 3, 18, "ReDMCSB DUNVIEW.C:1512/5691-5694" },
        { CSB_V1_F0115_PROJECTILE_ORDINAL_M716, CSB_V1_F0115_PROJECTILE_SIDE_RIGHT,
          -1, CSB_M613_GRAPHIC_FIRST_PROJECTILE + 4, CSB_C2900_PROJECTILE_ZONE_BASE + 1,
          1, 1, 2, 1, 3, 18, "ReDMCSB DUNVIEW.C:1512/5807" },
        { CSB_V1_F0115_PROJECTILE_ORDINAL_M717, CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
          -1, CSB_M613_GRAPHIC_FIRST_PROJECTILE + 6, CSB_C2900_PROJECTILE_ZONE_BASE,
          1, 1, 3, 0, 6, 36, "ReDMCSB DUNVIEW.C:1513/5691-5694" },
        { CSB_V1_F0115_PROJECTILE_ORDINAL_M717, CSB_V1_F0115_PROJECTILE_SIDE_RIGHT,
          -1, CSB_M613_GRAPHIC_FIRST_PROJECTILE + 7, CSB_C2900_PROJECTILE_ZONE_BASE + 1,
          1, 1, 3, 0, 6, 36, "ReDMCSB DUNVIEW.C:1513/5807" },
        { CSB_V1_F0115_PROJECTILE_ORDINAL_M718, CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
          -1, CSB_M613_GRAPHIC_FIRST_PROJECTILE + 9, CSB_C2900_PROJECTILE_ZONE_BASE,
          1, 1, 4, 2, 9, 54, "ReDMCSB DUNVIEW.C:1514/5691-5694" },
        { CSB_V1_F0115_PROJECTILE_ORDINAL_M718, CSB_V1_F0115_PROJECTILE_SIDE_RIGHT,
          -1, CSB_M613_GRAPHIC_FIRST_PROJECTILE + 10, CSB_C2900_PROJECTILE_ZONE_BASE + 1,
          1, 1, 4, 2, 9, 54, "ReDMCSB DUNVIEW.C:1514/5807" },
        { CSB_V1_F0115_PROJECTILE_ORDINAL_M719, CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
          -1, CSB_M613_GRAPHIC_FIRST_PROJECTILE + 11, CSB_C2900_PROJECTILE_ZONE_BASE,
          1, 1, 5, 1, 11, 54, "ReDMCSB DUNVIEW.C:1515/5691-5694" },
        { CSB_V1_F0115_PROJECTILE_ORDINAL_M719, CSB_V1_F0115_PROJECTILE_SIDE_RIGHT,
          -1, CSB_M613_GRAPHIC_FIRST_PROJECTILE + 12, CSB_C2900_PROJECTILE_ZONE_BASE + 1,
          1, 1, 5, 1, 11, 54, "ReDMCSB DUNVIEW.C:1515/5807" },
        { CSB_V1_F0115_PROJECTILE_ORDINAL_M720, CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
          -1, CSB_M613_GRAPHIC_FIRST_PROJECTILE + 14, CSB_C2900_PROJECTILE_ZONE_BASE,
          1, 1, 6, 0, 14, 72, "ReDMCSB DUNVIEW.C:1516/5691-5694" },
        { CSB_V1_F0115_PROJECTILE_ORDINAL_M720, CSB_V1_F0115_PROJECTILE_SIDE_RIGHT,
          -1, CSB_M613_GRAPHIC_FIRST_PROJECTILE + 15, CSB_C2900_PROJECTILE_ZONE_BASE + 1,
          1, 1, 6, 0, 14, 72, "ReDMCSB DUNVIEW.C:1516/5807" }
    };

size_t csb_v1_viewport_f0115_projectile_metadata_table_count(void)
{
    return sizeof(csb_v1_viewport_f0115_projectile_table) /
           sizeof(csb_v1_viewport_f0115_projectile_table[0]);
}

const CSB_V1_ViewportF0115ProjectileMetadataPc34 *
csb_v1_viewport_f0115_projectile_metadata_table_entry(size_t index)
{
    if (index >= csb_v1_viewport_f0115_projectile_metadata_table_count()) {
        return 0;
    }
    return &csb_v1_viewport_f0115_projectile_table[index];
}

const CSB_V1_ViewportF0115ProjectileMetadataPc34 *
csb_v1_viewport_f0115_projectile_metadata_lookup(
    int ordinal,
    int side,
    int coordinateSet)
{
    static CSB_V1_ViewportF0115ProjectileMetadataPc34
        s_results[CSB_PROJECTILE_LOOKUP_RESULT_COUNT];
    static unsigned int s_next_result;

    if ((side != CSB_V1_F0115_PROJECTILE_SIDE_LEFT &&
         side != CSB_V1_F0115_PROJECTILE_SIDE_RIGHT) ||
        coordinateSet < CSB_V1_F0115_PROJECTILE_COORDINATE_SET_NEAR ||
        coordinateSet > CSB_V1_F0115_PROJECTILE_COORDINATE_SET_FAR) {
        return 0;
    }

    for (size_t i = 0; i < csb_v1_viewport_f0115_projectile_metadata_table_count(); ++i) {
        const CSB_V1_ViewportF0115ProjectileMetadataPc34 *base =
            &csb_v1_viewport_f0115_projectile_table[i];
        if (base->ordinal == ordinal && base->side == side) {
            CSB_V1_ViewportF0115ProjectileMetadataPc34 *out =
                &s_results[s_next_result++ % CSB_PROJECTILE_LOOKUP_RESULT_COUNT];
            *out = *base;
            out->coordinateSet = coordinateSet;
            out->bitmapIndex =
                base->bitmapIndex + (coordinateSet * CSB_PROJECTILE_COORDINATE_STRIDE);
            out->zOrder =
                base->zOrder + (coordinateSet * CSB_PROJECTILE_ZORDER_DISTANCE_STRIDE);
            out->transparentFlag = (CSB_C10_TRANSPARENT_COLOR == 10);
            return out;
        }
    }

    return 0;
}

int csb_v1_viewport_f0115_projectile_metadata_min_bitmap_pc34(void)
{
    return CSB_M613_GRAPHIC_FIRST_PROJECTILE;
}

int csb_v1_viewport_f0115_projectile_metadata_max_bitmap_pc34(void)
{
    return CSB_M719_GRAPHIC_FIRST_SOUND - 1;
}

const char *csb_v1_viewport_f0115_projectile_metadata_source_evidence_pc34(void)
{
    return s_source_evidence;
}

int csb_v1_f0219_post_teleport_projectile_impact_material_handoff_pc34(
    const CSB_V1_DungeonData *dungeon,
    int map_index,
    int map_x,
    int map_y,
    uint16_t projectile_thing,
    int projectile_aspect_ordinal,
    int side,
    int coordinate_set,
    CSB_V1_F0219ProjectileImpactMaterialHandoffPc34 *out_handoff)
{
    const CSB_V1_ViewportF0115ProjectileMetadataPc34 *metadata;
    const uint8_t *record;
    uint16_t thing;
    int type;
    int size;
    int guard;

    if (!out_handoff) return 0;
    memset(out_handoff, 0, sizeof(*out_handoff));
    if (!dungeon || projectile_thing == 0xfffeu || projectile_thing == 0xffffu ||
        ((projectile_thing >> 10) & 0x0fu) != 14u) {
        return 0;
    }

    /* ReDMCSB PROJEXPL.C F0219 resolves C05 through F0267 before the next
     * F0115 pass.  Walk the authoritative F0161/F0159 list at that resolved
     * square; a raw C14 record alone is not presentation ownership. */
    thing = (uint16_t)csb_v1_dungeon_get_first_thing(
        dungeon, map_index, map_x, map_y);
    for (guard = 0; thing != 0xfffeu && guard < 1024; ++guard) {
        if (thing == projectile_thing) break;
        thing = csb_v1_dungeon_f0159_get_next_thing_pc34(dungeon, thing);
    }
    if (thing != projectile_thing || guard >= 1024) return 0;

    record = csb_v1_dungeon_get_thing_record(
        dungeon, projectile_thing, &type, NULL, &size);
    if (!record || type != 14 || size < 8) return 0;
    metadata = csb_v1_viewport_f0115_projectile_metadata_lookup(
        projectile_aspect_ordinal, side, coordinate_set);
    if (!metadata) return 0;

    out_handoff->valid = 1;
    out_handoff->projectileThing = projectile_thing;
    out_handoff->associatedThing = (uint16_t)(record[2] | ((uint16_t)record[3] << 8));
    out_handoff->mapIndex = map_index;
    out_handoff->mapX = map_x;
    out_handoff->mapY = map_y;
    out_handoff->cell = (projectile_thing >> 14) & 3;
    out_handoff->kineticEnergy = record[4];
    out_handoff->attack = record[5];
    out_handoff->bitmapIndex = metadata->bitmapIndex;
    out_handoff->zOrder = metadata->zOrder;
    out_handoff->transparentFlag = metadata->transparentFlag;
    out_handoff->doubleWidthFlag = metadata->doubleWidthFlag;
    out_handoff->projectileAspectOrdinal = metadata->projectileAspectOrdinal;
    out_handoff->projectileAspectType = metadata->projectileAspectType;
    return 1;
}
