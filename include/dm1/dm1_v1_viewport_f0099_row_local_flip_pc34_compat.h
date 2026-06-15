#ifndef FIRESTAFF_DM1_V1_VIEWPORT_F0099_ROW_LOCAL_FLIP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_F0099_ROW_LOCAL_FLIP_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_F1000_DIRECT_PC34 = 0,
    DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_F1000_VIEWPORT_SCRATCH_PC34 = 1,
    DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_DOOR_FRAME_D1C_PC34 = 2,
    DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D3L2_TO_D3R2_WALL_PC34 = 3,
    DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D3C_WALL_PC34 = 4,
    DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D2C_WALL_PC34 = 5,
    DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D1LCR_WALL_PC34 = 6,
    DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D0L_TO_D0R_WALL_PC34 = 7,
    DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_D0R_TO_D0L_WALL_PC34 = 8,
    DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_G4052_FLOOR_PC34 = 9,
    DM1_V1_F0099_ROW_LOCAL_FLIP_ROLE_G4053_CEILING_PC34 = 10
} DM1_V1_F0099RowLocalFlipRolePc34;

typedef struct {
    DM1_V1_F0099RowLocalFlipRolePc34 role;
    const char *role_name;
    const char *source_symbol;
    const char *destination_symbol;
    const char *copy_anchor;
    const char *caller_anchor;
    int byte_width;
    int height;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_F0099RowLocalFlipEvidencePc34;

typedef struct {
    const uint8_t *source;
    size_t source_len;
    uint8_t *destination;
    size_t destination_len;
    size_t row_width;
    size_t height;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_F0099RowLocalFlipStatePc34;

typedef struct {
    bool ok;
    bool contract_only;
    bool real_asset_claim;
    bool in_place;
    size_t row_width;
    size_t height;
    size_t byte_count;
    size_t rows_flipped;
    uint8_t first_source_byte;
    uint8_t first_destination_byte;
    uint8_t last_source_byte;
    uint8_t last_destination_byte;
    const char *source_evidence;
} DM1_V1_F0099RowLocalFlipResultPc34;

const DM1_V1_F0099RowLocalFlipEvidencePc34 *
dm1_v1_viewport_f0099_row_local_flip_evidence_pc34(size_t *count);

const DM1_V1_F0099RowLocalFlipEvidencePc34 *
dm1_v1_viewport_f0099_row_local_flip_evidence_for_role_pc34(
    DM1_V1_F0099RowLocalFlipRolePc34 role);

bool dm1_v1_viewport_f0099_row_local_flip_pc34_compat(
    const DM1_V1_F0099RowLocalFlipStatePc34 *state,
    DM1_V1_F0099RowLocalFlipResultPc34 *out);

const char *dm1_v1_viewport_f0099_row_local_flip_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_F0099_ROW_LOCAL_FLIP_PC34_COMPAT_H */
