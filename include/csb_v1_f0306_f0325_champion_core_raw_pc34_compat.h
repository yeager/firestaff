#ifndef FIRESTAFF_CSB_V1_F0306_F0325_CHAMPION_CORE_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0306_F0325_CHAMPION_CORE_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum CSB_V1_ChampionCoreFunctionPc34 {
    CSB_V1_CHAMPION_CORE_F0306 = 306, CSB_V1_CHAMPION_CORE_F0307 = 307,
    CSB_V1_CHAMPION_CORE_F0308 = 308, CSB_V1_CHAMPION_CORE_F0309 = 309,
    CSB_V1_CHAMPION_CORE_F0310 = 310, CSB_V1_CHAMPION_CORE_F0311 = 311,
    CSB_V1_CHAMPION_CORE_F0312 = 312, CSB_V1_CHAMPION_CORE_F0313 = 313,
    CSB_V1_CHAMPION_CORE_F0314 = 314, CSB_V1_CHAMPION_CORE_F0315 = 315,
    CSB_V1_CHAMPION_CORE_F0316 = 316, CSB_V1_CHAMPION_CORE_F0317 = 317,
    CSB_V1_CHAMPION_CORE_F0318 = 318, CSB_V1_CHAMPION_CORE_F0319 = 319,
    CSB_V1_CHAMPION_CORE_F0320 = 320, CSB_V1_CHAMPION_CORE_F0321 = 321,
    CSB_V1_CHAMPION_CORE_F0322 = 322, CSB_V1_CHAMPION_CORE_F0323 = 323,
    CSB_V1_CHAMPION_CORE_F0324 = 324, CSB_V1_CHAMPION_CORE_F0325 = 325
} CSB_V1_ChampionCoreFunctionPc34;

typedef struct CSB_V1_ChampionCoreRawMaterialPc34 {
    const uint8_t *champion_record;
    size_t champion_record_size;
    uint32_t champion_record_identity;
    const uint8_t *graphics_material;
    size_t graphics_material_size;
    uint32_t graphics_material_identity;
    const uint8_t *dungeon_material;
    size_t dungeon_material_size;
    uint32_t dungeon_material_identity;
    const uint8_t *timeline_material;
    size_t timeline_material_size;
    uint32_t timeline_material_identity;
    int authenticated_pc34;
} CSB_V1_ChampionCoreRawMaterialPc34;

typedef struct CSB_V1_ChampionCoreAuditReceiptPc34 {
    int raw_material_admitted;
    int graphics_material_required;
    int dungeon_material_required;
    int timeline_material_required;
    int read_only_query;
    int runtime_execution_blocked;
    int platform_behavior_fail_closed;
    CSB_V1_ChampionCoreFunctionPc34 function_id;
    uint32_t champion_record_identity;
    const char *source_evidence;
} CSB_V1_ChampionCoreAuditReceiptPc34;

int csb_v1_f0306_f0325_champion_core_audit_pc34(
    const CSB_V1_ChampionCoreRawMaterialPc34 *raw,
    CSB_V1_ChampionCoreFunctionPc34 function_id,
    CSB_V1_ChampionCoreAuditReceiptPc34 *out);

#endif
