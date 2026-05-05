#ifndef FIRESTAFF_DM1_V2_ITEM_RENDER_PC34_H
#define FIRESTAFF_DM1_V2_ITEM_RENDER_PC34_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V2_ITEM_SURFACE_FLOOR = 0,
    DM1_V2_ITEM_SURFACE_ACTION_HAND = 1,
    DM1_V2_ITEM_SURFACE_INVENTORY_HAND = 2,
    DM1_V2_ITEM_SURFACE_ALCOVE = 3
} DM1_V2_ItemSurface;

typedef enum {
    DM1_V2_CELL_LAYER_FLOOR_ITEM = 0,
    DM1_V2_CELL_LAYER_CREATURE = 1,
    DM1_V2_CELL_LAYER_PROJECTILE = 2,
    DM1_V2_CELL_LAYER_EXPLOSION = 3,
    DM1_V2_CELL_LAYER_FLUXCAGE = 4
} DM1_V2_CellLayer;

typedef struct {
    const char* assetId;
    DM1_V2_ItemSurface surface;
    int drawLayer;
    int sourceCellOrdinal;
    int supportsSubcellOffset;
} DM1_V2_ItemRenderBinding;

const DM1_V2_ItemRenderBinding* dm1_v2_item_render_empty_hand_binding(void);
const DM1_V2_ItemRenderBinding* dm1_v2_item_render_floor_item_binding(void);
int dm1_v2_item_render_layer_precedes(DM1_V2_CellLayer earlier, DM1_V2_CellLayer later);
const char* dm1_v2_item_render_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_ITEM_RENDER_PC34_H */
