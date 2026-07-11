/* DM1 V1 Field/Teleporter Visual Effects — source-locked from ReDMCSB
 * MOVESENS.C: teleporter trigger logic, F0263_MOVE_ProcessSquareFirstTime
 * PROJEXPL.C: explosion/particle visual effects
 * TIMELINE.C: event scheduling for timed visual effects
 * Note: DM1 has no overworld/field map — teleporters are in-dungeon */
#ifndef FIRESTAFF_DM1_V1_FIELD_TELEPORTER_EFFECT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_FIELD_TELEPORTER_EFFECT_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_FT_MAX_PARTICLES    32
#define DM1_FT_TELEPORT_FRAMES  12   /* Animation frames for teleport effect */
#define DM1_FT_PIT_FALL_FRAMES   8   /* Pit fall animation frames */

typedef enum {
    DM1_V1_FT_EFFECT_NONE = 0,
    DM1_V1_FT_EFFECT_TELEPORT,     /* Flash + warp */
    DM1_V1_FT_EFFECT_PIT_FALL,     /* Falling down pit */
    DM1_V1_FT_EFFECT_STAIRS,       /* Stairs transition */
    DM1_V1_FT_EFFECT_SPELL_FLASH,  /* Spell impact flash */
    DM1_V1_FT_EFFECT_EXPLOSION     /* PROJEXPL.C explosion */
} DM1_V1_FieldTeleporterEffectTypePc34;

typedef struct {
    int16_t x, y;               /* Screen position */
    int16_t dx, dy;             /* Velocity */
    uint8_t color;              /* Palette color index */
    uint8_t life;               /* Remaining frames */
    bool    active;
} DM1_V1_FieldTeleporterParticlePc34;

typedef struct {
    DM1_V1_FieldTeleporterEffectTypePc34 type;
    int               frame;
    int               total_frames;
    bool              active;

    /* Teleporter-specific: source → destination */
    int16_t src_x, src_y, src_level;
    int16_t dst_x, dst_y, dst_level;
    uint8_t flash_intensity;    /* 0-15 for palette flash */

    /* Particle system */
    DM1_V1_FieldTeleporterParticlePc34 particles[DM1_FT_MAX_PARTICLES];
    uint8_t particle_count;
} DM1_V1_FieldTeleporterEffectStatePc34;

typedef struct DM1_FieldRenderPlanPc34 {
    int relForward;
    int relSide;
    int dstX;
    int dstY;
    int dstW;
    int dstH;
    int baseStartUnit;
    int transparentColor;
    int maskIndexAndFlip;
} DM1_FieldRenderPlanPc34;

typedef struct DM1_FieldBitmapSamplePc34 {
    int fieldX;
    int fieldY;
    int maskPresent;
    int maskX;
    int maskY;
    int maskFlip;
    int transparentColor;
} DM1_FieldBitmapSamplePc34;

typedef struct DM1_FieldAssetBindingPc34 {
    int fieldGraphicIndex;
    int maskGraphicIndex;
    int maskRequired;
    int maskIndex;
    int maskFlip;
    int transparentColor;
} DM1_FieldAssetBindingPc34;

void DM1_V1_FieldTeleporter_InitPc34Compat(DM1_V1_FieldTeleporterEffectStatePc34* state);
void DM1_V1_FieldTeleporter_StartTeleportPc34Compat(DM1_V1_FieldTeleporterEffectStatePc34* state,
                            int16_t sx, int16_t sy, int16_t sl,
                            int16_t dx, int16_t dy, int16_t dl);
void DM1_V1_FieldTeleporter_StartPitFallPc34Compat(DM1_V1_FieldTeleporterEffectStatePc34* state,
                            int16_t x, int16_t y, int16_t level);
void DM1_V1_FieldTeleporter_StartExplosionPc34Compat(DM1_V1_FieldTeleporterEffectStatePc34* state,
                             int16_t screen_x, int16_t screen_y,
                             uint8_t radius);
bool DM1_V1_FieldTeleporter_TickPc34Compat(DM1_V1_FieldTeleporterEffectStatePc34* state);
bool DM1_V1_FieldTeleporter_IsActivePc34Compat(const DM1_V1_FieldTeleporterEffectStatePc34* state);
uint8_t DM1_V1_FieldTeleporter_GetFlashIntensityPc34Compat(const DM1_V1_FieldTeleporterEffectStatePc34* state);
uint8_t DM1_V1_FieldTeleporter_GetParticleCountPc34Compat(const DM1_V1_FieldTeleporterEffectStatePc34* state);

int dm1_v1_field_render_plan_count_pc34(void);

int dm1_v1_field_render_plan_at_pc34(
    int planIndex,
    DM1_FieldRenderPlanPc34* outPlan);

int dm1_v1_field_render_plan_for_relative_pc34(
    int relForward,
    int relSide,
    DM1_FieldRenderPlanPc34* outPlan);

int dm1_v1_field_graphic_index_pc34(void);

int dm1_v1_field_mask_graphic_index_pc34(int maskIndex);

int dm1_v1_field_asset_binding_pc34(
    const DM1_FieldRenderPlanPc34* plan,
    DM1_FieldAssetBindingPc34* outBinding);

int dm1_v1_field_bitmap_sample_pc34(
    const DM1_FieldRenderPlanPc34* plan,
    uint32_t animTick,
    int localX,
    int localY,
    int fieldWidth,
    int fieldHeight,
    int maskWidth,
    int maskHeight,
    DM1_FieldBitmapSamplePc34* outSample);

int dm1_v1_field_bitmap_pixel_pc34(
    const DM1_FieldRenderPlanPc34* plan,
    uint32_t animTick,
    int localX,
    int localY,
    const uint8_t* fieldPixels,
    int fieldWidth,
    int fieldHeight,
    int fieldStride,
    const uint8_t* maskPixels,
    int maskWidth,
    int maskHeight,
    int maskStride,
    uint8_t* outPixel);

int dm1_v1_field_asset_indices_pc34(
    const DM1_FieldRenderPlanPc34* plan,
    int* outFieldGraphicIndex,
    int* outMaskGraphicIndex);

int dm1_v1_field_square_is_visible_open_pc34(int square);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_FIELD_TELEPORTER_EFFECT_PC34_COMPAT_H */
