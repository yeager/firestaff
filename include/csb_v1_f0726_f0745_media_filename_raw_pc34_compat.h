#ifndef FIRESTAFF_CSB_V1_F0726_F0745_MEDIA_FILENAME_RAW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0726_F0745_MEDIA_FILENAME_RAW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

typedef enum CSB_V1_MediaFilenameFunctionPc34 {
    CSB_V1_MEDIA_FILENAME_F0726 = 726, CSB_V1_MEDIA_FILENAME_F0727,
    CSB_V1_MEDIA_FILENAME_F0728, CSB_V1_MEDIA_FILENAME_F0729,
    CSB_V1_MEDIA_FILENAME_F0730, CSB_V1_MEDIA_FILENAME_F0731,
    CSB_V1_MEDIA_FILENAME_F0732, CSB_V1_MEDIA_FILENAME_F0733,
    CSB_V1_MEDIA_FILENAME_F0734, CSB_V1_MEDIA_FILENAME_F0735,
    CSB_V1_MEDIA_FILENAME_F0736, CSB_V1_MEDIA_FILENAME_F0737,
    CSB_V1_MEDIA_FILENAME_F0738, CSB_V1_MEDIA_FILENAME_F0739,
    CSB_V1_MEDIA_FILENAME_F0740, CSB_V1_MEDIA_FILENAME_F0741,
    CSB_V1_MEDIA_FILENAME_F0742, CSB_V1_MEDIA_FILENAME_F0743,
    CSB_V1_MEDIA_FILENAME_F0744, CSB_V1_MEDIA_FILENAME_F0745
} CSB_V1_MediaFilenameFunctionPc34;

typedef struct CSB_V1_MediaFilenameRawMaterialPc34 {
    const uint8_t *graphics; size_t graphics_size; uint32_t graphics_identity;
    const uint8_t *palette; size_t palette_size; uint32_t palette_identity;
    const uint8_t *zone; size_t zone_size; uint32_t zone_identity;
    const uint8_t *music; size_t music_size; uint32_t music_identity;
    const uint8_t *package; size_t package_size; uint32_t package_identity;
    const uint8_t *file_names; size_t file_names_size; uint32_t file_names_identity;
    int authenticated_pc34;
} CSB_V1_MediaFilenameRawMaterialPc34;

typedef struct CSB_V1_MediaFilenameAuditReceiptPc34 {
    int raw_material_admitted;
    int existing_runtime_owner_preserved;
    int graphics_required, palette_required, zone_required, music_required;
    int package_required, file_names_required;
    int read_only_query, runtime_execution_blocked, platform_behavior_fail_closed;
    CSB_V1_MediaFilenameFunctionPc34 function_id;
    const char *source_evidence;
} CSB_V1_MediaFilenameAuditReceiptPc34;

int csb_v1_f0726_f0745_media_filename_audit_pc34(
    const CSB_V1_MediaFilenameRawMaterialPc34 *raw,
    CSB_V1_MediaFilenameFunctionPc34 function_id,
    CSB_V1_MediaFilenameAuditReceiptPc34 *out);

#endif
