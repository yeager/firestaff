#ifndef FIRESTAFF_DM2_V1_OBJECT_TRANSFER_HELPERS_H
#define FIRESTAFF_DM2_V1_OBJECT_TRANSFER_HELPERS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_OBJECT_TRANSFER_NULL 0xffffu
#define DM2_V1_OBJECT_TRANSFER_MAX_LINKS 64u

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    int mutated;
    uint16_t object_ref;
    uint16_t container_ref;
    uint16_t previous_ref;
    uint16_t next_ref;
    uint16_t new_head_ref;
    uint16_t hand_ref;
    const char *symbol;
    const char *source_path;
} DM2_V1_ObjectTransferReceipt;

typedef struct {
    uint16_t ref;
    uint16_t next_ref;
} DM2_V1_ObjectTransferLink;

typedef struct {
    uint16_t projectile_ref;
    uint16_t hand_ref;
    uint16_t source_head_ref;
    uint16_t source_next_ref;
} DM2_V1_LoadProjectileToHandInput;

void dm2_v1_object_transfer_receipt_clear(
    DM2_V1_ObjectTransferReceipt *receipt);

int dm2_v1_REMOVE_POSSESSION(
    const DM2_V1_ObjectTransferLink *links,
    size_t link_count,
    uint16_t head_ref,
    uint16_t object_ref,
    uint16_t *out_new_head_ref,
    uint16_t *out_removed_next_ref,
    DM2_V1_ObjectTransferReceipt *out_receipt);

int dm2_v1_PUT_OBJECT_INTO_CONTAINER(
    const DM2_V1_ObjectTransferLink *links,
    size_t link_count,
    uint16_t container_ref,
    uint16_t container_head_ref,
    uint16_t object_ref,
    uint16_t *out_new_head_ref,
    uint16_t *out_previous_tail_ref,
    DM2_V1_ObjectTransferReceipt *out_receipt);

int dm2_v1_LOAD_PROJECTILE_TO_HAND(
    const DM2_V1_LoadProjectileToHandInput *input,
    uint16_t *out_hand_ref,
    uint16_t *out_source_head_ref,
    DM2_V1_ObjectTransferReceipt *out_receipt);

const char *dm2_v1_object_transfer_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_OBJECT_TRANSFER_HELPERS_H */
