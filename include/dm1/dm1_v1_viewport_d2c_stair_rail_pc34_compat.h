#ifndef FIRESTAFF_DM1_V1_VIEWPORT_D2C_STAIR_RAIL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_D2C_STAIR_RAIL_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V1_D2C_STAIR_RAIL_ROLE_UP_FRONT_PC34 = 0,
    DM1_V1_D2C_STAIR_RAIL_ROLE_DOWN_FRONT_PC34 = 1
} DM1_V1_D2CStairRailRolePc34;

typedef struct {
    DM1_V1_D2CStairRailRolePc34 role;
    const char *role_name;
    const char *dispatch_anchor;
    const char *draw_anchor;
    const char *defs_anchor;
    int stairs_bitmap_slot;
    int zone_index;
    int view_square_index;
    int transparent_color;
    size_t synthetic_width;
    size_t synthetic_height;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D2CStairRailEvidencePc34;

typedef struct {
    int wallset_index;
    DM1_V1_D2CStairRailRolePc34 role;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D2CStairRailResolveInputPc34;

typedef struct {
    bool ok;
    bool contract_only;
    bool real_asset_claim;
    int first_stairs_graphic_index;
    int stairs_bitmap_slot;
    int native_bitmap_index;
    int zone_index;
    int view_square_index;
    const DM1_V1_D2CStairRailEvidencePc34 *evidence;
} DM1_V1_D2CStairRailResolveResultPc34;

typedef struct {
    DM1_V1_D2CStairRailRolePc34 role;
    const uint8_t *source;
    size_t source_len;
    uint8_t *destination;
    size_t destination_len;
    size_t row_width;
    size_t height;
    size_t destination_stride;
    bool contract_only;
    bool real_asset_claim;
} DM1_V1_D2CStairRailBlitInputPc34;

typedef struct {
    bool ok;
    bool contract_only;
    bool real_asset_claim;
    bool wrote_any;
    bool transparent_skip_seen;
    size_t row_width;
    size_t height;
    size_t byte_count;
    size_t destination_stride;
    size_t writes;
    size_t transparent_skips;
    uint8_t first_source_byte;
    uint8_t last_source_byte;
    uint8_t first_destination_byte;
    uint8_t last_destination_byte;
    const DM1_V1_D2CStairRailEvidencePc34 *evidence;
} DM1_V1_D2CStairRailBlitResultPc34;

const DM1_V1_D2CStairRailEvidencePc34 *
dm1_v1_viewport_d2c_stair_rail_evidence_pc34(size_t *count);

const DM1_V1_D2CStairRailEvidencePc34 *
dm1_v1_viewport_d2c_stair_rail_evidence_for_role_pc34(
    DM1_V1_D2CStairRailRolePc34 role);

bool dm1_v1_viewport_d2c_stair_rail_resolve_pc34(
    const DM1_V1_D2CStairRailResolveInputPc34 *input,
    DM1_V1_D2CStairRailResolveResultPc34 *out);

bool dm1_v1_viewport_d2c_stair_rail_blit_pc34(
    const DM1_V1_D2CStairRailBlitInputPc34 *input,
    DM1_V1_D2CStairRailBlitResultPc34 *out);

const char *dm1_v1_viewport_d2c_stair_rail_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_D2C_STAIR_RAIL_PC34_COMPAT_H */
