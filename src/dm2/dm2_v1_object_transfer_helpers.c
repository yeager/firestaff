#include "dm2_v1_object_transfer_helpers.h"

#include <string.h>

static void dm2_object_transfer_begin(
    DM2_V1_ObjectTransferReceipt *receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_v1_object_transfer_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

static int dm2_find_link(const DM2_V1_ObjectTransferLink *links,
                         size_t link_count,
                         uint16_t ref,
                         uint16_t *out_next_ref)
{
    size_t i;

    if (out_next_ref) {
        *out_next_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    }
    if (!links || ref == DM2_V1_OBJECT_TRANSFER_NULL ||
        link_count > DM2_V1_OBJECT_TRANSFER_MAX_LINKS) {
        return 0;
    }
    for (i = 0u; i < link_count; ++i) {
        if (links[i].ref == ref) {
            if (out_next_ref) {
                *out_next_ref = links[i].next_ref;
            }
            return 1;
        }
    }
    return 0;
}

void dm2_v1_object_transfer_receipt_clear(
    DM2_V1_ObjectTransferReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->object_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    receipt->container_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    receipt->previous_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    receipt->next_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    receipt->new_head_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    receipt->hand_ref = DM2_V1_OBJECT_TRANSFER_NULL;
}

int dm2_v1_REMOVE_POSSESSION(
    const DM2_V1_ObjectTransferLink *links,
    size_t link_count,
    uint16_t head_ref,
    uint16_t object_ref,
    uint16_t *out_new_head_ref,
    uint16_t *out_removed_next_ref,
    DM2_V1_ObjectTransferReceipt *out_receipt)
{
    uint16_t current;
    uint16_t previous = DM2_V1_OBJECT_TRANSFER_NULL;
    uint16_t next = DM2_V1_OBJECT_TRANSFER_NULL;
    size_t guard = 0u;

    if (out_new_head_ref) {
        *out_new_head_ref = head_ref;
    }
    if (out_removed_next_ref) {
        *out_removed_next_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    }
    dm2_object_transfer_begin(out_receipt,
                              "REMOVE_POSSESSION",
                              "SKWIN/SkWinCore.cpp:5430");
    if (!links || link_count > DM2_V1_OBJECT_TRANSFER_MAX_LINKS ||
        object_ref == DM2_V1_OBJECT_TRANSFER_NULL) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    current = head_ref;
    while (current != DM2_V1_OBJECT_TRANSFER_NULL &&
           guard++ < DM2_V1_OBJECT_TRANSFER_MAX_LINKS) {
        if (!dm2_find_link(links, link_count, current, &next)) {
            if (out_receipt) {
                out_receipt->blocked = 1;
            }
            return 0;
        }
        if (current == object_ref) {
            uint16_t new_head =
                previous == DM2_V1_OBJECT_TRANSFER_NULL ? next : head_ref;
            if (out_new_head_ref) {
                *out_new_head_ref = new_head;
            }
            if (out_removed_next_ref) {
                *out_removed_next_ref = next;
            }
            if (out_receipt) {
                out_receipt->valid = 1;
                out_receipt->mutated = 1;
                out_receipt->object_ref = object_ref;
                out_receipt->previous_ref = previous;
                out_receipt->next_ref = next;
                out_receipt->new_head_ref = new_head;
            }
            return 1;
        }
        previous = current;
        current = next;
    }
    if (out_receipt) {
        out_receipt->blocked = 1;
        out_receipt->object_ref = object_ref;
    }
    return 0;
}

int dm2_v1_PUT_OBJECT_INTO_CONTAINER(
    const DM2_V1_ObjectTransferLink *links,
    size_t link_count,
    uint16_t container_ref,
    uint16_t container_head_ref,
    uint16_t object_ref,
    uint16_t *out_new_head_ref,
    uint16_t *out_previous_tail_ref,
    DM2_V1_ObjectTransferReceipt *out_receipt)
{
    uint16_t current;
    uint16_t next;
    uint16_t tail = DM2_V1_OBJECT_TRANSFER_NULL;
    size_t guard = 0u;

    if (out_new_head_ref) {
        *out_new_head_ref = container_head_ref;
    }
    if (out_previous_tail_ref) {
        *out_previous_tail_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    }
    dm2_object_transfer_begin(out_receipt,
                              "PUT_OBJECT_INTO_CONTAINER",
                              "SKWIN/SkWinCore.cpp:7064");
    if (container_ref == DM2_V1_OBJECT_TRANSFER_NULL ||
        object_ref == DM2_V1_OBJECT_TRANSFER_NULL ||
        object_ref == container_ref ||
        link_count > DM2_V1_OBJECT_TRANSFER_MAX_LINKS) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    if (container_head_ref == DM2_V1_OBJECT_TRANSFER_NULL) {
        if (out_new_head_ref) {
            *out_new_head_ref = object_ref;
        }
        if (out_receipt) {
            out_receipt->valid = 1;
            out_receipt->mutated = 1;
            out_receipt->container_ref = container_ref;
            out_receipt->object_ref = object_ref;
            out_receipt->new_head_ref = object_ref;
        }
        return 1;
    }
    current = container_head_ref;
    while (current != DM2_V1_OBJECT_TRANSFER_NULL &&
           guard++ < DM2_V1_OBJECT_TRANSFER_MAX_LINKS) {
        if (current == object_ref ||
            !dm2_find_link(links, link_count, current, &next)) {
            if (out_receipt) {
                out_receipt->blocked = 1;
            }
            return 0;
        }
        tail = current;
        current = next;
    }
    if (guard > DM2_V1_OBJECT_TRANSFER_MAX_LINKS) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    if (out_previous_tail_ref) {
        *out_previous_tail_ref = tail;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->mutated = 1;
        out_receipt->container_ref = container_ref;
        out_receipt->object_ref = object_ref;
        out_receipt->previous_ref = tail;
        out_receipt->new_head_ref = container_head_ref;
    }
    return 1;
}

int dm2_v1_LOAD_PROJECTILE_TO_HAND(
    const DM2_V1_LoadProjectileToHandInput *input,
    uint16_t *out_hand_ref,
    uint16_t *out_source_head_ref,
    DM2_V1_ObjectTransferReceipt *out_receipt)
{
    uint16_t source_head;

    if (out_hand_ref) {
        *out_hand_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    }
    if (out_source_head_ref) {
        *out_source_head_ref = DM2_V1_OBJECT_TRANSFER_NULL;
    }
    dm2_object_transfer_begin(out_receipt,
                              "LOAD_PROJECTILE_TO_HAND",
                              "SKWIN/SkWinCore.cpp:5543");
    if (!input ||
        input->projectile_ref == DM2_V1_OBJECT_TRANSFER_NULL ||
        input->hand_ref != DM2_V1_OBJECT_TRANSFER_NULL) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    source_head = input->source_head_ref == input->projectile_ref
                      ? input->source_next_ref
                      : input->source_head_ref;
    if (out_hand_ref) {
        *out_hand_ref = input->projectile_ref;
    }
    if (out_source_head_ref) {
        *out_source_head_ref = source_head;
    }
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->mutated = 1;
        out_receipt->object_ref = input->projectile_ref;
        out_receipt->next_ref = input->source_next_ref;
        out_receipt->new_head_ref = source_head;
        out_receipt->hand_ref = input->projectile_ref;
    }
    return 1;
}

const char *dm2_v1_object_transfer_helpers_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp REMOVE_POSSESSION:5430 "
           "LOAD_PROJECTILE_TO_HAND:5543 PUT_OBJECT_INTO_CONTAINER:7064; "
           "bounded object-transfer receipts only.";
}
