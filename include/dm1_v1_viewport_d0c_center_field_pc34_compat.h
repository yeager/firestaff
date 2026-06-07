#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D0C_CENTER_FIELD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D0C_CENTER_FIELD_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_D0C_CENTER_FIELD_PC34_C10_COLOR_FLESH 10
#define DM1_V1_D0C_CENTER_FIELD_PC34_VIEWPORT_WIDTH 224
#define DM1_V1_D0C_CENTER_FIELD_PC34_VIEWPORT_HEIGHT 136

typedef enum {
    DM1_V1_D0C_CENTER_FIELD_PC34_VIEW_SQUARE_D0C = 9
} DM1_V1_D0CCenterFieldViewSquarePc34;

typedef struct {
    int x1;
    int x2;
    int y1;
    int y2;
    int byte_width;
    int height;
    int blit_x;
    int blit_y;
} DM1_V1_D0CCenterFieldFramePc34;

typedef struct {
    DM1_V1_D0CCenterFieldViewSquarePc34 view_square;
    int view_square_macro_value;
    const char *draw_square_function;
    const char *dispatch_source_lines;
    const char *no_wall_source_lines;
    const char *field_source_lines;
    const char *thing_pass_contract_source_lines;
    DM1_V1_D0CCenterFieldFramePc34 wall_frame;
    int wall_ordinal;
    int media508_field_zone;
    int media720_field_zone;
    int transparent_color;
    uint16_t cell_order;
    bool calls_f0127;
    bool calls_f0100_wall_bitmap;
    bool calls_f0105_scratch_flip_for_wall;
    bool calls_f0107_wall_ornament;
    bool calls_f0113_field;
    bool calls_f0111_door;
    bool calls_extra_f0115_thing_pass_after_field;
    bool standard_f0115_precedes_field;
    bool field_blit_preserves_c10_transparency;
    bool wall_case_returns;
    bool has_c_wall_ordinal;
    bool contract_only;
} DM1_V1_D0CCenterFieldSpecPc34;

const DM1_V1_D0CCenterFieldSpecPc34 *
dm1_v1_viewport_d0c_center_field_pc34_compat_spec(void);

const char *dm1_v1_viewport_d0c_center_field_pc34_compat_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D0C_CENTER_FIELD_PC34_COMPAT_H */
