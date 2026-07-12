#ifndef THERON_V1_SYSTEM_CARD_IRQ2_STATE_HANDOFF_H
#define THERON_V1_SYSTEM_CARD_IRQ2_STATE_HANDOFF_H

#include "theron_v1_track02.h"

#include <stddef.h>
#include <stdint.h>

/*
 * Source-locked state boundary from Theron's stage-two jump into the System
 * Card 3.0 IRQ2 handler. It describes register/control flow only: no later
 * Track 02 request, descriptor meaning, object, level, palette, or graphics
 * role is inferred.
 */

typedef struct {
    int valid;
    Theron_Track02Variant variant;
    uint32_t stage3_track02_record;
    uint16_t stage2_a_source_address;
    uint16_t stage2_x_source_address;
    uint16_t stage2_y_source_address;
    int stage2_increments_y;
    uint16_t irq2_vector_address;
    uint16_t irq2_handler_address;
    int handler_swaps_a_x;
    int stage2_x_becomes_handler_a;
    uint8_t handler_a_minimum;
    uint8_t handler_a_maximum;
    uint16_t handler_out_of_range_branch;
} Theron_V1SystemCardIrq2StateHandoff;

/* The caller must authenticate the supplied System Card 3.0 ROM. */
int theron_v1_system_card_irq2_state_handoff_from_original_media(
    const Theron_Track02Stage2DynamicPayloadReceipt *payload,
    const uint8_t *system_card_rom,
    size_t system_card_rom_size,
    const char *system_card_rom_md5_hex,
    Theron_V1SystemCardIrq2StateHandoff *out_handoff);

#endif /* THERON_V1_SYSTEM_CARD_IRQ2_STATE_HANDOFF_H */
