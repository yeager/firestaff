#ifndef FIRESTAFF_DM2_V1_CREATURE_TABLES_H
#define FIRESTAFF_DM2_V1_CREATURE_TABLES_H

#include <stdint.h>
#include "dm2_v1_creature.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const int8_t dm2_v1_creature_params[132];
extern const int8_t dm2_v1_creature_dir_rotate[4][5];
extern const int8_t dm2_v1_creature_step_dx[2][4];
extern const int8_t dm2_v1_creature_step_dy[2][4];
extern const int8_t dm2_v1_creature_attack_dir[4];
extern const int8_t dm2_v1_creature_facing[4];
extern const int8_t dm2_v1_creature_projectile_offset[4];

extern const int8_t dm2_v1_creature_render_desc[92];

extern const int16_t dm2_v1_creature_gfx_face[6];
extern const int8_t dm2_v1_creature_gfx_orn[6];
extern const int8_t dm2_v1_creature_gfx_flip[6];
extern const int8_t dm2_v1_creature_gfx_side[6];

extern const int8_t dm2_v1_inventory_remap[25];
extern const int8_t dm2_v1_inventory_remap_mirror[25];
extern const int8_t dm2_v1_inventory_remap_alt[25];

extern const int8_t dm2_v1_tile_type_visibility[23];

extern const char dm2_v1_text_charset[38];
extern const char *dm2_v1_skill_abbrev[18];

#define DM2_AI_TABLE_GENUINE_SIZE 62
extern const DM2_AIDefinition dm2_v1_ai_table_genuine[DM2_AI_TABLE_GENUINE_SIZE];

const char *dm2_v1_creature_tables_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
