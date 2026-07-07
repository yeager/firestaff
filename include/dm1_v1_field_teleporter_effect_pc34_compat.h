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
    M11_FT_EFFECT_NONE = 0,
    M11_FT_EFFECT_TELEPORT,     /* Flash + warp */
    M11_FT_EFFECT_PIT_FALL,     /* Falling down pit */
    M11_FT_EFFECT_STAIRS,       /* Stairs transition */
    M11_FT_EFFECT_SPELL_FLASH,  /* Spell impact flash */
    M11_FT_EFFECT_EXPLOSION     /* PROJEXPL.C explosion */
} M11_FT_EffectType;

typedef struct {
    int16_t x, y;               /* Screen position */
    int16_t dx, dy;             /* Velocity */
    uint8_t color;              /* Palette color index */
    uint8_t life;               /* Remaining frames */
    bool    active;
} M11_FT_Particle;

typedef struct {
    M11_FT_EffectType type;
    int               frame;
    int               total_frames;
    bool              active;

    /* Teleporter-specific: source → destination */
    int16_t src_x, src_y, src_level;
    int16_t dst_x, dst_y, dst_level;
    uint8_t flash_intensity;    /* 0-15 for palette flash */

    /* Particle system */
    M11_FT_Particle particles[DM1_FT_MAX_PARTICLES];
    uint8_t particle_count;
} M11_FT_EffectState;

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

void m11_ft_init(M11_FT_EffectState* state);
void m11_ft_start_teleport(M11_FT_EffectState* state,
                            int16_t sx, int16_t sy, int16_t sl,
                            int16_t dx, int16_t dy, int16_t dl);
void m11_ft_start_pit_fall(M11_FT_EffectState* state,
                            int16_t x, int16_t y, int16_t level);
void m11_ft_start_explosion(M11_FT_EffectState* state,
                             int16_t screen_x, int16_t screen_y,
                             uint8_t radius);
bool m11_ft_tick(M11_FT_EffectState* state);
bool m11_ft_is_active(const M11_FT_EffectState* state);
uint8_t m11_ft_get_flash_intensity(const M11_FT_EffectState* state);
uint8_t m11_ft_get_particle_count(const M11_FT_EffectState* state);

int dm1_v1_field_render_plan_count_pc34(void);

int dm1_v1_field_render_plan_at_pc34(
    int planIndex,
    DM1_FieldRenderPlanPc34* outPlan);

int dm1_v1_field_render_plan_for_relative_pc34(
    int relForward,
    int relSide,
    DM1_FieldRenderPlanPc34* outPlan);

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

int dm1_v1_field_square_is_visible_open_pc34(int square);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_FIELD_TELEPORTER_EFFECT_PC34_COMPAT_H */
