#ifndef DM1_V1_ORIGINAL_SAVE_CLASSIFIER_H
#define DM1_V1_ORIGINAL_SAVE_CLASSIFIER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 original-save classifier.
 *
 * This is a bounded readiness gate for original DMSAVE.DAT / DMSAVE.BAK
 * shaped bytes.  It validates only the ReDMCSB save-header envelope:
 * 512-byte DM_SAVE_HEADER, first-half checksum, second-half XOR
 * obfuscation, and the stable metadata fields needed before any importer
 * can be trusted.
 *
 * It deliberately does not load the save into GameWorld_Compat and does
 * not claim compatibility.  Full compatibility still requires real original
 * bytes to round-trip through the Firestaff runtime and writer.
 *
 * Source-lock:
 *   ReDMCSB DEFS.H:468-480   DM_SAVE_HEADER layout
 *   ReDMCSB DEFS.H:500-508   header key index + FormatID constants
 *   ReDMCSB SAVEHEAD.C:30-54 F0429 read/check/deobfuscate
 *   ReDMCSB SAVEHEAD.C:76-104 F0430 write/check/obfuscate
 *   ReDMCSB READWRIT.C:191-209 F0417 XOR/checksum pass
 *   ReDMCSB LOADSAVE.C:1590-1628 F0433 save header + parts
 *   ReDMCSB LOADSAVE.C:2665-2722 F0435 load header + global data
 */

#define DM1_ORIGINAL_SAVE_HEADER_BYTES 512u
#define DM1_ORIGINAL_SAVE_MIN_BYTES    512u
#define DM1_ORIGINAL_SAVE_PATH_MAX     512u
#define DM1_ORIGINAL_SAVE_DEFAULT_CANDIDATE_COUNT 4u

typedef enum {
    DM1_ORIGINAL_SAVE_SHAPE_ABSENT = 0,
    DM1_ORIGINAL_SAVE_SHAPE_FIRESTAFF_NATIVE = 1,
    DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1 = 2,
    DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_COMPAT_FAMILY = 3,
    DM1_ORIGINAL_SAVE_SHAPE_REJECTED = 4,
    DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34 = 5
} DM1OriginalSaveShape;

typedef enum {
    DM1_ORIGINAL_SAVE_READY_ABSENT = 0,
    DM1_ORIGINAL_SAVE_READY_CLASSIFIED_HEADER_ONLY = 1,
    DM1_ORIGINAL_SAVE_READY_NOT_ORIGINAL = 2,
    DM1_ORIGINAL_SAVE_READY_REJECTED = 3
} DM1OriginalSaveReadiness;

typedef struct {
    DM1OriginalSaveShape shape;
    DM1OriginalSaveReadiness readiness;
    uint64_t size_bytes;
    uint16_t header_key;
    uint16_t format_id;
    uint16_t useless;
    uint32_t game_id;
    uint8_t save_and_play;
    uint16_t platform;
    uint16_t dungeon_id;
    uint16_t save_part_key_count_nonzero;
    uint16_t save_part_checksum_count_nonzero;
    uint16_t header_expected_checksum;
    uint16_t header_actual_checksum;
    uint32_t prefix_checksum32;
    int header_checksum_ok;
    int import_blocked_until_roundtrip;
    int pc34_importer_candidate;
    char reason[96];
} DM1OriginalSaveClassifyResult;

typedef struct {
    char root[DM1_ORIGINAL_SAVE_PATH_MAX];
    int candidate_count;
    int present_count;
    int classified_count;
    int original_dm1_count;
    int original_dm1_pc34_count;
    int pc34_importer_candidate_count;
    int firestaff_native_count;
    DM1OriginalSaveClassifyResult results[DM1_ORIGINAL_SAVE_DEFAULT_CANDIDATE_COUNT];
    char paths[DM1_ORIGINAL_SAVE_DEFAULT_CANDIDATE_COUNT][DM1_ORIGINAL_SAVE_PATH_MAX];
} DM1OriginalSaveManifest;

int dm1_v1_original_save_default_root(char out_root[DM1_ORIGINAL_SAVE_PATH_MAX]);
int dm1_v1_original_save_candidate_path(
    const char *root,
    int candidate_index,
    char out_path[DM1_ORIGINAL_SAVE_PATH_MAX]);

int dm1_v1_original_save_classify_bytes(
    const uint8_t *bytes,
    size_t size,
    DM1OriginalSaveClassifyResult *out_result);

int dm1_v1_original_save_classify_file(
    const char *path,
    DM1OriginalSaveClassifyResult *out_result);

int dm1_v1_original_save_classify_root(
    const char *root,
    DM1OriginalSaveManifest *out_manifest);

const char *dm1_v1_original_save_shape_name(DM1OriginalSaveShape shape);
const char *dm1_v1_original_save_readiness_name(DM1OriginalSaveReadiness readiness);
const char *dm1_v1_original_save_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_ORIGINAL_SAVE_CLASSIFIER_H */
