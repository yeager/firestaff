#ifndef FIRESTAFF_DM1_V1_ACTION_DAMAGE_RENDER_PLAN_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_DAMAGE_RENDER_PLAN_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB ACTIDRAW.C F0385_MENUS_DrawActionDamage, PC 3.4 EN. */
enum {
    DM1_V1_ACTION_DAMAGE_CANT_REACH_PC34 = -1,
    DM1_V1_ACTION_DAMAGE_MAX_PC34 = 32767,
    DM1_V1_ACTION_DAMAGE_TEXT_X_PC34 = 274,
    DM1_V1_ACTION_DAMAGE_TEXT_Y_PC34 = 100,
    DM1_V1_ACTION_DAMAGE_TEXT_DIGIT_X_STEP_PC34 = 3,
    DM1_V1_ACTION_DAMAGE_SMALL_MAX_PC34 = 15,
    DM1_V1_ACTION_DAMAGE_MEDIUM_MAX_PC34 = 40,
    DM1_V1_ACTION_DAMAGE_GRAPHIC_PC34 = 14,
    DM1_V1_ACTION_DAMAGE_DERIVED_MEDIUM_PC34 = 2,
    DM1_V1_ACTION_DAMAGE_DERIVED_SMALL_PC34 = 3,
    DM1_V1_ACTION_DAMAGE_COLOR_CYAN_PC34 = 4,
    DM1_V1_ACTION_DAMAGE_COLOR_BLACK_PC34 = 0,
    DM1_V1_ACTION_DAMAGE_MAX_STEPS_PC34 = 5
};

typedef enum DM1_V1_ActionDamageRenderKindPc34Compat {
    DM1_V1_ACTION_DAMAGE_RENDER_CANT_REACH_PC34 = 0,
    DM1_V1_ACTION_DAMAGE_RENDER_NEED_AMMO_PC34 = 1,
    DM1_V1_ACTION_DAMAGE_RENDER_SMALL_PC34 = 2,
    DM1_V1_ACTION_DAMAGE_RENDER_MEDIUM_PC34 = 3,
    DM1_V1_ACTION_DAMAGE_RENDER_FULL_PC34 = 4
} DM1_V1_ActionDamageRenderKindPc34Compat;

typedef enum DM1_V1_ActionDamageRenderStepKindPc34Compat {
    DM1_V1_ACTION_DAMAGE_STEP_ENABLE_SCREEN_UPDATE_PC34 = 0,
    DM1_V1_ACTION_DAMAGE_STEP_CLEAR_ACTION_AREA_PC34 = 1,
    DM1_V1_ACTION_DAMAGE_STEP_BLIT_DAMAGE_GRAPHIC_PC34 = 2,
    DM1_V1_ACTION_DAMAGE_STEP_PRINT_TEXT_PC34 = 3,
    DM1_V1_ACTION_DAMAGE_STEP_DISABLE_SCREEN_UPDATE_PC34 = 4
} DM1_V1_ActionDamageRenderStepKindPc34Compat;

typedef struct DM1_V1_ActionDamageBoxPc34Compat {
    int left;
    int right;
    int top;
    int bottom;
} DM1_V1_ActionDamageBoxPc34Compat;

typedef struct DM1_V1_ActionDamageRenderStepPc34Compat {
    DM1_V1_ActionDamageRenderStepKindPc34Compat kind;
} DM1_V1_ActionDamageRenderStepPc34Compat;

typedef struct DM1_V1_ActionDamageRenderPlanPc34Compat {
    int valid;
    int damage;
    DM1_V1_ActionDamageRenderKindPc34Compat render_kind;
    DM1_V1_ActionDamageBoxPc34Compat clear_box;
    DM1_V1_ActionDamageBoxPc34Compat blit_box;
    int has_damage_graphic;
    int graphic_index;
    int derived_bitmap_index;
    int bitmap_width;
    int bitmap_height;
    int bitmap_byte_width;
    int opaque_blit;
    int text_x;
    int text_y;
    int text_foreground_color;
    int text_background_color;
    char text[16];
    DM1_V1_ActionDamageRenderStepPc34Compat
        steps[DM1_V1_ACTION_DAMAGE_MAX_STEPS_PC34];
    int step_count;
} DM1_V1_ActionDamageRenderPlanPc34Compat;

typedef int (*DM1_V1_ActionDamageRenderConsumerPc34Compat)(
    void *context,
    const DM1_V1_ActionDamageRenderPlanPc34Compat *plan,
    const DM1_V1_ActionDamageRenderStepPc34Compat *step);

const char *DM1_V1_ActionDamageRenderPlan_SourceEvidencePc34Compat(void);

int F0385_MENUS_DrawActionDamage(
    int damage,
    DM1_V1_ActionDamageRenderConsumerPc34Compat consumer,
    void *context,
    DM1_V1_ActionDamageRenderPlanPc34Compat *out_plan);

int DM1_V1_ActionDamageRenderPlan_BuildPc34Compat(
    int damage,
    DM1_V1_ActionDamageRenderPlanPc34Compat *out_plan);

int DM1_V1_ActionDamageRenderPlan_ConsumePc34Compat(
    const DM1_V1_ActionDamageRenderPlanPc34Compat *plan,
    DM1_V1_ActionDamageRenderConsumerPc34Compat consumer,
    void *context);

#ifdef __cplusplus
}
#endif

#endif
