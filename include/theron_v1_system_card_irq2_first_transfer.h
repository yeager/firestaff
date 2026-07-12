#ifndef THERON_V1_SYSTEM_CARD_IRQ2_FIRST_TRANSFER_H
#define THERON_V1_SYSTEM_CARD_IRQ2_FIRST_TRANSFER_H

#include "theron_v1_track02.h"

#include <stddef.h>
#include <stdint.h>

/*
 * The first direct control transfers in the authenticated System Card 3.0
 * IRQ2 handler reached by Theron's stage-three BRK. The live handler A value
 * is deliberately not inferred, so no branch, record request, or loader-data
 * route is selected here.
 */

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage3_track02_record;
    uint16_t handler_address;
    uint8_t accepted_a_minimum;
    uint8_t accepted_a_maximum;
    uint16_t out_of_range_transfer_address;
    uint16_t in_range_transfer_address;
    int selected_transfer_unobserved;
    int record_request_unproven;
} Theron_V1SystemCardIrq2FirstTransfer;

/* The caller must authenticate the supplied System Card 3.0 ROM. */
int theron_v1_system_card_irq2_first_transfer_from_original_media(
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    const uint8_t *system_card_rom,
    size_t system_card_rom_size,
    const char *system_card_rom_md5_hex,
    Theron_V1SystemCardIrq2FirstTransfer *out_transfer);

#endif /* THERON_V1_SYSTEM_CARD_IRQ2_FIRST_TRANSFER_H */
