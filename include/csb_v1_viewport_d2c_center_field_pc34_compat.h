#ifndef FIRESTAFF_CSB_V1_VIEWPORT_D2C_CENTER_FIELD_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_D2C_CENTER_FIELD_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_D2C_CENTER_FIELD_PC34_C10_COLOR_FLESH 10
#define CSB_V1_D2C_CENTER_FIELD_PC34_VIEWPORT_WIDTH 224
#define CSB_V1_D2C_CENTER_FIELD_PC34_VIEWPORT_HEIGHT 136

typedef enum {
    CSB_V1_D2C_CENTER_FIELD_PC34_VIEW_SQUARE_D2C = 6
} CSB_V1_D2CCenterFieldViewSquarePc34;

typedef struct {
    int x1;
    int x2;
    int y1;
    int y2;
    int byte_width;
    int height;
    int blit_x;
    int blit_y;
} CSB_V1_D2CCenterFieldFramePc34;

typedef struct {
    CSB_V1_D2CCenterFieldViewSquarePc34 view_square;
    int view_square_macro_value;
    int redmcsb_view_square_index;
    int view_depth;
    int view_lane;
    int field_aspect_index;
    const char *draw_square_function;
    const char *dispatch_source_lines;
    const char *center_route_source_lines;
    const char *field_source_lines;
    const char *lineage_source_lines;
    CSB_V1_D2CCenterFieldFramePc34 wall_frame;
    int wall_ordinal;
    int media508_field_zone;
    int media720_field_zone;
    int media720_base_zone_c702;
    int media720_next_zone_c703;
    int media720_field_zone_from_c702;
    int transparent_color;
    uint16_t cell_order;
    bool calls_d2c_draw_square;
    bool calls_f0100_wall_bitmap;
    bool calls_f0105_scratch_flip_for_wall;
    bool calls_f0107_wall_ornament;
    bool calls_f0111_door;
    bool calls_f0113_field;
    bool only_field_surface_call_is_f0113;
    bool standard_f0115_precedes_field;
    bool ordinary_floor_ceiling_prework;
    bool field_blit_preserves_c10_transparency;
    bool wall_case_returns_before_field;
    bool has_c_wall_ordinal;
    bool contract_only;
} CSB_V1_D2CCenterFieldSpecPc34;

const CSB_V1_D2CCenterFieldSpecPc34 *
M11_GameView_ViewportD2CCenterFieldPc34Spec(void);

int M11_GameView_ViewportD2CCenterFieldPc34ZoneFromC702Base(
    const CSB_V1_D2CCenterFieldSpecPc34 *spec);

int M11_GameView_ViewportD2CCenterFieldPc34ApplySyntheticC10FieldBlit(
    const CSB_V1_D2CCenterFieldSpecPc34 *spec,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height);

const char *M11_GameView_ViewportD2CCenterFieldPc34SourceEvidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_VIEWPORT_D2C_CENTER_FIELD_PC34_COMPAT_H */
