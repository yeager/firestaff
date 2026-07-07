#ifndef FIRESTAFF_CSB_V1_TEXT_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_TEXT_MATERIAL_PC34_COMPAT_H

typedef struct CSB_V1_TextMaterial_PC34 {
    int scale_x;
    int scale_y;
    int color;
    int shadow_dx;
    int shadow_dy;
    int shadow_color;
} CSB_V1_TextMaterial_PC34;

enum {
    CSB_V1_TEXT_STYLE_SMALL_PC34 = 1,
    CSB_V1_TEXT_STYLE_TITLE_PC34 = 2,
    CSB_V1_TEXT_STYLE_SHADOW_PC34 = 3
};

static inline CSB_V1_TextMaterial_PC34
csb_v1_text_material_pc34(int style)
{
    CSB_V1_TextMaterial_PC34 material;
    material.scale_x = 1;
    material.scale_y = 1;
    material.color = 15;
    material.shadow_dx = 0;
    material.shadow_dy = 0;
    material.shadow_color = 0;
    if (style == CSB_V1_TEXT_STYLE_TITLE_PC34) {
        material.scale_x = 2;
        material.color = 11;
        material.shadow_dx = 1;
        material.shadow_dy = 1;
    } else if (style == CSB_V1_TEXT_STYLE_SHADOW_PC34) {
        material.shadow_dx = 1;
        material.shadow_dy = 1;
    }
    return material;
}

#endif /* FIRESTAFF_CSB_V1_TEXT_MATERIAL_PC34_COMPAT_H */
