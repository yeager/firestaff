#ifndef FIRESTAFF_DM1_V1_F0440_F0441_ENTRANCE_ASSET_FLOW_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0440_F0441_ENTRANCE_ASSET_FLOW_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0440_GRAPHIC_ENTRANCE_PC34 = 4,
    DM1_V1_F0440_GRAPHIC_CREDITS_PC34 = 5,
    DM1_V1_F0440_GRAPHIC_SOUND_DOOR_RATTLE_PC34 = 534,
    DM1_V1_F0440_GRAPHIC_SOUND_SWITCH_PC34 = 535,
    DM1_V1_F0441_GRAPHIC_LEFT_DOOR_PC34 = 2,
    DM1_V1_F0441_GRAPHIC_RIGHT_DOOR_PC34 = 3,
    DM1_V1_F0441_DOOR_FRAME_BANK_COUNT_PC34 = 8,
    DM1_V1_F0441_COMPOSITE_SURFACE_COUNT_PC34 = 2
};

typedef struct DM1_V1_F0440TemporaryGraphicRequestPc34 {
    int graphic_index;
    const uint8_t *decompressed_bytes;
    size_t decompressed_byte_count;
    uint32_t graphics_dat_record_fingerprint;
    int original_graphics_dat_member;
    int raw_record_verified;
    int not_expanded_route;
    int temporary_heap_target_bound;
    int no_synthetic_bytes;
    int no_host_wrapper;
} DM1_V1_F0440TemporaryGraphicRequestPc34;

typedef struct DM1_V1_F0440TemporaryGraphicReceiptPc34 {
    int accepted;
    int graphic_index;
    size_t decompressed_byte_count;
    uint32_t graphics_dat_record_fingerprint;
    int temporary_heap_target_bound;
    int not_expanded_route;
    int suppress_synthetic_fallback;
    const char *source_evidence;
} DM1_V1_F0440TemporaryGraphicReceiptPc34;

typedef struct DM1_V1_F0441EntranceFlowRequestPc34 {
    const DM1_V1_F0440TemporaryGraphicReceiptPc34 *c004_entrance;
    const DM1_V1_F0440TemporaryGraphicReceiptPc34 *c005_credits;
    const DM1_V1_F0440TemporaryGraphicRequestPc34 *c002_left_door;
    const DM1_V1_F0440TemporaryGraphicRequestPc34 *c003_right_door;
    unsigned int door_frame_bank_count;
    unsigned int composite_surface_count;
    int graphics_dat_closed_after_source_load;
    int no_synthetic_pages;
    int no_host_lifecycle;
} DM1_V1_F0441EntranceFlowRequestPc34;

typedef struct DM1_V1_F0441EntranceFlowReceiptPc34 {
    int accepted;
    int c004_entrance_receipt_consumed;
    int c005_credits_receipt_consumed;
    int c002_left_door_bound;
    int c003_right_door_bound;
    unsigned int door_frame_bank_count;
    unsigned int composite_surface_count;
    int graphics_dat_closed_after_source_load;
    int suppress_synthetic_fallback;
    const char *source_evidence;
} DM1_V1_F0441EntranceFlowReceiptPc34;

int dm1_v1_f0440_temporary_graphic_byte_count_pc34(
    const DM1_V1_F0440TemporaryGraphicRequestPc34 *request,
    DM1_V1_F0440TemporaryGraphicReceiptPc34 *out_receipt);

int dm1_v1_f0441_entrance_asset_flow_admission_pc34(
    const DM1_V1_F0441EntranceFlowRequestPc34 *request,
    DM1_V1_F0441EntranceFlowReceiptPc34 *out_receipt);

const char *dm1_v1_f0440_f0441_entrance_asset_flow_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_F0440_F0441_ENTRANCE_ASSET_FLOW_PC34_COMPAT_H */
