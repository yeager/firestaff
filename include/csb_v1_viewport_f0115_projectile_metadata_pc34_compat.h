#ifndef FIRESTAFF_CSB_V1_VIEWPORT_F0115_PROJECTILE_METADATA_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_F0115_PROJECTILE_METADATA_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_dungeon_loader_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_F0115_PROJECTILE_SIDE_LEFT = 0,
    CSB_V1_F0115_PROJECTILE_SIDE_RIGHT = 1,

    CSB_V1_F0115_PROJECTILE_ORDINAL_M715 = 715,
    CSB_V1_F0115_PROJECTILE_ORDINAL_M716 = 716,
    CSB_V1_F0115_PROJECTILE_ORDINAL_M717 = 717,
    CSB_V1_F0115_PROJECTILE_ORDINAL_M718 = 718,
    CSB_V1_F0115_PROJECTILE_ORDINAL_M719 = 719,
    CSB_V1_F0115_PROJECTILE_ORDINAL_M720 = 720,

    CSB_V1_F0115_PROJECTILE_COORDINATE_SET_NEAR = 0,
    CSB_V1_F0115_PROJECTILE_COORDINATE_SET_MIDDLE = 1,
    CSB_V1_F0115_PROJECTILE_COORDINATE_SET_FAR = 2
};

typedef struct {
    int ordinal;
    int side;
    int coordinateSet;
    int bitmapIndex;
    int zOrder;
    int transparentFlag;
    int doubleWidthFlag;
    int projectileAspectOrdinal;
    int projectileAspectType;
    int firstNativeBitmapRelativeIndex;
    int firstDerivedBitmapRelativeIndex;
    const char *sourceAnchor;
} CSB_V1_ViewportF0115ProjectileMetadataPc34;

/* A F0115 presentation handoff is admitted only for the exact live C14
 * handle still linked on F0219's resolved destination square.  The caller
 * supplies the source-derived F0142 aspect ordinal; this boundary neither
 * invents an aspect nor creates a marker when the real Thing ownership is
 * gone. */
typedef struct {
    int valid;
    uint16_t projectileThing;
    uint16_t associatedThing;
    int mapIndex;
    int mapX;
    int mapY;
    int cell;
    int kineticEnergy;
    int attack;
    int bitmapIndex;
    int zOrder;
    int transparentFlag;
    int doubleWidthFlag;
    int projectileAspectOrdinal;
    int projectileAspectType;
} CSB_V1_F0219ProjectileImpactMaterialHandoffPc34;

size_t csb_v1_viewport_f0115_projectile_metadata_table_count(void);

const CSB_V1_ViewportF0115ProjectileMetadataPc34 *
csb_v1_viewport_f0115_projectile_metadata_table_entry(size_t index);

const CSB_V1_ViewportF0115ProjectileMetadataPc34 *
csb_v1_viewport_f0115_projectile_metadata_lookup(
    int ordinal,
    int side,
    int coordinateSet);

int csb_v1_viewport_f0115_projectile_metadata_min_bitmap_pc34(void);
int csb_v1_viewport_f0115_projectile_metadata_max_bitmap_pc34(void);
const char *csb_v1_viewport_f0115_projectile_metadata_source_evidence_pc34(void);

int csb_v1_f0219_post_teleport_projectile_impact_material_handoff_pc34(
    const CSB_V1_DungeonData *dungeon,
    int map_index,
    int map_x,
    int map_y,
    uint16_t projectile_thing,
    int projectile_aspect_ordinal,
    int side,
    int coordinate_set,
    CSB_V1_F0219ProjectileImpactMaterialHandoffPc34 *out_handoff);

#ifdef __cplusplus
}
#endif

#endif
