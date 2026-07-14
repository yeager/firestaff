/* DM1 V1 Wall Ornament Rendering — source-locked from ReDMCSB
 * DUNVIEW.C: F0107_IsDrawnWallOrnamentAnAlcove_CPSF (line 3502)
 * DUNVIEW.C: ornament coordinate sets from graphic 558
 * DRAWVIEW.C: wall ornament overlay on viewport walls
 * Ornaments: alcoves, switches, keyholes, inscriptions, fountains */
#ifndef FIRESTAFF_DM1_V1_WALL_ORNAMENT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_WALL_ORNAMENT_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_WALL_ORN_MAX       16   /* Max wall ornament types per level */
#define DM1_FLOOR_ORN_MAX      16   /* Max floor ornament types per level */
#define DM1_DOOR_ORN_MAX        1   /* Max door ornament types per level */
#define DM1_ORN_COORD_SETS     12   /* Coordinate sets for depth/side */

/* Ornament types from ReDMCSB */
typedef enum {
    DM1_V1_WALL_ORN_NONE = 0,
    DM1_V1_WALL_ORN_ALCOVE,            /* Interactive alcove (item storage) */
    DM1_V1_WALL_ORN_SWITCH,            /* Wall switch (lever/button) */
    DM1_V1_WALL_ORN_KEYHOLE,           /* Keyhole */
    DM1_V1_WALL_ORN_INSCRIPTION,       /* Text inscription */
    DM1_V1_WALL_ORN_FOUNTAIN,          /* Fountain (water source) */
    DM1_V1_WALL_ORN_TORCH_HOLDER,      /* Wall torch */
    DM1_V1_WALL_ORN_GENERIC            /* Generic decorative ornament */
} DM1_V1_WallOrnamentKindPc34;

/* Ornament coordinate set — where to draw at each depth/side */
typedef struct {
    int16_t x, y;             /* Position relative to wall zone */
    int16_t w, h;             /* Ornament bitmap dimensions at this depth */
    int16_t depth;            /* View depth (0=closest, 3=farthest) */
    int16_t side;             /* 0=left, 1=center, 2=right */
} DM1_V1_WallOrnamentCoordPc34;

/* Ornament definition */
typedef struct {
    uint16_t gfx_index;       /* Bitmap index in GRAPHICS.DAT */
    DM1_V1_WallOrnamentKindPc34 kind;
    bool is_alcove;            /* F0107 check result */
    bool is_interactive;       /* Can be clicked */
    DM1_V1_WallOrnamentCoordPc34 coords[DM1_ORN_COORD_SETS]; /* Per depth/side */
} DM1_V1_WallOrnamentDefPc34;

typedef struct {
    DM1_V1_WallOrnamentDefPc34 wall_ornaments[DM1_WALL_ORN_MAX];
    DM1_V1_WallOrnamentDefPc34 floor_ornaments[DM1_FLOOR_ORN_MAX];
    DM1_V1_WallOrnamentDefPc34 door_ornaments[DM1_DOOR_ORN_MAX];
    uint8_t wall_count;
    uint8_t floor_count;
    uint8_t door_count;
} DM1_V1_WallOrnamentStatePc34;

typedef struct DM1_WallOrnamentZoneBlitPc34 {
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
} DM1_WallOrnamentZoneBlitPc34;

typedef struct DM1_WallOrnamentViewSpecPc34 {
    int relForward;
    int relSide;
    int viewWallIndex;
    int unreadableInscriptionCompactBox;
} DM1_WallOrnamentViewSpecPc34;

typedef struct DM1_WallOrnamentRenderPlanPc34 {
    int graphicIndex;
    int srcX;
    int srcY;
    int dstX;
    int dstY;
    int width;
    int height;
    int transparentColor;
    int flipHorizontal;
    int paletteMapValid;
    unsigned char paletteMap[16];
    int isAlcove;
} DM1_WallOrnamentRenderPlanPc34;

typedef struct DM1_FrontMirrorRenderPlanPc34 {
    DM1_WallOrnamentRenderPlanPc34 ornament;
    int portraitGraphicIndex;
    int portraitSrcX;
    int portraitSrcY;
    int portraitDstX;
    int portraitDstY;
    int portraitWidth;
    int portraitHeight;
    int portraitTransparentColor;
    int backingDstX;
    int backingDstY;
    int backingWidth;
    int backingHeight;
} DM1_FrontMirrorRenderPlanPc34;

void DM1_V1_WallOrnament_InitPc34Compat(DM1_V1_WallOrnamentStatePc34* state);
void DM1_V1_WallOrnament_SetLevelOrnamentsPc34Compat(DM1_V1_WallOrnamentStatePc34* state,
                                 uint8_t wall_count, uint8_t floor_count,
                                 uint8_t door_count);
bool DM1_V1_WallOrnament_IsAlcovePc34Compat(const DM1_V1_WallOrnamentDefPc34* orn);
const DM1_V1_WallOrnamentCoordPc34* DM1_V1_WallOrnament_GetCoordPc34Compat(const DM1_V1_WallOrnamentDefPc34* orn,
                                          int16_t depth, int16_t side);
void DM1_V1_WallOrnament_SetupDefaultCoordsPc34Compat(DM1_V1_WallOrnamentDefPc34* orn);

int dm1_v1_wall_ornament_coord_set_index_pc34(int globalIndex);
int dm1_v1_wall_ornament_zone_pc34(int coordSet,
                                   int viewWallIndex,
                                   DM1_WallOrnamentZoneBlitPc34* outBlit);
int dm1_v1_wall_ornament_flip_horizontal_pc34(int viewWallIndex);
int dm1_v1_wall_ornament_is_alcove_global_pc34(int globalIndex);
int dm1_v1_wall_ornament_view_spec_count_pc34(void);
int dm1_v1_wall_ornament_view_spec_pc34(
    int index,
    DM1_WallOrnamentViewSpecPc34* outSpec);
int dm1_v1_wall_ornament_render_plan_pc34(
    int globalIndex,
    int viewWallIndex,
    int maxHeight,
    DM1_WallOrnamentRenderPlanPc34* outPlan);
int dm1_v1_front_mirror_render_plan_pc34(
    int portraitOrdinal,
    DM1_FrontMirrorRenderPlanPc34* outPlan);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_WALL_ORNAMENT_PC34_COMPAT_H */
