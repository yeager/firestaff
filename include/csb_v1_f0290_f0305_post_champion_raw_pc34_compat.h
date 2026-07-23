#ifndef FIRESTAFF_CSB_V1_F0290_F0305_POST_CHAMPION_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0290_F0305_POST_CHAMPION_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum CSB_V1_PostChampionFunctionPc34 {
    CSB_V1_POST_CHAMPION_F0290 = 290,
    CSB_V1_POST_CHAMPION_F0291 = 291,
    CSB_V1_POST_CHAMPION_F0292 = 292,
    CSB_V1_POST_CHAMPION_F0293 = 293,
    CSB_V1_POST_CHAMPION_F0294 = 294,
    CSB_V1_POST_CHAMPION_F0296 = 296,
    CSB_V1_POST_CHAMPION_F0297 = 297,
    CSB_V1_POST_CHAMPION_F0298 = 298,
    CSB_V1_POST_CHAMPION_F0299 = 299,
    CSB_V1_POST_CHAMPION_F0300 = 300,
    CSB_V1_POST_CHAMPION_F0301 = 301,
    CSB_V1_POST_CHAMPION_F0302 = 302,
    CSB_V1_POST_CHAMPION_F0303 = 303,
    CSB_V1_POST_CHAMPION_F0304 = 304,
    CSB_V1_POST_CHAMPION_F0305 = 305
} CSB_V1_PostChampionFunctionPc34;

typedef struct CSB_V1_PostChampionRawMaterialPc34 {
    const uint8_t *champion_record;
    size_t champion_record_size;
    uint32_t champion_record_identity;
    const uint8_t *graphics_material;
    size_t graphics_material_size;
    uint32_t graphics_material_identity;
    const uint8_t *dungeon_material;
    size_t dungeon_material_size;
    uint32_t dungeon_material_identity;
    int authenticated_pc34;
} CSB_V1_PostChampionRawMaterialPc34;

typedef struct CSB_V1_PostChampionAuditReceiptPc34 {
    int raw_material_admitted;
    int graphics_material_required;
    int dungeon_material_required;
    int read_only_query;
    int runtime_execution_blocked;
    int platform_behavior_fail_closed;
    CSB_V1_PostChampionFunctionPc34 function_id;
    uint32_t champion_record_identity;
    const char *source_evidence;
} CSB_V1_PostChampionAuditReceiptPc34;

/* This is a source audit, not a dispatcher. A successful return means that
 * source-shaped raw material was authenticated and execution was blocked. */
int csb_v1_f0290_f0305_post_champion_audit_pc34(
    const CSB_V1_PostChampionRawMaterialPc34 *raw,
    CSB_V1_PostChampionFunctionPc34 function_id,
    CSB_V1_PostChampionAuditReceiptPc34 *out);

#endif
